/**
  ******************************************************************************
  * @file    function.c
  * @author  Bowen (wbw20)
  * @date    2026-09-01
  * @version V1.5
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   通用工具函数：格式化 / 滤波 / 校准 / 按键状态机 / 低功耗
  *
  * 设计说明：
  *   - 所有函数与具体业务解耦，可被多个模块复用；
  *   - 按键短按/长按/双击用状态机 + 静态状态记忆；
  *   - NTC 温度先按公式换算，再交给卡尔曼滤波；
  *   - 关键阈值（温度/PWM/长按/双击）全部 #define。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 移植自 G030 工程 function.c v1.3.0；
 *   V1.1 (2026-08-19) 适配 PY32F002B：MAX_KEYS=3、按键读取走 BSP、
 *                     移除 RTC 备份寄存器、STANDBY 改为 STOP 唤醒。
 *   V1.2 (2026-08-24) 修复按键链路：PY32 HAL 只有 HAL_GPIO_EXTI_Callback，
 *                     没有 STM32 风格的 Rising/Falling 回调；原实现导致
 *                     "只进中断、无业务逻辑"。同时消抖改为时间戳对比，
 *                     避免边沿标志位在中断与主循环间的竞态。
 *   V1.3 (2026-08-24) 修复长按关机循环：进 STOP 前只保留 SW_WKUP 上升沿唤醒、
 *                     停 SysTick 并清 pending，唤醒后恢复按键配置并复位按键状态机。
  *   V1.4 (2026-08-30) 关机提示 POWEROFF 改走通用 LCD_ShowString
  *                     （LCD_ShowString_16_8/24_12 专用实现已移除）。
  *   V1.5 (2026-09-01) 双击检测改为 BTN_DOUBLE_CLICK_ENABLE 宏可开关：
  *                     关闭时短按松开即报，不再等待双击窗口（提升单按效率）。
 ******************************************************************************
 */

/* 头文件包含 --------------------------------------------------------*/
#include "function.h"
#include "main.h"
#include "global.h"
#include "st7735.h"
#include "Font_asc.h"
#include "bsp_gpio.h"
#include "bsp_tim.h"

/* ------------------------------------------------------------------*/
/* 数值格式化                                                        */
/* ------------------------------------------------------------------*/

/**
  * @brief  把浮点数格式化为定宽字符串（右对齐，不足补 0）
  * @param  value: 输入值
  * @param  out:   输出缓冲（至少 width+1 字节）
  * @param  width: 总宽度（含符号与小数点）
  * @param  prec:  小数位数
  * @note   NaN/Inf 或放不下时用 'x' 填充，防御异常值
  */
void formatFloatToStr(float value, char *out, int width, int prec)
{
    if ((out == NULL) || (width <= 0))
    {
        return;
    }

    /* NaN/Inf 防御：用 'x' 表示无效 */
    if (!isfinite(value))
    {
        memset(out, 'x', (size_t)width);
        out[width] = '\0';
        return;
    }

    int negative = signbit(value) ? 1 : 0;
    float absval = negative ? -value : value;

    long long intpart = (long long)absval;           /* 截断取整 */
    /* 手写整数转字符串：避免引入 snprintf 的大段库代码（Flash 紧张） */
    char intbuf[64];
    unsigned long long ip = (unsigned long long)intpart;
    int ipos = 63;
    intbuf[ipos] = '\0';
    if (ip == 0ULL)
    {
        intbuf[--ipos] = '0';
    }
    while (ip > 0ULL)
    {
        intbuf[--ipos] = (char)('0' + (int)(ip % 10ULL));
        ip /= 10ULL;
    }
    int intlen = 63 - ipos;
    int sign_len = negative ? 1 : 0;

    /* 整数部分都放不下：整体打 'x' */
    if (sign_len + intlen > width)
    {
        memset(out, 'x', (size_t)width);
        out[width] = '\0';
        return;
    }

    /* 计算实际能放的小数位数 */
    int max_frac = 0;
    if (prec > 0)
    {
        int avail = width - (sign_len + intlen + 1); /* 预留小数点 */
        max_frac = (avail > 0) ? avail : 0;
        if (max_frac > prec)
        {
            max_frac = prec;
        }
    }

    int needed = sign_len + intlen + (max_frac > 0 ? (1 + max_frac) : 0);
    int pad_zeros = (width > needed) ? (width - needed) : 0;

    char tmp[128];
    int pos = 0;
    if (negative)
    {
        tmp[pos++] = '-';
    }
    for (int i = 0; i < pad_zeros; ++i)
    {
        tmp[pos++] = '0';
    }
    memcpy(tmp + pos, intbuf + ipos, (size_t)intlen);
    pos += intlen;

    if (max_frac > 0)
    {
        tmp[pos++] = '.';
        float frac = absval - (float)intpart;
        if (frac < 0.0f)
        {
            frac = 0.0f;
        }
        for (int i = 0; i < max_frac; ++i)
        {
            frac *= 10.0f;
            int digit = (int)frac;
            if (digit < 0) digit = 0;
            if (digit > 9) digit = 9;
            tmp[pos++] = (char)('0' + digit);
            frac -= (float)digit;
        }
    }

    if (pos > width)
    {
        memset(out, '.', (size_t)width);
        out[width] = '\0';
        return;
    }

    memcpy(out, tmp, (size_t)pos);
    out[pos] = '\0';
}

