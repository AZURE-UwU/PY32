/**
  ******************************************************************************
  * @file    app_ui.c
  * @author  Bowen (wbw20)
  * @date    2026-09-02
  * @version V1.8
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），ST7735S 160x80 横屏
  * @brief   设置菜单（二级界面）：UI 状态机 + 菜单渲染 + 数值编辑
  *
  * 设计说明：
  *   - 四状态 UI 状态机：UI_MAIN / UI_MENU / UI_VALUE（数值编辑）/ UI_OPTION（选项选择）；
  *   - 主界面只保留"长按 UP 关机、短按 SET 进设置"，其余用户设置全部收进菜单；
  *   - 菜单用查表描述（类型 + 字段 + 范围/步长），选中/滚动只做下标运算；
  *   - 勾选项：SET 切换（再按取消）；数值项：方案 A；选项项：UP/DOWN 循环选、SET 确认；
  *   - 编辑全部落在 s_cfg_edit 工作副本上："保存并退出"才写回 g_cfg 并生效，
  *     "放弃并返回"丢弃副本，天然实现整页回滚；
  *   - 掉电存储（Flash）：配置整块写入内部 Flash 配置页（MAGIC+版本+CRC16），开机自动恢复。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-31) 首次创建：设置菜单 + 数值编辑（方案A）+ 保存/回滚。
  *   V1.1 (2026-09-02) 数值编辑新增长按连跳：前2秒慢档(200ms/步)、2秒后快档(50ms/步)，
  *                     步进钳位，数值到顶/到底停在边界不循环。
  *   V1.2 (2026-09-02) 设置菜单新增长按连跳（与值编辑共用两档速度）；
  *                     选中/滚动改为增量重绘，翻项过程无整屏清屏闪烁。
  *   V1.3 (2026-09-02) 值编辑步进改为仅刷新数值区域（ValueNumberDraw），
  *                     不再整屏重绘；数值到边界不变时直接不重绘。
  *   V1.4 (2026-09-02) "保存并退出"写入内部 Flash（MAGIC+版本+CRC16+配置），
  *                     开机自动恢复；首启/损坏自动写回默认值。
  *   V1.5 (2026-09-06) THEME 改为选项式（IT_OPTION/UI_OPTION）：
  *                     UP/DOWN 在 WHITE/BLACK 间循环选择，SET 确认，类似数值编辑界面。
  *   V1.6 (2026-09-06) 主题改为"编号"（只影响主界面）：设置/数值/选项页固定深色底，
  *                     与主题解耦；Settings_t 布局变化，CFG_VERSION 升级到 2。
  *   V1.7 (2026-09-08) 命名严格对齐：模块私有 static 统一 s_ 前缀
  *                     （g_menu->s_menu、g_theme_opts->s_theme_opts、g_cfg_edit->s_cfg_edit）。
  *   V1.8 (2026-10-03) 修正设计说明中"掉电存储本版暂缺"的过时表述（Flash 掉电存储已在 V1.4 实现）。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "app_ui.h"
#include "global.h"
#include "st7735.h"
#include "Font_asc.h"
#include "bsp_flash.h"

#include <string.h>

/* ---------- 掉电存储帧格式（存放在 bsp_flash 的配置页内） ---------- */
/* 页内布局：0~3=MAGIC，4~5=VERSION，6~7=CRC16(payload)，8~=Settings_t，其余 0xFF */
#define CFG_HDR_SIZE   8U
#define CFG_VERSION    2U
static const uint8_t CFG_MAGIC[4] = {'P', 'Y', '3', '2'};

/* ---------- 菜单项描述表（数据驱动，加一项只改表不动逻辑） ---------- */

typedef enum {
    IT_CHECK,       /* 勾选项：SET 切换，再按取消 */
    IT_VALUE,       /* 数值项：方案 A 整值加减 */
    IT_OPTION,      /* 选项项：进入选择页，UP/DOWN 循环选，SET 确认 */
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
    FieldId_t   field;  /* 对应 s_cfg_edit 的字段     */
    float       min, max, step;  /* 仅 IT_VALUE 用    */
    uint8_t     prec;   /* 数值显示小数位             */
    const char *unit;   /* 值编辑页单位               */
} MenuItem_t;

