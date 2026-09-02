/**
  ******************************************************************************
  * @file    app_ui.c
  * @author  Bowen (wbw20)
  * @date    2026-08-31
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），ST7735S 160x80 横屏
  * @brief   设置菜单（二级界面）：UI 状态机 + 菜单渲染 + 数值编辑
  *
  * 设计说明：
  *   - 三状态 UI 状态机：UI_MAIN（主界面）/ UI_MENU（设置菜单）/ UI_VALUE（数值编辑）；
  *   - 主界面只保留"长按 UP 关机、短按 SET 进设置"，其余用户设置全部收进菜单；
  *   - 菜单用查表描述（类型 + 字段 + 范围/步长），选中/滚动只做下标运算；
  *   - 勾选项：SET 切换（再按取消）；数值项：方案 A（UP=+步长、DOWN=-步长、SET=确认）；
  *   - 编辑全部落在 g_cfg_edit 工作副本上："保存并退出"才写回 g_cfg 并生效，
  *     "放弃并返回"丢弃副本，天然实现整页回滚；
  *   - 掉电存储（Flash）本版暂缺，Settings_Save 内留 TODO，先保证 RAM 逻辑可测。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-31) 首次创建：设置菜单 + 数值编辑（方案A）+ 保存/回滚。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "app_ui.h"
#include "global.h"
#include "st7735.h"
#include "Font_asc.h"

#include <string.h>

/* ---------- 菜单项描述表（数据驱动，加一项只改表不动逻辑） ---------- */

typedef enum {
    IT_CHECK,       /* 勾选项：SET 切换，再按取消 */
    IT_VALUE,       /* 数值项：方案 A 整值加减 */
    IT_ACTION       /* 动作项：保存并退出 / 放弃并返回 */
} ItemType_t;

typedef enum {
    F_FLIP,         /* 屏幕方向 */
    F_THEME,        /* 主题 */
    F_AUTOOFF,      /* 自动关机使能 */
    F_BLK,          /* 背光亮度 */
    F_AOFF_I,       /* 自动关机电流阈值 */
    F_AOFF_MIN,     /* 自动关机时长 */
    F_VBUS,         /* INA226 母线分压系数 */
    F_SAVE,         /* 保存并退出 */
    F_DISCARD       /* 放弃并返回 */
} FieldId_t;

typedef struct {
    const char *name;   /* ASCII 显示名（16x8 字库） */
    ItemType_t  type;   /* 菜单项类型                */
    FieldId_t   field;  /* 对应 g_cfg_edit 的字段     */
    float       min, max, step;  /* 仅 IT_VALUE 用    */
    uint8_t     prec;   /* 数值显示小数位             */
    const char *unit;   /* 值编辑页单位               */
} MenuItem_t;

/* 一屏最多 5 行：160x80，16x8 字库每行 16px */
#define MENU_ROWS 5

static const MenuItem_t g_menu[] = {
    { "FLIP",        IT_CHECK,  F_FLIP,     0.0f,   0.0f, 0.0f,   0, ""    },
    { "THEME",       IT_CHECK,  F_THEME,    0.0f,   0.0f, 0.0f,   0, ""    },
    { "AUTO OFF",    IT_CHECK,  F_AUTOOFF,  0.0f,   0.0f, 0.0f,   0, ""    },
    { "BLK",         IT_VALUE,  F_BLK,      0.0f, 100.0f, 5.0f,   0, "%"   },
    { "AOFF I",      IT_VALUE,  F_AOFF_I,   0.0f,   0.1f, 0.005f, 3, "A"   },
    { "AOFF MIN",    IT_VALUE,  F_AOFF_MIN, 1.0f, 120.0f, 1.0f,   0, "MIN" },
    { "VBUS DIV",    IT_VALUE,  F_VBUS,     0.5f,   3.0f, 0.01f,  2, ""    },
    { "SAVE & EXIT", IT_ACTION, F_SAVE,     0.0f,   0.0f, 0.0f,   0, ""    },
    { "DISCARD",     IT_ACTION, F_DISCARD,  0.0f,   0.0f, 0.0f,   0, ""    },
};
#define MENU_LEN ((uint8_t)(sizeof(g_menu) / sizeof(g_menu[0])))