/* ------------------------------------------------------------------*/
/* 时间格式化                                                        */
/* ------------------------------------------------------------------*/

/**
  * @brief  毫秒时间戳转 "hh:mm:ss"，超过 99 小时转 "XdXXhXXm"
  */
char* FormatTimeString(uint64_t msTicks)
{
    static char buf[20];
    uint32_t total_sec  = (uint32_t)(msTicks / 1000U);
    uint32_t total_hour = total_sec / 3600U;

    if (total_hour <= 99U)
    {
        uint32_t hh = total_hour;
        uint32_t mm = (total_sec % 3600U) / 60U;
        uint32_t ss = total_sec % 60U;
        snprintf(buf, sizeof(buf), "%02u:%02u:%02u", (unsigned)hh, (unsigned)mm, (unsigned)ss);
    }
    else
    {
        uint32_t days = total_sec / 86400U;
        uint32_t rem  = total_sec % 86400U;
        uint32_t hh   = rem / 3600U;
        uint32_t mm   = (rem % 3600U) / 60U;
        snprintf(buf, sizeof(buf), "%ud%02uh%02um", (unsigned)days, (unsigned)hh, (unsigned)mm);
    }
    return buf;
}

/* ------------------------------------------------------------------*/
/* 移动平均滤波                                                      */
/* ------------------------------------------------------------------*/

void MovingAverageInit(MovingAverageFilter *f)
{
    if (f == NULL)
    {
        return;
    }
    f->sum = 0.0f;
    f->write_idx = 0;
    f->count = 0;
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        f->buffer[i] = 0.0f;
    }
}

float MovingAverageUpdate(MovingAverageFilter *f, float value)
{
    if (f == NULL)
    {
        return value;
    }

    float old_value = f->buffer[f->write_idx];
    f->buffer[f->write_idx] = value;
    f->sum += value - old_value;
    f->write_idx = (uint8_t)((f->write_idx + 1U) % WINDOW_SIZE);
    if (f->count < WINDOW_SIZE)
    {
        f->count++;
    }
    return f->sum / (float)f->count;
}

/* ------------------------------------------------------------------*/
/* 卡尔曼滤波                                                        */
/* ------------------------------------------------------------------*/

void KalmanInit(KalmanFilter *kf, float init_x, float init_p)
{
    if (kf == NULL)
    {
        return;
    }
    kf->x_est = init_x;
    kf->p_est = init_p;
    kf->initialized = 1;
}

float KalmanUpdate(KalmanFilter *kf, float value, float Q, float R)
{
    if (kf == NULL)
    {
        return value;
    }
    if (!kf->initialized)
    {
        kf->x_est = value;
        kf->p_est = 1.0f;
        kf->initialized = 1;
        return value;
    }

    /* 预测 */
    float x_pred = kf->x_est;
    float p_pred = kf->p_est + Q;

    /* 更新 */
    float K = p_pred / (p_pred + R);
    kf->x_est = x_pred + K * (value - x_pred);
    kf->p_est = (1.0f - K) * p_pred;

    return kf->x_est;
}

