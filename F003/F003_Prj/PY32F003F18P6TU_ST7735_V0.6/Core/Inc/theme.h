/**
  ******************************************************************************
  * @file    theme.h
  * @author  Bowen (wbw20)
  * @date    2026-09-07
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   主界面主题渲染接口
  *
  * 设计说明：
  *   - 主题只作用于主界面；设置/数值/选项页固定深色底，不走这里；
  *   - Theme_RenderFrame 在整屏重绘（FLAG）时调用；
  *   - Theme_RenderValues 在主界面每圈数值刷新时调用。
  *
  * CHANGELOG:
  *   V1.0 (2026-09-07) 从 main.c 拆分主题渲染到独立模块。
  ******************************************************************************
  */

#ifndef __THEME_H__
#define __THEME_H__

#ifdef __cplusplus
extern "C" {
#endif

void Theme_RenderFrame(void);   /* 主题相关整帧：清屏/标签/进度条 */
void Theme_RenderValues(void);  /* 主题相关每圈：进度条增量 + 数值显示 */

#ifdef __cplusplus
}
#endif

#endif /* __THEME_H__ */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