/* UI 状态机运行态 ----------------------------------------------------*/
volatile uint8_t UI_STATE = UI_MAIN;

static Settings_t g_cfg_edit;       /* 设置界面的工作副本（保存/回滚的对象） */
static uint8_t   menu_idx = 0;      /* 选中项下标           */
static uint8_t   menu_top = 0;      /* 可视窗口顶部项下标   */
static uint8_t   edit_idx = 0;      /* 正在编辑的数值项下标 */

/* ---------- 配置字段读写：统一按 float 域操作，写入时转回各自类型 ---------- */

static float CfgGetF(FieldId_t f)
{
    switch (f)
    {
        case F_BLK:      return (float)g_cfg_edit.blk;
        case F_AOFF_I:   return g_cfg_edit.auto_off_i;
        case F_AOFF_MIN: return (float)g_cfg_edit.auto_off_min;
        case F_VBUS:     return g_cfg_edit.vbus_div;
        default:         return 0.0f;
    }
}

static void CfgSetF(FieldId_t f, float v)
{
    switch (f)
    {
        case F_BLK:      g_cfg_edit.blk          = (uint8_t)v;  break;
        case F_AOFF_I:   g_cfg_edit.auto_off_i   = v;           break;
        case F_AOFF_MIN: g_cfg_edit.auto_off_min = (uint16_t)v; break;
        case F_VBUS:     g_cfg_edit.vbus_div     = v;           break;
        default:                                                   break;
    }
}

/* 勾选项：在两种取值间切换；CfgChecked 决定菜单里显示 [x] 还是 [ ] */
static void CfgToggle(FieldId_t f)
{
    switch (f)
    {
        case F_FLIP:    g_cfg_edit.flip     = (g_cfg_edit.flip == 3) ? 1 : 3;              break;
        case F_THEME:   g_cfg_edit.theme    = (g_cfg_edit.theme == BLACK) ? WHITE : BLACK; break;
        case F_AUTOOFF: g_cfg_edit.auto_off = (uint8_t)(g_cfg_edit.auto_off ? 0U : 1U);    break;
        default:                                                                           break;
    }
}

static uint8_t CfgChecked(FieldId_t f)
{
    switch (f)
    {
        case F_FLIP:    return (g_cfg_edit.flip == 3);
        case F_THEME:   return (g_cfg_edit.theme == WHITE);
        case F_AUTOOFF: return (g_cfg_edit.auto_off != 0U);
        default:        return 0;
    }
}

/* 去掉 formatFloatToStr 右对齐补的前导 0：如 "00100"->"100"、"01.00"->"1.00"，
   但保留小数点前那个 0（"0.015" 不动） */
static void TrimLeadingZeros(char *s)
{
    if ((s == NULL) || (s[0] == '\0'))
    {
        return;
    }

    char *p   = s;
    char *dot = strchr(s, '.');
    while ((p[0] == '0') && (p[1] != '\0'))
    {
        if ((dot != NULL) && (p == (dot - 1)))
        {
            break;   /* 小数点前一位的 0 必须保留 */
        }
        p++;
    }

    if (p != s)
    {
        memmove(s, p, strlen(p) + 1U);
    }
}

/* ---------- 选中与滚动（只做下标运算，移动数据量为 0） ---------- */

static void MenuScroll(void)
{
    if (MENU_LEN <= MENU_ROWS)
    {
        menu_top = 0;   /* 项少到不需要滚动 */
        return;
    }

    if (menu_idx < menu_top)
    {
        menu_top = menu_idx;                        /* 选中项跑到窗口上方：窗口上移 */
    }
    else if (menu_idx >= (uint8_t)(menu_top + MENU_ROWS))
    {
        menu_top = (uint8_t)(menu_idx - (MENU_ROWS - 1U));  /* 下方：窗口下压，选中行贴底 */
    }

    if (menu_top > (uint8_t)(MENU_LEN - MENU_ROWS))
    {
        menu_top = (uint8_t)(MENU_LEN - MENU_ROWS);  /* 底部边界 */
    }
}