/* ------------------------------------------------------------------*/
/* 分段线性校准                                                      */
/* ------------------------------------------------------------------*/

/**
  * @brief  分段线性插值校准电流，边界外自动外推（取边界偏移）
  */
float CalibrateCurrent(const CalTable *ct, float rawCurrent)
{
    if ((ct == NULL) || (ct->num_points < 1) || (ct->num_points > MAX_CAL_POINTS))
    {
        return rawCurrent;
    }

    const float *points = ct->points;
    const float *offs   = ct->offsets;
    int n = ct->num_points;

    if (rawCurrent <= points[0])
    {
        return rawCurrent + offs[0];
    }
    if (rawCurrent >= points[n - 1])
    {
        return rawCurrent + offs[n - 1];
    }

    for (int i = 0; i < n - 1; i++)
    {
        float x0 = points[i];
        float x1 = points[i + 1];
        if ((rawCurrent > x0) && (rawCurrent <= x1))
        {
            float o0 = offs[i];
            float o1 = offs[i + 1];
            float slope = (o1 - o0) / (x1 - x0);
            return rawCurrent + o0 + slope * (rawCurrent - x0);
        }
    }

    return rawCurrent; /* 理论上到不了这里 */
}

/* ------------------------------------------------------------------*/
/* 通道历史最大值                                                    */
/* ------------------------------------------------------------------*/

#define MAX_CHANNELS_MAX  8
static float max_values[MAX_CHANNELS_MAX];
static uint8_t max_inited[MAX_CHANNELS_MAX] = {0};

float MaxValue(uint8_t id, float value)
{
    if (id >= MAX_CHANNELS_MAX)
    {
        return value; /* 越界防御：原样返回 */
    }
    if (!max_inited[id])
    {
        max_values[id] = value;
        max_inited[id] = 1;
    }
    else if (value > max_values[id])
    {
        max_values[id] = value;
    }
    return max_values[id];
}

/* ------------------------------------------------------------------*/
/* 符号判定                                                          */
/* ------------------------------------------------------------------*/

const char* check_sign_str(float x, const char* neg_str, const char* pos_str)
{
    union { float f; unsigned u; } un = { x };
    return (un.u & 0x80000000U) ? neg_str : pos_str;
}

/* ------------------------------------------------------------------*/
/* 温度 -> 风扇 PWM（滞回状态机）                                     */
/* ------------------------------------------------------------------*/

#define TEMP_OFF        48.0f   /* OFF <-> SPAN 临界温度 */
#define TEMP_SPAN       55.0f   /* SPAN <-> ON 临界温度  */
#define TEMP_SET        70.0f   /* ON <-> FULL 临界温度  */

#define PWM_OFF         0       /* OFF 区占空比        */
#define PWM_SPAN_DOWN   10      /* 降温经过 SPAN 的占空比 */
#define PWM_ON          PWM_SPAN_DOWN
#define PWM_MAX         99      /* 最大占空比          */

typedef enum {
    STATE_OFF = 0,
    STATE_SPAN,
    STATE_ON
} PWM_State;

/**
  * @brief  温度转风扇占空比，带滞回：防止临界温度附近反复跳变
  */
uint8_t CalculatePWM(float temperature)
{
    /* 记住上一次离开 SPAN 区的状态，实现滞回 */
    static PWM_State lastNonSpan = STATE_OFF;

    PWM_State curState;
    if (temperature < TEMP_OFF)
    {
        curState = STATE_OFF;
    }
    else if (temperature < TEMP_SPAN)
    {
        curState = STATE_SPAN;
    }
    else
    {
        curState = STATE_ON;
    }

    if (curState != STATE_SPAN)
    {
        lastNonSpan = curState;
    }

    uint8_t pwm;
    switch (curState)
    {
        case STATE_OFF:
            pwm = PWM_OFF;
            break;

        case STATE_SPAN:
            /* 从 OFF 升上来还是从 ON 降下来，输出不同，避免风扇抖振 */
            pwm = (lastNonSpan == STATE_OFF) ? PWM_OFF : PWM_SPAN_DOWN;
            break;

        case STATE_ON:
        default:
            if (temperature < TEMP_SET)
            {
                float ratio = (temperature - TEMP_SPAN) / (TEMP_SET - TEMP_SPAN);
                pwm = (uint8_t)(PWM_ON + (PWM_MAX - PWM_ON) * ratio + 0.5f);
            }
            else
            {
                pwm = PWM_MAX;
            }
            break;
    }
    return pwm;
}