/* 一屏最多 5 行：160x80，16x8 字库每行 16px */
#define MENU_ROWS 5

static const MenuItem_t s_menu[] = {
    { "FLIP",        IT_CHECK,  F_FLIP,     0.0f,   0.0f, 0.0f,   0, ""    },
    { "THEME",       IT_OPTION, F_THEME,    0.0f,   0.0f, 0.0f,   0, ""    },
    { "AUTO OFF",    IT_CHECK,  F_AUTOOFF,  0.0f,   0.0f, 0.0f,   0, ""    },
    { "BLK",         IT_VALUE,  F_BLK,      0.0f, 100.0f, 5.0f,   0, "%"   },
    { "AOFF I",      IT_VALUE,  F_AOFF_I,   0.0f,   0.1f, 0.005f, 3, "A"   },
    { "AOFF MIN",    IT_VALUE,  F_AOFF_MIN, 1.0f, 120.0f, 1.0f,   0, "MIN" },
    { "VBUS DIV",    IT_VALUE,  F_VBUS,     0.5f,   3.0f, 0.01f,  2, ""    },
    { "SAVE & EXIT", IT_ACTION, F_SAVE,     0.0f,   0.0f, 0.0f,   0, ""    },
    { "DISCARD",     IT_ACTION, F_DISCARD,  0.0f,   0.0f, 0.0f,   0, ""    },
};
#define MENU_LEN ((uint8_t)(sizeof(s_menu) / sizeof(s_menu[0])))

/* THEME 选项表：{显示标签, 主题编号}。g_cfg.theme 存的是主题编号，
   主题只影响主界面（main.c 的 switch 按编号绘制）；以后新增主题加一行即可，
   并保持总数与 THEME_COUNT 一致。 */
typedef struct {
    const char *label;
    uint8_t     code;
} ThemeOption_t;

static const ThemeOption_t s_theme_opts[] = {
    { "BLACK", 0 },
    { "WHITE", 1 },
};
#define THEME_OPT_COUNT ((uint8_t)(sizeof(s_theme_opts) / sizeof(s_theme_opts[0])))

/* 按主题编号找选项下标；找不到回 0（防御） */
static uint8_t ThemeOptIndexByCode(uint8_t code)
{
    for (uint8_t i = 0; i < THEME_OPT_COUNT; i++)
    {
        if (s_theme_opts[i].code == code)
        {
            return i;
        }
    }
    return 0;
}

/* 长按连跳参数：慢档先粗定位，1.5 秒后转快档快速扫大范围。
   步进本身走 ValueStep 的钳位，数值到顶/到底停在边界，不会循环。 */
#define REPEAT_SLOW_MS         100    /* 慢档：每 100ms 一步 */
#define REPEAT_FAST_MS          20    /* 快档：每 20ms 一步  */
#define REPEAT_SLOW_WINDOW_MS 1500    /* 慢档持续 1.5s，之后转快档 */

/* UI 状态机运行态 ----------------------------------------------------*/
volatile uint8_t UI_STATE = UI_MAIN;

static Settings_t s_cfg_edit;       /* 设置界面的工作副本（保存/回滚的对象） */
static uint8_t   menu_idx = 0;      /* 选中项下标           */
static uint8_t   menu_top = 0;      /* 可视窗口顶部项下标   */
static uint8_t   edit_idx = 0;      /* 正在编辑的数值项下标 */
static uint8_t   opt_item = 0;      /* 正在编辑的选项项下标 */
static uint8_t   opt_idx  = 0;      /* 当前选项下标         */
static uint64_t  repeat_start = 0;  /* 长按连跳开始时刻（sys） */
static uint64_t  last_step    = 0;  /* 上一次步进时刻         */

/* 渲染辅助前向声明：MenuMove/MenuActivate 定义在前，需提前可见 */
static void MenuRowDraw(uint8_t idx, uint8_t r, uint8_t sel);
static void ValueNumberDraw(void);
static void OptionNumberDraw(void);