static void MenuMove(int8_t dir)
{
    if (dir > 0)
    {
        menu_idx = (uint8_t)((menu_idx + 1U) % MENU_LEN);             /* 向下，尾接首 */
    }
    else
    {
        menu_idx = (uint8_t)((menu_idx + MENU_LEN - 1U) % MENU_LEN);  /* 向上，首接尾 */
    }
    MenuScroll();
    FLAG = 1;
}

/* ---------- 数值编辑（方案 A：整值加减 + 钳位，无光标） ---------- */

static void ValueStep(int8_t dir)
{
    const MenuItem_t *it = &g_menu[edit_idx];
    float v = CfgGetF(it->field) + (float)dir * it->step;
    if (v > it->max) { v = it->max; }
    if (v < it->min) { v = it->min; }
    CfgSetF(it->field, v);
    FLAG = 1;
}

/* "保存并退出"：工作副本写回生效配置，并立即应用可即时生效的项。
   TODO(掉电存储)：下一步把 g_cfg 整块写内部 Flash（MAGIC+CRC+首启默认值恢复），
   本版先只做 RAM 生效，保证交互逻辑可测。 */
static void Settings_Save(void)
{
    g_cfg = g_cfg_edit;
    LCD_BLK(g_cfg.blk);   /* 背光立即生效；方向/主题在 FLAG 整屏重绘时生效 */
}

static void MenuActivate(void)
{
    const MenuItem_t *it = &g_menu[menu_idx];

    switch (it->type)
    {
        case IT_CHECK:                   /* 勾选：SET 切换，再按取消 */
            CfgToggle(it->field);
            FLAG = 1;
            break;

        case IT_VALUE:                   /* 数值：进入值编辑 */
            edit_idx = menu_idx;
            UI_STATE = UI_VALUE;
            FLAG = 1;
            break;

        case IT_ACTION:
            if (it->field == F_SAVE)
            {
                Settings_Save();         /* 保存并退出 */
            }
            else
            {
                g_cfg_edit = g_cfg;      /* 放弃并返回：整页回滚 */
            }
            UI_STATE = UI_MAIN;
            FLAG = 1;
            break;

        default:
            break;
    }
}

/* ---------- 对外：按键分发（按状态解释三键） ---------- */

void APP_UI_HandleKeys(uint8_t bt_up, uint8_t bt_set, uint8_t bt_down)
{
    switch (UI_STATE)
    {
        case UI_MAIN:
            /* 主界面：长按 UP 关机，短按 SET 进设置；DOWN 无功能 */
            if (bt_up == BTN_LONG)
            {
                power_off();
                FLAG = 1;            /* 唤醒后整屏重绘，恢复背光与画面 */
            }
            else if (bt_set == BTN_SHORT)
            {
                g_cfg_edit = g_cfg;  /* 进菜单从当前配置起步 */
                menu_idx = 0;
                menu_top = 0;
                UI_STATE = UI_MENU;
                FLAG = 1;
            }
            break;

        case UI_MENU:
            if      (bt_up   == BTN_SHORT) { MenuMove(-1); }
            else if (bt_down == BTN_SHORT) { MenuMove(+1); }
            else if (bt_set  == BTN_SHORT) { MenuActivate(); }
            break;

        case UI_VALUE:
            if      (bt_up   == BTN_SHORT) { ValueStep(+1); }
            else if (bt_down == BTN_SHORT) { ValueStep(-1); }
            else if (bt_set  == BTN_SHORT) { UI_STATE = UI_MENU; FLAG = 1; }  /* 确认返回菜单 */
            break;

        default:
            UI_STATE = UI_MAIN;      /* 防御：异常状态回主界面 */
            FLAG = 1;
            break;
    }
}

/* ---------- 渲染 ---------- */

/* 深浅主题的易读前景色：完整"主题调色板"后续再做，这里先保证黑白两底可读 */
static uint16_t MenuFg(uint16_t bg)
{
    return (bg == BLACK) ? GREEN_UI : 0x03E0;   /* 深底亮绿 / 浅底深绿 */
}