/* ------------------------------------------------------------------*/
/* 瓦时积分                                                          */
/* ------------------------------------------------------------------*/

/**
  * @brief  功率对时间积分得到累计瓦时（每 50ms 调用一次）
  */
float calc_wh(uint64_t curr_ms, float power_w)
{
    static uint64_t prev_ms = 0;
    static float total_wh = 0.0f;

    if (prev_ms == 0)
    {
        prev_ms = curr_ms;   /* 首次调用只记录时间 */
        return total_wh;
    }

    uint64_t delta_ms = curr_ms - prev_ms;
    prev_ms = curr_ms;
    float delta_h = (float)delta_ms / 3600000.0f;
    total_wh += power_w * delta_h;
    return total_wh;
}

/* ------------------------------------------------------------------*/
/* NTC 电压 -> 温度                                                  */
/* ------------------------------------------------------------------*/

#define VCC         3.3f
#define B_VALUE     3950.0f
#define R0          10000.0f
#define T0_KELVIN   298.15f

/* 分压方案：1=上拉（NTC 接地），2=下拉（NTC 接电源） */
#define NTC_SCHEME  1

/* ln(x) 查表：覆盖 NTC 阻值比 R/R0 的常见范围（约 -40°C~125°C），
   表内线性插值、表外按端点斜率外推。避免链接浮点数学库 logf。 */
static const float LN_X_TAB[] = {
    0.03f, 0.05f, 0.08f, 0.12f, 0.20f, 0.32f, 0.50f, 0.80f,
    1.20f, 1.80f, 2.80f, 4.50f, 7.00f, 11.0f, 17.0f, 30.0f
};
static const float LN_Y_TAB[] = {
    -3.50656f, -2.99573f, -2.52573f, -2.12026f, -1.60944f, -1.13943f, -0.69315f, -0.22314f,
    0.18232f, 0.58779f, 1.02962f, 1.50408f, 1.94591f, 2.39790f, 2.83321f, 3.40120f
};

static float fast_ln(float x)
{
    const int N = (int)(sizeof(LN_X_TAB) / sizeof(LN_X_TAB[0]));

    if (x <= LN_X_TAB[0])
    {
        float slope = (LN_Y_TAB[1] - LN_Y_TAB[0]) / (LN_X_TAB[1] - LN_X_TAB[0]);
        return LN_Y_TAB[0] + slope * (x - LN_X_TAB[0]);
    }
    if (x >= LN_X_TAB[N - 1])
    {
        float slope = (LN_Y_TAB[N - 1] - LN_Y_TAB[N - 2]) / (LN_X_TAB[N - 1] - LN_X_TAB[N - 2]);
        return LN_Y_TAB[N - 1] + slope * (x - LN_X_TAB[N - 1]);
    }
    for (int i = 0; i < N - 1; i++)
    {
        if (x <= LN_X_TAB[i + 1])
        {
            float slope = (LN_Y_TAB[i + 1] - LN_Y_TAB[i]) / (LN_X_TAB[i + 1] - LN_X_TAB[i]);
            return LN_Y_TAB[i] + slope * (x - LN_X_TAB[i]);
        }
    }
    return 0.0f;
}

#if (NTC_SCHEME == 1)
#define R_PULLUP    10000.0f

float Voltage_To_Temperature(float voltage)
{
    if ((voltage <= 0.0f) || (voltage >= VCC))
    {
        return -273.15f; /* 异常哨兵值 */
    }
    float r_ntc = R_PULLUP * voltage / (VCC - voltage);
    float inv_T = 1.0f / T0_KELVIN + fast_ln(r_ntc / R0) / B_VALUE;
    return (1.0f / inv_T) - 273.15f;
}
#elif (NTC_SCHEME == 2)
#define R_PULLDOWN  10000.0f
float Voltage_To_Temperature(float voltage)
{
    if (voltage <= 0.0f)
    {
        return -273.15f;
    }
    float r_ntc = R_PULLDOWN * (VCC - voltage) / voltage;
    float inv_T = 1.0f / T0_KELVIN + fast_ln(r_ntc / R0) / B_VALUE;
    return (1.0f / inv_T) - 273.15f;
}
#endif

