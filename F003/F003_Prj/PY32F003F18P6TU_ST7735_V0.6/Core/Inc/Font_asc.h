#ifndef __Font_asc_H
#define __Font_asc_H



#include <stdint.h>

/* ================= 字模表 Sample =================
   注：h48w64/h32w16/h12w6 三种未使用字号已裁剪，
       PY32F002B 只有 24KB Flash，保留全部字号放不下 */
extern const char h24w12_sample[];
extern const char h16w8_sample[];
extern const char h30w27_sample[];
extern const char h26w23_sample[];
extern const char h22w20_sample[];
extern const char h18w16_sample[];







/* ================= 字模数据 ================= */
extern const unsigned char h24w12[];
extern const unsigned char h16w8[];
extern const unsigned char h30w27[];
extern const unsigned char h26w23[];
extern const unsigned char h22w20[];
extern const unsigned char h18w16[];


#endif