static void MenuDraw(void)
{
    uint16_t bg      = g_cfg.theme;                              /* 菜单沿用当前生效主题底色 */
    uint16_t fg      = MenuFg(bg);
    uint16_t hl_bg   = (bg == BLACK) ? WHITE : BLACK;            /* 选中行反显 */
    uint8_t  max_top = (MENU_LEN > MENU_ROWS) ? (uint8_t)(MENU_LEN - MENU_ROWS) : 0;

    LCD_Clear(bg);

    for (uint8_t r = 0; r < MENU_ROWS; r++)
    {
        uint8_t idx = (uint8_t)(menu_top + r);
        if (idx >= MENU_LEN)
        {
            break;
        }

        const MenuItem_t *it = &g_menu[idx];
        uint16_t y   = (uint16_t)r * 16U;
        uint8_t  sel = (idx == menu_idx);

        if (sel)
        {
            LCD_DrawRect_Fill(0, y, LCD_W, 16, hl_bg);   /* 反显高亮选中行 */
        }
        uint16_t row_bg = sel ? hl_bg : bg;
        uint16_t row_fg = sel ? bg : fg;

        LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, y, row_fg, row_bg, (char *)it->name);

        char rhs[8] = {0};
        if (it->type == IT_CHECK)
        {
            strcpy(rhs, CfgChecked(it->field) ? "[x]" : "[ ]");
        }
        else if (it->type == IT_VALUE)
        {
            formatFloatToStr(CfgGetF(it->field), rhs, 5, it->prec);
            TrimLeadingZeros(rhs);
        }

        if (rhs[0] != '\0')
        {
            uint16_t x = (uint16_t)(154 - (strlen(rhs) * 8U));   /* 右对齐，右侧留 4px 滚动条 */
            LCD_ShowString(h16w8_sample, h16w8, 16, 8, x, y, row_fg, row_bg, rhs);
        }
    }

    /* 右侧滚动条：滑块位置表示窗口位置（项数超过一屏时才有意义） */
    if (MENU_LEN > MENU_ROWS)
    {
        uint16_t thumb_h = (uint16_t)((MENU_ROWS * 80U) / MENU_LEN);
        uint16_t thumb_y = (uint16_t)(((uint32_t)menu_top * (80U - thumb_h)) / (uint32_t)max_top);
        LCD_DrawRect_Fill(156, 0,       3, 80,      GRAY_UI);     /* 轨道 */
        LCD_DrawRect_Fill(156, thumb_y, 3, thumb_h, YELLOW_UI);   /* 滑块 */
    }
}

static void ValueDraw(void)
{
    const MenuItem_t *it = &g_menu[edit_idx];
    uint16_t bg  = g_cfg.theme;
    uint16_t fg  = MenuFg(bg);
    uint16_t big = (bg == BLACK) ? WHITE : BLACK;   /* 大数值用反色保证对比度 */
    char buf[8];

    LCD_Clear(bg);

    /* 顶行：项目名 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 0, fg, bg, (char *)it->name);

    /* 分隔线 */
    LCD_DrawRect_Fill(0, 16, LCD_W, 1, fg);

    /* 大号数值（24x12），水平居中 */
    formatFloatToStr(CfgGetF(it->field), buf, 5, it->prec);
    TrimLeadingZeros(buf);
    uint16_t w = (uint16_t)(strlen(buf) * 12U);
    uint16_t x = (uint16_t)((LCD_W - w) / 2U);
    LCD_ShowString(h24w12_sample, h24w12, 24, 12, x, 28, big, bg, buf);

    /* 单位（紧贴数值右侧） */
    if (it->unit[0] != '\0')
    {
        LCD_ShowString(h16w8_sample, h16w8, 16, 8, (uint16_t)(x + w + 4), 32, fg, bg, (char *)it->unit);
    }

    /* 底部操作提示 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2,   64, fg, bg, "SET=OK");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 104, 64, fg, bg, "UP+");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 132, 64, fg, bg, "DN-");
}

void APP_UI_Draw(void)
{
    switch (UI_STATE)
    {
        case UI_MENU:  MenuDraw();   break;
        case UI_VALUE: ValueDraw();  break;
        default:       break;        /* UI_MAIN 由 main.c 主界面绘制 */
    }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