/* ---------- 配置字段读写：统一按 float 域操作，写入时转回各自类型 ---------- */

static float CfgGetF(FieldId_t f)
{
    switch (f)
    {
        case F_BLK:      return (float)s_cfg_edit.blk;
        case F_AOFF_I:   return s_cfg_edit.auto_off_i;
        case F_AOFF_MIN: return (float)s_cfg_edit.auto_off_min;
        case F_VBUS:     return s_cfg_edit.vbus_div;
        default:         return 0.0f;
    }
}

static void CfgSetF(FieldId_t f, float v)
{
    switch (f)
    {
        case F_BLK:      s_cfg_edit.blk          = (uint8_t)v;  break;
        case F_AOFF_I:   s_cfg_edit.auto_off_i   = v;           break;
        case F_AOFF_MIN: s_cfg_edit.auto_off_min = (uint16_t)v; break;
        case F_VBUS:     s_cfg_edit.vbus_div     = v;           break;
        default:                                                   break;
    }
}

/* 勾选项：在两种取值间切换；CfgChecked 决定菜单里显示 [x] 还是 [ ] */
static void CfgToggle(FieldId_t f)
{
    switch (f)
    {
        case F_FLIP:    s_cfg_edit.flip     = (s_cfg_edit.flip == 3) ? 1 : 3;              break;
        case F_AUTOOFF: s_cfg_edit.auto_off = (uint8_t)(s_cfg_edit.auto_off ? 0U : 1U);    break;
        default:                                                                           break;
    }
}

