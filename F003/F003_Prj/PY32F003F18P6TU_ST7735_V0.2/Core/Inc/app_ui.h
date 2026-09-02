/**
  ******************************************************************************
  * @file    app_ui.h
  * @author  Bowen (wbw20)
  * @date    2026-08-31
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   设置菜单（二级界面）接口：UI 状态机 + 按键分发 + 菜单渲染
  *
  * 设计说明：
  *   - UI_STATE 供 main.c 分流：主界面画仪表盘，菜单/数值编辑画设置页；
  *   - APP_UI_HandleKeys 在 50ms 任务里调用，按当前状态解释三个按键；
  *   - APP_UI_Draw 在 FLAG 整屏重绘且不在主界面时调用。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-31) 首次创建。
  ******************************************************************************
  */

#ifndef __APP_UI_H__
#define __APP_UI_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI 状态机 */
#define UI_MAIN   0   /* 主界面   */
#define UI_MENU   1   /* 设置菜单 */
#define UI_VALUE  2   /* 数值编辑 */

extern volatile uint8_t UI_STATE;   /* 当前 UI 状态，main.c 据此分流 */

void APP_UI_HandleKeys(uint8_t bt_up, uint8_t bt_set, uint8_t bt_down);
void APP_UI_Draw(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_UI_H__ */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
