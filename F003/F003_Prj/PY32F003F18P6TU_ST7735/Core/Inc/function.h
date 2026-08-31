/**
  ******************************************************************************
  * @file    function.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.1
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   通用工具函数：格式化 / 滤波 / 校准 / 按键逻辑 / 低功耗
  *
  * 设计说明：
  *   - 工具函数集中收纳，一处实现、处处复用；
  *   - 按键使用状态机 + 静态状态记忆，支持短按/长按/双击；
  *   - NTC 温度用查表/公式换算，关键阈值全部 #define。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 移植自 G030 工程 function.h；
  *   V1.1 (2026-08-19) MAX_KEYS 由 2 扩到 3；移除 PY32F002B 不存在的
  *                     RTC 备份寄存器相关接口。
  ******************************************************************************
  */

#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 按键数量：本工程 3 个按键 */
#define MAX_KEYS 3

/* 字符串格式化为定宽字段（宽度不足显示 'x'） */
void formatFloatToStr(float value, char *out, int width, int prec);

/* 毫秒时间戳转时间字符串 */
char* FormatTimeString(uint64_t msTicks);

/* ---------- 移动平均滤波 ---------- */
#define WINDOW_SIZE 20
typedef struct {
    float buffer[WINDOW_SIZE];
    float sum;
    uint8_t write_idx;
    uint8_t count;
} MovingAverageFilter;
void  MovingAverageInit(MovingAverageFilter *f);
float MovingAverageUpdate(MovingAverageFilter *f, float value);

/* ---------- 卡尔曼滤波 ---------- */
typedef struct {
    float x_est;
    float p_est;
    uint8_t initialized;
} KalmanFilter;
void  KalmanInit(KalmanFilter *kf, float init_x, float init_p);
float KalmanUpdate(KalmanFilter *kf, float value, float Q, float R);

/* ---------- 分段线性校准 ---------- */
#define MAX_CAL_POINTS 10
typedef struct {
    int id;
    int num_points;
    float points[MAX_CAL_POINTS];
    float offsets[MAX_CAL_POINTS];
} CalTable;
float CalibrateCurrent(const CalTable *ct, float rawCurrent);

/* 通道历史最大值（带防御性边界检查） */
float MaxValue(uint8_t id, float value);

/* 符号判定字符串 */
const char* check_sign_str(float x, const char* neg_str, const char* pos_str);

/* 温度 -> 风扇 PWM 占空比（滞回状态机） */
uint8_t CalculatePWM(float temperature);

/* 功率对时间积分得到瓦时 */
float calc_wh(uint64_t curr_ms, float power_w);

/* NTC 分压电压 -> 温度（支持上拉/下拉两种方案） */
float Voltage_To_Temperature(float voltage);

/* ---------- 按键状态机 ---------- */
#define BTN_NONE   0
#define BTN_LONG   1
#define BTN_SHORT  2
#define BTN_DOUBLE 3

uint8_t btnLogic(uint8_t num);
uint8_t btnLogic_withExtLongFlag(uint8_t num);
void    Debounce_Process(void);

/* ---------- 低功耗 ---------- */
void power_off(void);

#ifdef __cplusplus
}
#endif

#endif /* __FUNCTION_H__ */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