static uint8_t CfgChecked(FieldId_t f)
{
    switch (f)
    {
        case F_FLIP:    return (s_cfg_edit.flip == 3);
        case F_AUTOOFF: return (s_cfg_edit.auto_off != 0U);
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
    uint8_t old_idx = menu_idx;
    uint8_t old_top = menu_top;

    if (dir > 0)
    {
        menu_idx = (uint8_t)((menu_idx + 1U) % MENU_LEN);             /* 向下，尾接首 */
    }
    else
    {
        menu_idx = (uint8_t)((menu_idx + MENU_LEN - 1U) % MENU_LEN);  /* 向上，首接尾 */
    }
    MenuScroll();

    if (menu_top != old_top)
    {
        FLAG = 1;   /* 可视窗口滚动：整屏重绘 */
    }
    else if (menu_idx != old_idx)
    {
        /* 窗口未动：只增量重绘"旧选中行(恢复)"与"新选中行(反显)"，
           避免连跳时整屏清屏闪烁 */
        MenuRowDraw(old_idx,  (uint8_t)(old_idx  - menu_top), 0);
        MenuRowDraw(menu_idx, (uint8_t)(menu_idx - menu_top), 1);
    }
}

/* ---------- 数值编辑（方案 A：整值加减 + 钳位，无光标） ---------- */

static void ValueStep(int8_t dir)
{
    const MenuItem_t *it = &s_menu[edit_idx];
    float old = CfgGetF(it->field);
    float v   = old + (float)dir * it->step;
    if (v > it->max) { v = it->max; }
    if (v < it->min) { v = it->min; }

    if (v == old)
    {
        return;              /* 已到边界：数值没变，不重绘（到顶/到底不循环） */
    }

    CfgSetF(it->field, v);
    ValueNumberDraw();       /* 仅刷新数值区域，不整屏重绘 */
}

/* 清除连跳状态：释放按键或进出值编辑时调用，下次长按重新从慢档开始 */
static void RepeatReset(void)
{
    repeat_start = 0;
    last_step    = 0;
}

/* 连跳节流：返回 1 表示本拍该步进一次。按住期间 btnLogic_withExtLongFlag
   每 50ms 持续返回 BTN_LONG，这里按"连跳已持续多久"选慢/快档节流，
   第一拍立即步进；状态由 RepeatReset 清理，下次长按重新从慢档开始。 */
static uint8_t RepeatReady(void)
{
    uint64_t now = sys;

    if (repeat_start == 0)
    {
        repeat_start = now;   /* 连跳开始：第一拍立即步进一次 */
    }

    uint32_t elapsed  = (uint32_t)(now - repeat_start);
    uint32_t interval = (elapsed < REPEAT_SLOW_WINDOW_MS) ? REPEAT_SLOW_MS : REPEAT_FAST_MS;

    if ((last_step == 0) || ((now - last_step) >= interval))
    {
        last_step = now;
        return 1;
    }
    return 0;
}

/* 值编辑连跳：到顶/到底时 ValueStep 钳位在边界，不会跳到另一端 */
static void ValueRepeat(int8_t dir)
{
    if (RepeatReady())
    {
        ValueStep(dir);
    }
}

/* 菜单连跳：MenuMove 内部负责增量/整屏重绘 */
static void MenuRepeat(int8_t dir)
{
    if (RepeatReady())
    {
        MenuMove(dir);
    }
}

/* 选项循环：UP/DOWN 在选项表里循环，即时写入编辑副本并只刷选项标签区 */
static void OptionMove(int8_t dir)
{
    if (dir > 0)
    {
        opt_idx = (uint8_t)((opt_idx + 1U) % THEME_OPT_COUNT);
    }
    else
    {
        opt_idx = (uint8_t)((opt_idx + THEME_OPT_COUNT - 1U) % THEME_OPT_COUNT);
    }
    s_cfg_edit.theme = s_theme_opts[opt_idx].code;
    OptionNumberDraw();
}

/* CRC16-CCITT（poly 0x1021，初值 0xFFFF）：只校验 Settings_t 负载，
   MAGIC/版本在加载时单独比对，CRC 损坏同样会被识别为无效 */
static uint16_t Crc16Ccitt(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFFU;

    if (buf == NULL)
    {
        return crc;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)buf[i] << 8;
        for (uint8_t k = 0; k < 8; k++)
        {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U)
                                  : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

/* 组帧并写整页（未用字节填 0xFF），成功返回 1，失败返回 0 */
static uint8_t Settings_Write(const Settings_t *s)
{
    if (s == NULL)
    {
        return 0;
    }

    CfgPage_t page;
    memset(page.b, 0xFF, sizeof(page));

    memcpy(&page.b[0], CFG_MAGIC, 4U);
    uint16_t ver = CFG_VERSION;
    memcpy(&page.b[4], &ver, 2U);
    memcpy(&page.b[CFG_HDR_SIZE], s, sizeof(*s));
    uint16_t crc = Crc16Ccitt(page.b + CFG_HDR_SIZE, (uint16_t)sizeof(*s));
    memcpy(&page.b[6], &crc, 2U);

    return BSP_Flash_CfgPageWrite(&page);
}

/* "保存并退出"：先把编辑副本写进 Flash，写成功才提交到 g_cfg 并立即生效。
   写失败则保持旧配置（RAM 不变），避免"屏幕变了、断电又回旧值"的不一致。 */
static void Settings_Save(void)
{
    if (Settings_Write(&s_cfg_edit))
    {
        g_cfg = s_cfg_edit;
        LCD_BLK(g_cfg.blk);   /* 背光立即生效；方向/主题在 FLAG 整屏重绘时生效 */
    }
}

/* 开机恢复：校验 MAGIC/版本/CRC 与字段范围，通过则回填 g_cfg；
   首次上电或数据损坏时，用当前默认值写回 Flash，实现"首启自动写默认"。 */
void APP_UI_SettingsLoad(void)
{
    CfgPage_t page;
    BSP_Flash_CfgPageRead(&page);

    Settings_t tmp;
    memcpy(&tmp, page.b + CFG_HDR_SIZE, sizeof(tmp));

    uint16_t ver = 0;
    uint16_t crc = 0;
    memcpy(&ver, &page.b[4], 2U);
    memcpy(&crc, &page.b[6], 2U);

    uint8_t valid = (memcmp(page.b, CFG_MAGIC, 4U) == 0) &&
                    (ver == CFG_VERSION) &&
                    (crc == Crc16Ccitt(page.b + CFG_HDR_SIZE, (uint16_t)sizeof(tmp)));

    /* 防御：字段范围再验一遍，坏值/NaN 一律视为无效回默认 */
    valid = valid &&
            ((tmp.flip == 1) || (tmp.flip == 3)) &&
            (tmp.theme < THEME_COUNT) &&
            (tmp.blk <= 100U) &&
            (tmp.auto_off <= 1U) &&
            (tmp.auto_off_i >= 0.0f) && (tmp.auto_off_i <= 0.5f) &&
            (tmp.auto_off_min >= 1U) && (tmp.auto_off_min <= 1440U) &&
            (tmp.vbus_div >= 0.1f) && (tmp.vbus_div <= 10.0f);

    if (valid)
    {
        g_cfg = tmp;
    }
    else
    {
        Settings_Write(&g_cfg);   /* 首启/损坏：把当前默认值写回 Flash */
    }
}

static void MenuActivate(void)
{
    const MenuItem_t *it = &s_menu[menu_idx];

    switch (it->type)
    {
        case IT_CHECK:                   /* 勾选：SET 切换，再按取消 */
            CfgToggle(it->field);
            MenuRowDraw(menu_idx, (uint8_t)(menu_idx - menu_top), 1);   /* 只重画当前行 */
            break;

        case IT_VALUE:                   /* 数值：进入值编辑 */
            edit_idx = menu_idx;
            RepeatReset();               /* 清上一轮连跳状态，从头慢档开始 */
            UI_STATE = UI_VALUE;
            FLAG = 1;
            break;

        case IT_OPTION:                  /* 选项：进入选项选择页 */
            opt_item = menu_idx;
            opt_idx  = ThemeOptIndexByCode(s_cfg_edit.theme);
            RepeatReset();
            UI_STATE = UI_OPTION;
            FLAG = 1;
            break;

        case IT_ACTION:
            if (it->field == F_SAVE)
            {
                Settings_Save();         /* 保存并退出 */
            }
            else
            {
                s_cfg_edit = g_cfg;      /* 放弃并返回：整页回滚 */
            }
            RepeatReset();               /* 离开设置界面，清连跳状态 */
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
                s_cfg_edit = g_cfg;  /* 进菜单从当前配置起步 */
                menu_idx = 0;
                menu_top = 0;
                RepeatReset();       /* 清连跳状态 */
                UI_STATE = UI_MENU;
                FLAG = 1;
            }
            break;

        case UI_MENU:
            if (bt_up == BTN_LONG)
            {
                MenuRepeat(-1);          /* 按住 UP：连续上翻 */
            }
            else if (bt_down == BTN_LONG)
            {
                MenuRepeat(+1);          /* 按住 DOWN：连续下翻 */
            }
            else if (bt_up == BTN_SHORT)
            {
                RepeatReset();
                MenuMove(-1);            /* 单击 UP：上移一项 */
            }
            else if (bt_down == BTN_SHORT)
            {
                RepeatReset();
                MenuMove(+1);            /* 单击 DOWN：下移一项 */
            }
            else if (bt_set == BTN_SHORT)
            {
                RepeatReset();
                MenuActivate();
            }
            else
            {
                RepeatReset();           /* 释放后清连跳状态 */
            }
            break;

        case UI_VALUE:
            if (bt_up == BTN_LONG)
            {
                ValueRepeat(+1);         /* 按住 UP：连跳加 */
            }
            else if (bt_down == BTN_LONG)
            {
                ValueRepeat(-1);         /* 按住 DOWN：连跳减 */
            }
            else if (bt_up == BTN_SHORT)
            {
                RepeatReset();
                ValueStep(+1);           /* 单击 UP：+1 步 */
            }
            else if (bt_down == BTN_SHORT)
            {
                RepeatReset();
                ValueStep(-1);           /* 单击 DOWN：-1 步 */
            }
            else if (bt_set == BTN_SHORT)
            {
                RepeatReset();
                UI_STATE = UI_MENU;      /* 确认返回菜单 */
                FLAG = 1;
            }
            else
            {
                RepeatReset();           /* 释放后清状态，下次长按重新从慢档开始 */
            }
            break;

        case UI_OPTION:
            if (bt_up == BTN_SHORT)
            {
                OptionMove(-1);          /* 上一个选项 */
            }
            else if (bt_down == BTN_SHORT)
            {
                OptionMove(+1);          /* 下一个选项 */
            }
            else if (bt_set == BTN_SHORT)
            {
                RepeatReset();
                UI_STATE = UI_MENU;      /* 确认返回菜单 */
                FLAG = 1;
            }
            break;

        default:
            UI_STATE = UI_MAIN;      /* 防御：异常状态回主界面 */
            FLAG = 1;
            break;
    }
}

/* ---------- 渲染 ---------- */

/* 画菜单一行：先整行填底色抹掉旧内容/旧高亮，再画名称与右值。
   sel=1 时反显高亮。只填到 x=154，右侧 155~159 留给滚动条，避免增量重绘时擦掉它。 */
static void MenuRowDraw(uint8_t idx, uint8_t r, uint8_t sel)
{
    const MenuItem_t *it = &s_menu[idx];
    uint16_t bg      = BLACK;      /* 设置界面固定深色底，不随主题变化 */
    uint16_t fg      = GREEN_UI;
    uint16_t hl_bg   = WHITE;      /* 选中行反显 */
    uint16_t y       = (uint16_t)r * 16U;
    uint16_t row_bg  = sel ? hl_bg : bg;
    uint16_t row_fg  = sel ? bg : fg;

    LCD_DrawRect_Fill(0, y, 155, 16, row_bg);

    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, y, row_fg, row_bg, (char *)it->name);

    char rhs[8] = {0};
    if (it->type == IT_CHECK)
    {
        strcpy(rhs, CfgChecked(it->field) ? "[v]" : "[ ]");
    }
    else if (it->type == IT_VALUE)
    {
        formatFloatToStr(CfgGetF(it->field), rhs, 5, it->prec);
        TrimLeadingZeros(rhs);
    }
    else if (it->type == IT_OPTION)
    {
        strcpy(rhs, s_theme_opts[ThemeOptIndexByCode(s_cfg_edit.theme)].label);
    }

    if (rhs[0] != '\0')
    {
        uint16_t x = (uint16_t)(154 - (strlen(rhs) * 8U));   /* 右对齐，右侧留滚动条 */
        LCD_ShowString(h16w8_sample, h16w8, 16, 8, x, y, row_fg, row_bg, rhs);
    }
}

