#ifndef CRCX_CRCX_H
#define CRCX_CRCX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* crcx：参数化 CRC 计算库（C99）。
 *
 * 模型定义对齐 CRC catalogue（reveng）规范：width / poly / init / refin /
 * refout / xorout 六元组，8~32 位通吃。
 *
 * 每个模型都用 catalogue 的权威校验值做过单元测试：
 * 字符串 "123456789" 的 CRC，例如 CRC-32 算出 0xCBF43926。
 *
 * 两种用法：
 *   一次性：crcx_calc(&model, data, n)
 *   流式  ：crcx_begin / crcx_update / crcx_finish，串口逐字节收也能算
 */

typedef struct {
    uint8_t width;    /* 位宽：8 / 16 / 32 */
    uint32_t poly;    /* 生成多项式，正常序（非反转） */
    uint32_t init;    /* 寄存器初值 */
    uint32_t xorout;  /* 输出异或值 */
    bool refin;       /* 输入字节按位反转 */
    bool refout;      /* 输出按位反转 */
} crcx_model_t;

/* 常用预置模型（const，放 flash 不占 RAM） */
extern const crcx_model_t crcx_crc8;              /* CRC-8               */
extern const crcx_model_t crcx_crc16_modbus;      /* CRC-16/MODBUS       */
extern const crcx_model_t crcx_crc16_ccitt_false; /* CRC-16/CCITT-FALSE  */
extern const crcx_model_t crcx_crc16_xmodem;      /* CRC-16/XMODEM       */
extern const crcx_model_t crcx_crc32;             /* CRC-32/ISO-HDLC     */

/* 一次性算完 */
uint32_t crcx_calc(const crcx_model_t *model, const uint8_t *data, size_t n);

/* 流式：begin 之后可以 update 任意次、每次任意长度，finish 出结果 */
typedef struct {
    crcx_model_t model;
    uint32_t crc; /* 内部寄存器（可能是反转序） */
} crcx_ctx_t;

void crcx_begin(crcx_ctx_t *ctx, const crcx_model_t *model);
void crcx_update(crcx_ctx_t *ctx, const uint8_t *data, size_t n);
uint32_t crcx_finish(crcx_ctx_t *ctx);

/* 位反转：reflect(0x01, 8) == 0x80 */
uint32_t crcx_reflect(uint32_t value, uint8_t bits);

#endif
