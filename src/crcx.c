#include "crcx/crcx.h"

/* 预置模型，参数全部来自 CRC catalogue */
const crcx_model_t crcx_crc8 = {
    8, 0x07u, 0x00u, 0x00u, false, false,
};
const crcx_model_t crcx_crc16_modbus = {
    16, 0x8005u, 0xFFFFu, 0x0000u, true, true,
};
const crcx_model_t crcx_crc16_ccitt_false = {
    16, 0x1021u, 0xFFFFu, 0x0000u, false, false,
};
const crcx_model_t crcx_crc16_xmodem = {
    16, 0x1021u, 0x0000u, 0x0000u, false, false,
};
const crcx_model_t crcx_crc32 = {
    32, 0x04C11DB7u, 0xFFFFFFFFu, 0xFFFFFFFFu, true, true,
};

static uint32_t mask_of(uint8_t width)
{
    return (width >= 32) ? 0xFFFFFFFFu : ((1u << width) - 1u);
}

uint32_t crcx_reflect(uint32_t value, uint8_t bits)
{
    uint32_t out = 0;
    for (uint8_t i = 0; i < bits; i++) {
        if (value & 1u) {
            out |= 1u << (bits - 1u - i);
        }
        value >>= 1;
    }
    return out;
}

void crcx_begin(crcx_ctx_t *ctx, const crcx_model_t *model)
{
    ctx->model = *model;
    /* 反射模型（refin=true）用反转多项式右移实现，业界标准（MODBUS、zlib）
     * 都是 init 直接进寄存器、不预反转 */
    ctx->crc = model->init;
}

void crcx_update(crcx_ctx_t *ctx, const uint8_t *data, size_t n)
{
    uint32_t crc = ctx->crc;
    const crcx_model_t *m = &ctx->model;

    if (m->refin) {
        /* 反射路径：反转后的多项式，低位异或右移 */
        uint32_t poly_r = crcx_reflect(m->poly, m->width);
        for (size_t i = 0; i < n; i++) {
            crc ^= data[i];
            for (int b = 0; b < 8; b++) {
                crc = (crc & 1u) ? ((crc >> 1) ^ poly_r) : (crc >> 1);
            }
        }
    } else {
        /* 正常路径：字节左对齐到寄存器高位，高位异或左移 */
        uint32_t msb = 1u << (m->width - 1u);
        uint32_t mask = mask_of(m->width);
        uint8_t shift = (uint8_t)(m->width - 8u);
        for (size_t i = 0; i < n; i++) {
            crc ^= (uint32_t)data[i] << shift;
            for (int b = 0; b < 8; b++) {
                crc = (crc & msb) ? (((crc << 1) ^ m->poly) & mask)
                                  : ((crc << 1) & mask);
            }
        }
    }
    ctx->crc = crc;
}

uint32_t crcx_finish(crcx_ctx_t *ctx)
{
    const crcx_model_t *m = &ctx->model;
    uint32_t crc = ctx->crc & mask_of(m->width);
    /* refout=true 的输出反转已隐含在「反转多项式右移」里（MODBUS、zlib 的
     * 标准做法）；只有 refin 和 refout 不一致时才需要补一次显式反转 */
    if (m->refout != m->refin) {
        crc = crcx_reflect(crc, m->width);
    }
    return (crc ^ m->xorout) & mask_of(m->width);
}

uint32_t crcx_calc(const crcx_model_t *model, const uint8_t *data, size_t n)
{
    crcx_ctx_t ctx;
    crcx_begin(&ctx, model);
    crcx_update(&ctx, data, n);
    return crcx_finish(&ctx);
}