/* 右侧滚动条：轨道 + 滑块（滑块位置表示可视窗口位置） */
static void MenuScrollbarDraw(void)
{
    if (MENU_LEN <= MENU_ROWS)
    {
        return;
    }

    uint8_t  max_top = (uint8_t)(MENU_LEN - MENU_ROWS);
    uint16_t thumb_h = (uint16_t)((MENU_ROWS * 80U) / MENU_LEN);
    uint16_t thumb_y = (uint16_t)(((uint32_t)menu_top * (80U - thumb_h)) / (uint32_t)max_top);
    LCD_DrawRect_Fill(156, 0,       3, 80,      GRAY_UI);     /* 轨道 */
    LCD_DrawRect_Fill(156, thumb_y, 3, thumb_h, YELLOW_UI);   /* 滑块 */
}

static void MenuDraw(void)
{
    uint16_t bg = BLACK;   /* 固定深色底 */

    /* 滚动条留白列（x=155~159）先填底色，覆盖进入菜单前的旧画面残留；
       菜单行本身整行填色，因此无需整屏 LCD_Clear，翻项不闪烁 */
    LCD_DrawRect_Fill(155, 0, 5, 80, bg);

    if (MENU_LEN < MENU_ROWS)
    {
        LCD_Clear(bg);   /* 项数不足一屏：底部留白需整体清一次屏 */
    }

    for (uint8_t r = 0; r < MENU_ROWS; r++)
    {
        uint8_t idx = (uint8_t)(menu_top + r);
        if (idx >= MENU_LEN)
        {
            break;
        }
        MenuRowDraw(idx, r, (idx == menu_idx));
    }

    MenuScrollbarDraw();
}