/* ------------------------------------------------------------------*/
/* 按键：中断只记时间戳，消抖和判定放 50ms 周期任务                   */
/* ------------------------------------------------------------------*/

/**
  * @brief  按键引脚号 -> 按键索引
  */
static uint8_t BtnPinToIndex(uint16_t pin)
{
    if (pin == SW_WKUP_Pin) return 0;
    if (pin == SW_FUNC_Pin) return 1;
    if (pin == SW_MODE_Pin) return 2;
    return MAX_KEYS; /* 未识别，越界值 */
}

/**
  * @brief  GPIO EXTI 通用回调（PY32 HAL 只有这一个回调，没有上升/下降沿之分）
  *
  * 中断里只做一件事：记录按键边沿时间戳，消抖与业务逻辑放到 50ms 周期任务。
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t idx = BtnPinToIndex(GPIO_Pin);
    if (idx < MAX_KEYS)
    {
        btn_last_irq[idx] = sys;
    }
}

#define DEBOUNCE_TIME 10   /* 消抖时间 ms */

static volatile uint64_t btn_press_start[MAX_KEYS]    = {0};
static volatile uint64_t btn_press_duration[MAX_KEYS] = {0};
static volatile uint8_t  btn_flag[MAX_KEYS]           = {0};
static volatile uint64_t btn_processed[MAX_KEYS]      = {0}; /* 已消费的边沿时间戳 */
static volatile uint8_t  btn_stable_state[MAX_KEYS]   = {0}; /* 消抖后的稳定电平 */

/**
  * @brief  复位全部按键状态机（STOP 唤醒后调用）
  *
  * 保留 btn_last_irq：唤醒那次按下产生的边沿时间戳仍有效，
  * 消抖模块可把它当作"新的一次按下"继续跟踪；若刚唤醒就松手，
  * 则只当作一次唤醒，不产生按键事件。
  */
void ButtonStateReset(void)
{
    for (uint8_t i = 0; i < MAX_KEYS; i++)
    {
        btn_press_start[i]    = 0;
        btn_press_duration[i] = 0;
        btn_flag[i]           = 0;
        btn_processed[i]      = 0;
        btn_stable_state[i]   = 0;
        lock[i]               = 0;
        longFlag[i]           = 0;
    }
}

/**
  * @brief  消抖处理：每个 50ms 周期任务调用
  */
void Debounce_Process(void)
{
    for (uint8_t i = 0; i < MAX_KEYS; i++)
    {
        /* 临界区快照时间戳：避免 64 位变量在中断与主循环之间被撕开读 */
        __disable_irq();
        uint64_t ts = btn_last_irq[i];
        __enable_irq();

        if ((ts == 0U) || (ts == btn_processed[i]))
        {
            continue; /* 没有新边沿 */
        }

        if ((sys - ts) >= DEBOUNCE_TIME)
        {
            /* 消抖时间到，读稳定电平 */
            uint8_t cur_level = BSP_BtnRead(i);

            if (cur_level != btn_stable_state[i])
            {
                if (cur_level == 1U)
                {
                    btn_press_start[i] = sys;   /* 按下：记录起始时刻 */
                    btn_flag[i] = 1U;
                }
                else
                {
                    btn_press_duration[i] = sys - btn_press_start[i]; /* 松开：记录时长 */
                    btn_flag[i] = 0U;
                }
                btn_stable_state[i] = cur_level;
            }

            btn_processed[i] = ts; /* 消费本次边沿（新边沿会覆盖 btn_last_irq，自然重启消抖） */
        }
    }
}

/* 按键判定阈值与返回值 */
#define LONG_PRESS_THRESHOLD   1000   /* 长按阈值 ms */
#define DOUBLE_CLICK_THRESHOLD 350    /* 双击间隔 ms */

