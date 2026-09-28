# crcx

参数化 CRC 计算库（C99）：模型定义对齐 CRC catalogue（reveng）规范，
8 / 16 / 32 位通吃，支持流式逐字节计算。

## 特性

- **六元组参数化模型**：width / poly / init / refin / refout / xorout，
  理论上能表达 catalogue 里的任何标准 CRC
- **流式计算**：串口逐字节收也能边收边算，结果与一次性计算完全一致
- **零查表实现**：逐位算法，不占查表 RAM/flash；速度换内存，单片机友好
- 五个常用预置模型随库附带，全部用 catalogue 权威校验值做过单元测试
- 零依赖，`-Wall -Wextra -Werror` 零警告

## 预置模型

| 模型 | 常量 | `"123456789"` 校验值 |
| --- | --- | --- |
| CRC-8 | `crcx_crc8` | `0xF4` |
| CRC-16/MODBUS | `crcx_crc16_modbus` | `0x4B37` |
| CRC-16/CCITT-FALSE | `crcx_crc16_ccitt_false` | `0x29B1` |
| CRC-16/XMODEM | `crcx_crc16_xmodem` | `0x31C3` |
| CRC-32/ISO-HDLC（zlib 同款） | `crcx_crc32` | `0xCBF43926` |

## 使用

一次性：

```c
#include "crcx/crcx.h"

uint32_t crc = crcx_calc(&crcx_crc16_modbus, data, len);
```

流式（和一次性结果严格一致，测试保证）：

```c
crcx_ctx_t ctx;
crcx_begin(&ctx, &crcx_crc32);
while ((n = uart_read(buf, sizeof(buf))) > 0) {
    crcx_update(&ctx, buf, n);
}
uint32_t crc = crcx_finish(&ctx);
```

自定义模型：

```c
const crcx_model_t my_model = {
    .width = 16, .poly = 0x1021, .init = 0x1D0F,
    .refin = false, .refout = false, .xorout = 0x0000,
};
```

## 编译与测试

```bash
make test
```

```bash
# 没有 make 就手动编译运行：
gcc -std=c99 -Wall -Wextra -Werror -O2 -Iinclude tests/test_crcx.c src/crcx.c -o test_crcx
./test_crcx
```

## 设计取舍

逐位实现每字节 8 次循环，比查表法慢约 8 倍，但换来：
不占 512 字节表空间（CRC32）、任何位宽统一代码路径、没有「表生成时机」问题。
在 72MHz 的 STM32F103 上算几百字节的 CRC16 是亚毫秒级，多数串口场景够用。

## License

MIT