/* 仅刷新数值区域：抹掉数字带后重画大数值+单位，不动项目名/分隔线/底部提示。
   长按连跳每步只刷这一块，避免整屏清屏闪烁并节省 SPI 传输。 */
static void ValueNumberDraw(void)
{
    const MenuItem_t *it = &s_menu[edit_idx];
    uint16_t bg  = BLACK;
    uint16_t fg  = GREEN_UI;
    uint16_t big = WHITE;   /* 大数值用反色保证对比度 */
    char buf[8];

    /* 数字带：24x12 字高 24，覆盖 y=28~51 */
    LCD_DrawRect_Fill(0, 28, LCD_W, 24, bg);

    formatFloatToStr(CfgGetF(it->field), buf, 5, it->prec);
    TrimLeadingZeros(buf);
    uint16_t w = (uint16_t)(strlen(buf) * 12U);
    uint16_t x = (uint16_t)((LCD_W - w) / 2U);
    LCD_ShowString(h24w12_sample, h24w12, 24, 12, x, 28, big, bg, buf);

    if (it->unit[0] != '\0')
    {
        LCD_ShowString(h16w8_sample, h16w8, 16, 8, (uint16_t)(x + w + 4), 32, fg, bg, (char *)it->unit);
    }
}

static void ValueDraw(void)
{
    const MenuItem_t *it = &s_menu[edit_idx];
    uint16_t bg = BLACK;
    uint16_t fg = GREEN_UI;

    LCD_Clear(bg);

    /* 顶行：项目名 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 0, fg, bg, (char *)it->name);

    /* 分隔线 */
    LCD_DrawRect_Fill(0, 16, LCD_W, 1, fg);

    /* 大数值 + 单位（步进时由 ValueNumberDraw 单独刷新） */
    ValueNumberDraw();

    /* 底部操作提示 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2,   64, fg, bg, "SET=OK");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 104, 64, fg, bg, "UP+");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 132, 64, fg, bg, "DN-");
}

/* 仅刷新选项标签区（类似 ValueNumberDraw，但显示的是文字标签而非数字） */
static void OptionNumberDraw(void)
{
    uint16_t bg  = BLACK;
    uint16_t big = WHITE;   /* 反色保证对比度 */
    const char *label = s_theme_opts[opt_idx].label;

    LCD_DrawRect_Fill(0, 28, LCD_W, 24, bg);

    uint16_t w = (uint16_t)(strlen(label) * 12U);
    uint16_t x = (uint16_t)((LCD_W - w) / 2U);
    LCD_ShowString(h24w12_sample, h24w12, 24, 12, x, 28, big, bg, (char *)label);
}

static void OptionDraw(void)
{
    const MenuItem_t *it = &s_menu[opt_item];
    uint16_t bg = BLACK;
    uint16_t fg = GREEN_UI;

    LCD_Clear(bg);

    /* 顶行：项目名 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 0, fg, bg, (char *)it->name);

    /* 分隔线 */
    LCD_DrawRect_Fill(0, 16, LCD_W, 1, fg);

    /* 大号选项标签（UP/DOWN 时由 OptionNumberDraw 单独刷新） */
    OptionNumberDraw();

    /* 底部操作提示 */
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2,   64, fg, bg, "SET=OK");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 104, 64, fg, bg, "UP+");
    LCD_ShowString(h16w8_sample, h16w8, 16, 8, 132, 64, fg, bg, "DN-");
}

void APP_UI_Draw(void)
{
    switch (UI_STATE)
    {
        case UI_MENU:   MenuDraw();   break;
        case UI_VALUE:  ValueDraw();  break;
        case UI_OPTION: OptionDraw(); break;
        default:        break;        /* UI_MAIN 由 main.c 主界面绘制 */
    }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