/**
  * @brief  按键状态机：短按 / 长按 / 双击
  * @param  num: 按键索引 0~2
  * @retval BTN_NONE/BTN_LONG/BTN_SHORT/BTN_DOUBLE
  * @note   每 50ms 调用一次；一次按压只返回一次事件，之后需要 unlock
  */
uint8_t btnLogic(uint8_t num)
{
    if (num >= MAX_KEYS)
    {
        return BTN_NONE;
    }

    static uint8_t lastValue[MAX_KEYS]       = {BTN_NONE};
#if BTN_DOUBLE_CLICK_ENABLE
    static uint8_t pendingShort[MAX_KEYS]    = {0};
    static uint64_t lastReleaseTime[MAX_KEYS] = {0};
#endif

    /* 临界区保护：按键状态由中断与主循环共享 */
    __disable_irq();
    uint8_t  btEN       = btn_flag[num];
    uint64_t sys_inline = sys;
    uint64_t btn_start  = btn_press_start[num];
    uint64_t duration   = sys_inline - btn_start;
    __enable_irq();

    if (lock[num])
    {
        return BTN_NONE; /* 事件已被上层消费 */
    }

    /* 1) 长按：按下未松开且超过阈值，只触发一次 */
    if (btEN && (duration >= LONG_PRESS_THRESHOLD) && !longFlag[num])
    {
        lock[num] = 1U;
        lastValue[num] = BTN_LONG;
        longFlag[num] = 1U;
        return lastValue[num];
    }

    /* 2) 短按/双击：松开后按持续时间判断 */
    if (!btEN && (btn_press_duration[num] > 0))
    {
        if (btn_press_duration[num] < LONG_PRESS_THRESHOLD)
        {
#if BTN_DOUBLE_CLICK_ENABLE
            uint64_t now = sys_inline;
            if (pendingShort[num] && ((now - lastReleaseTime[num]) <= DOUBLE_CLICK_THRESHOLD))
            {
                /* 第二次短按 -> 双击 */
                pendingShort[num] = 0;
                lock[num] = 1U;
                lastValue[num] = BTN_DOUBLE;
                btn_press_duration[num] = 0;
                return lastValue[num];
            }
            /* 第一次短按：先挂起，等双击窗口过去再报短按 */
            pendingShort[num] = 1U;
            lastReleaseTime[num] = now;
            btn_press_duration[num] = 0;
            return BTN_NONE;
#else
            /* 双击已关闭：松开即报短按，省去等待窗口，单按效率更高 */
            btn_press_duration[num] = 0;
            lock[num] = 1U;
            lastValue[num] = BTN_SHORT;
            return lastValue[num];
#endif
        }
        /* 松开时长按：不报事件，只清状态 */
        btn_press_duration[num] = 0;
        longFlag[num] = 0;
        return BTN_NONE;
    }

#if BTN_DOUBLE_CLICK_ENABLE
    /* 3) 挂起的短按超时 -> 确认短按 */
    if (pendingShort[num] && ((sys_inline - lastReleaseTime[num]) > DOUBLE_CLICK_THRESHOLD))
    {
        pendingShort[num] = 0;
        lock[num] = 1U;
        lastValue[num] = BTN_SHORT;
        return lastValue[num];
    }
#endif

    return BTN_NONE;
}

/**
  * @brief  带外部长按标志的按键状态机（长按可重复触发）
  */
