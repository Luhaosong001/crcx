/* crcx 单元测试。校验值全部取自 CRC catalogue 的权威 "123456789" 测试向量。 */

#include <stdio.h>

#include "crcx/crcx.h"

static int failed = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        if (!(cond)) {                                                 \
            printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);     \
            failed++;                                                  \
        }                                                              \
    } while (0)

static const uint8_t CHECK_SEQ[] = "123456789"; /* 不含 NUL 共 9 字节 */

static void test_preset_check_values(void)
{
    CHECK(crcx_calc(&crcx_crc8, CHECK_SEQ, sizeof(CHECK_SEQ) - 1) == 0xF4u);
    CHECK(crcx_calc(&crcx_crc16_modbus, CHECK_SEQ, sizeof(CHECK_SEQ) - 1) == 0x4B37u);
    CHECK(crcx_calc(&crcx_crc16_ccitt_false, CHECK_SEQ, sizeof(CHECK_SEQ) - 1) == 0x29B1u);
    CHECK(crcx_calc(&crcx_crc16_xmodem, CHECK_SEQ, sizeof(CHECK_SEQ) - 1) == 0x31C3u);
    CHECK(crcx_calc(&crcx_crc32, CHECK_SEQ, sizeof(CHECK_SEQ) - 1) == 0xCBF43926u);
}

static void test_empty_input(void)
{
    /* 空输入 = 只做 init 和 xorout 的组合 */
    CHECK(crcx_calc(&crcx_crc8, CHECK_SEQ, 0) == 0x00u);
    CHECK(crcx_calc(&crcx_crc16_modbus, CHECK_SEQ, 0) == 0xFFFFu);
    CHECK(crcx_calc(&crcx_crc32, CHECK_SEQ, 0) == 0x00000000u);
}

static void test_single_byte(void)
{
    /* 单字节 "A"：0xFFFF^0x41 右移 8 步终值 0x707F，
     * 已用独立教科书实现交叉验证 */
    CHECK(crcx_calc(&crcx_crc16_modbus, (const uint8_t *)"A", 1) == 0x707Fu);
}

static void test_streaming_matches_oneshot(void)
{
    /* 逐字节流式喂入，结果必须和一次性计算完全一致 */
    uint8_t buf[256];
    uint32_t seed = 0x12345678u; /* 固定种子，结果可重复 */
    for (size_t i = 0; i < sizeof(buf); i++) {
        seed = seed * 1664525u + 1013904223u; /* LCG 造伪随机数据 */
        buf[i] = (uint8_t)(seed >> 24);
    }

    const crcx_model_t *models[] = {
        &crcx_crc8, &crcx_crc16_modbus, &crcx_crc16_ccitt_false,
        &crcx_crc16_xmodem, &crcx_crc32,
    };

    for (int mi = 0; mi < 5; mi++) {
        uint32_t oneshot = crcx_calc(models[mi], buf, sizeof(buf));

        crcx_ctx_t ctx;
        crcx_begin(&ctx, models[mi]);
        /* 逐字节喂 */
        for (size_t i = 0; i < sizeof(buf); i++) {
            crcx_update(&ctx, &buf[i], 1);
        }
        CHECK(crcx_finish(&ctx) == oneshot);

        /* 再按 7 字节一块喂（测试非整除分块） */
        crcx_begin(&ctx, models[mi]);
        for (size_t i = 0; i < sizeof(buf); i += 7) {
            size_t chunk = (sizeof(buf) - i < 7) ? (sizeof(buf) - i) : 7;
            crcx_update(&ctx, buf + i, chunk);
        }
        CHECK(crcx_finish(&ctx) == oneshot);
    }
}

static void test_reflect(void)
{
    CHECK(crcx_reflect(0x01u, 8) == 0x80u);
    CHECK(crcx_reflect(0x8005u, 16) == 0xA001u);
    CHECK(crcx_reflect(0xFFFFFFFFu, 32) == 0xFFFFFFFFu);
    CHECK(crcx_reflect(0x12345678u, 32) == 0x1E6A2C48u);
}

int main(void)
{
    test_preset_check_values();
    test_empty_input();
    test_single_byte();
    test_streaming_matches_oneshot();
    test_reflect();

    if (failed) {
        printf("FAILED: %d check(s)\n", failed);
        return 1;
    }
    printf("ALL PASSED\n");
    return 0;
}