uint8_t btnLogic_withExtLongFlag(uint8_t num)
{
    if (num >= MAX_KEYS)
    {
        return BTN_NONE;
    }

    static uint8_t lastValue[MAX_KEYS]        = {BTN_NONE};
#if BTN_DOUBLE_CLICK_ENABLE
    static uint8_t pendingShort[MAX_KEYS]     = {0};
    static uint64_t lastReleaseTime[MAX_KEYS] = {0};
#endif

    __disable_irq();
    uint8_t  btEN       = btn_flag[num];
    uint64_t sys_inline = sys;
    uint64_t btn_start  = btn_press_start[num];
    uint64_t duration   = sys_inline - btn_start;
    __enable_irq();

    if (lock[num])
    {
        return BTN_NONE;
    }

    if (btEN && (duration >= LONG_PRESS_THRESHOLD) && !longFlag[num])
    {
        lock[num] = 1U;
        lastValue[num] = BTN_LONG;
        /* 与 btnLogic 不同：不在这里置 longFlag，由外部控制重复触发 */
        return lastValue[num];
    }

    if (!btEN && (btn_press_duration[num] > 0))
    {
        if (btn_press_duration[num] < LONG_PRESS_THRESHOLD)
        {
#if BTN_DOUBLE_CLICK_ENABLE
            uint64_t now = sys_inline;
            if (pendingShort[num] && ((now - lastReleaseTime[num]) <= DOUBLE_CLICK_THRESHOLD))
            {
                pendingShort[num] = 0;
                lock[num] = 1U;
                lastValue[num] = BTN_DOUBLE;
                btn_press_duration[num] = 0;
                return lastValue[num];
            }
            pendingShort[num] = 1U;
            lastReleaseTime[num] = now;
            btn_press_duration[num] = 0;
            return BTN_NONE;
#else
            /* 双击已关闭：松开即报短按，单按效率更高 */
            btn_press_duration[num] = 0;
            lock[num] = 1U;
            lastValue[num] = BTN_SHORT;
            return lastValue[num];
#endif
        }
        btn_press_duration[num] = 0;
        longFlag[num] = 0;
        return BTN_NONE;
    }

#if BTN_DOUBLE_CLICK_ENABLE
    if (pendingShort[num] && ((sys_inline - lastReleaseTime[num]) > DOUBLE_CLICK_THRESHOLD))
    {
        pendingShort[num] = 0;
        lock[num] = 1U;
        lastValue[num] = BTN_SHORT;
        return lastValue[num];
    }
#endif

    return BTN_NONE;
}

/* ------------------------------------------------------------------*/
/* 低功耗：STOP + 按键唤醒                                            */
/* ------------------------------------------------------------------*/

/**
  * @brief  关机：清屏 -> 关负载 -> 进 STOP，SW_WKUP 按键唤醒
  *
  * 注意（与 G030 原工程的差异）：
  *   PY32F002B 没有 STANDBY 模式，只有 STOP。STOP 唤醒后外设寄存器全部保留，
  *   时钟自动回到 HSI，因此唤醒后无需重新初始化，主循环可直接继续运行。
  *   需要更低 STOP 电流时，可再逐引脚切模拟输入并做唤醒重建。
  */
void power_off(void)
{
    LCD_Clear(BLACK);
    LCD_BLK(100);
    LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 28, RED, BLACK, "POWEROFF");

    /* 软件延时约 1 秒，让用户看清 POWEROFF 提示 */
    for (volatile uint32_t i = 0; i < 6000000U; i++)
    {
        __NOP();
    }

    /* 先关屏幕与负载，降低进入 STOP 前的功耗 */
    LCD_BLK(0);
    BSP_PWM_SetFan(0);
    LCD_Clear(BLACK);

    /* 1. 收窄唤醒源：只允许 SW_WKUP 的"按下"（上升沿）唤醒 STOP。
       长按关机时按键仍被按住，松手的下降沿不会再唤醒芯片；
       另外两个按键的 EXTI 关闭，避免误唤醒 */
    BSP_BtnWakeConfig();

    /* 2. 清按键 EXTI 挂起位：手册要求 pending 清零才能进低功耗 */
    EXTI->PR = (SW_WKUP_Pin | SW_FUNC_Pin | SW_MODE_Pin);

    /* 3. 停 SysTick 并清掉可能已挂起的 SysTick 中断：
       否则 WFI 很可能被 1ms 一次的 SysTick pending 立即唤醒 */
    HAL_SuspendTick();
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;

    /* 4. 进入 STOP（LPR 供电，WFI）：
       只有再次按下 SW_WKUP 才会醒来，其余外设状态全部保留 */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* 5. 唤醒后恢复：SysTick、三键双沿中断、按键状态机复位。
       ButtonStateReset 保留唤醒边沿时间戳，唤醒这次按键可被正常跟踪；
       长按状态清零，避免"还按着就再次触发关机"的循环 */
    HAL_ResumeTick();
    BSP_BtnRestoreConfig();
    ButtonStateReset();
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

