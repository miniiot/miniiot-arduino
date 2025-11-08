#pragma once
#include <Arduino.h>
#include <cstdlib> // for rand()

// for sha1
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// SHA-1上下文结构体（线程安全：每个任务单独创建，不共享）
typedef struct {
    uint32_t h[5];          // 哈希中间结果
    uint32_t total_length;  // 输入数据总长度（bit）
    uint8_t  buffer[64];    // 64字节输入缓冲区（SHA-1分块大小）
    uint32_t buffer_pos;    // 缓冲区当前填充位置（0~63）
} SHA1_CTX;

// SHA-1循环左移函数
static inline uint32_t sha1_rotl(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

// SHA-1压缩函数（处理单个64字节块）
static void sha1_compress(SHA1_CTX *ctx, const uint8_t *block) {
    uint32_t w[80];
    uint32_t a, b, c, d, e, temp;
    int i;

    // 步骤1：将64字节块转换为80个32位字（大端序）
    for (i = 0; i < 16; i++) {
        w[i] = (uint32_t)block[4*i] << 24 |
               (uint32_t)block[4*i+1] << 16 |
               (uint32_t)block[4*i+2] << 8 |
               (uint32_t)block[4*i+3];
    }

    // 步骤2：扩展16字到80字
    for (i = 16; i < 80; i++) {
        w[i] = sha1_rotl(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
    }

    // 步骤3：初始化压缩变量
    a = ctx->h[0];
    b = ctx->h[1];
    c = ctx->h[2];
    d = ctx->h[3];
    e = ctx->h[4];

    // 步骤4：80轮压缩循环
    for (i = 0; i < 80; i++) {
        if (i < 20) {
            // 第一轮：f = (b & c) | (~b & d)
            temp = sha1_rotl(a, 5) + ((b & c) | (~b & d)) + e + w[i] + 0x5A827999;
        } else if (i < 40) {
            // 第二轮：f = b ^ c ^ d
            temp = sha1_rotl(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1;
        } else if (i < 60) {
            // 第三轮：f = (b & c) | (b & d) | (c & d)
            temp = sha1_rotl(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDC;
        } else {
            // 第四轮：f = b ^ c ^ d
            temp = sha1_rotl(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6;
        }

        // 变量轮转
        e = d;
        d = c;
        c = sha1_rotl(b, 30);
        b = a;
        a = temp;
    }

    // 步骤5：更新哈希中间结果
    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;
}

// 初始化SHA-1上下文
void sha1_init(SHA1_CTX *ctx) {
    memset(ctx, 0, sizeof(SHA1_CTX));
    // SHA-1初始哈希值（NIST标准）
    ctx->h[0] = 0x67452301;
    ctx->h[1] = 0xEFCDAB89;
    ctx->h[2] = 0x98BADCFE;
    ctx->h[3] = 0x10325476;
    ctx->h[4] = 0xC3D2E1F0;
}

// 分块更新SHA-1计算
void sha1_update(SHA1_CTX *ctx, const uint8_t *data, uint32_t len) {
    if (len == 0 || data == NULL) return;

    // 更新总长度（bit）
    ctx->total_length += (uint64_t)len * 8;

    // 填充缓冲区，满64字节则处理
    while (len > 0) {
        uint32_t copy_len = 64 - ctx->buffer_pos;
        if (copy_len > len) copy_len = len;

        // 复制数据到缓冲区
        memcpy(ctx->buffer + ctx->buffer_pos, data, copy_len);
        ctx->buffer_pos += copy_len;
        data += copy_len;
        len -= copy_len;

        // 缓冲区满，处理该块
        if (ctx->buffer_pos == 64) {
            sha1_compress(ctx, ctx->buffer);
            ctx->buffer_pos = 0;
        }
    }
}

// 完成SHA-1计算，输出最终哈希值
void sha1_final(SHA1_CTX *ctx, uint8_t *hash) {
    uint32_t i = ctx->buffer_pos;

    // 步骤1：添加填充位（100...0）
    ctx->buffer[i++] = 0x80;  // 先加1 bit
    while (i % 64 != 56) {    // 填充到距离64字节剩8字节（用于存储总长度）
        if (i >= 64) {
            sha1_compress(ctx, ctx->buffer);
            i = 0;
        }
        ctx->buffer[i++] = 0x00;
    }

    // 步骤2：添加总长度（64 bit，大端序）
    uint64_t total_len = ctx->total_length;
    for (int j = 7; j >= 0; j--) {
        ctx->buffer[i++] = (uint8_t)(total_len >> (8 * j));
    }

    // 步骤3：处理最后一个块
    sha1_compress(ctx, ctx->buffer);

    // 步骤4：输出哈希值（大端序，20字节）
    for (int j = 0; j < 5; j++) {
        hash[j*4] = (uint8_t)(ctx->h[j] >> 24);
        hash[j*4+1] = (uint8_t)(ctx->h[j] >> 16);
        hash[j*4+2] = (uint8_t)(ctx->h[j] >> 8);
        hash[j*4+3] = (uint8_t)(ctx->h[j]);
    }

    // 清空上下文（可选，增强安全性）
    memset(ctx, 0, sizeof(SHA1_CTX));
}

// 快捷计算：输入字符串→输出40位十六进制SHA-1
void sha1_calculate(const char *input, char *output) {
    if (input == NULL || output == NULL) return;

    SHA1_CTX ctx;
    uint8_t hash[20];

    // 计算SHA-1
    sha1_init(&ctx);
    sha1_update(&ctx, (const uint8_t *)input, strlen(input));
    sha1_final(&ctx, hash);

    // 转换为40位十六进制字符串（大写）
    for (int i = 0; i < 20; i++) {
        sprintf(output + 2*i, "%02x", hash[i]);
    }
    output[40] = '\0';  // 字符串终止符
}

// end sha1


class MiniIotUtils
{
public:
    // sha1
    static String ESPsha1(String str)
    {
        char output[41];
        sha1_calculate(str.c_str(), output);
        return String(output);
    }

    // 获取芯片ID
    static String ESPchipId()
    {
#ifdef ESP8266
        return (String)ESP.getChipId();
#endif

#ifdef ESP32
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        return (String)chipId;
#endif

#ifdef STM32F1
        uint32_t uid[3];
        uid[0] = *(__IO uint32_t *)(0x1FFFF7E8); // UID高32位
        uid[1] = *(__IO uint32_t *)(0x1FFFF7EC); // UID中32位
        uid[2] = *(__IO uint32_t *)(0x1FFFF7F0); // UID低32位
        return String(uid[0], HEX) + String(uid[1], HEX) + String(uid[2], HEX);
#endif
    }

    // 通过芯片ID生成mac地址
    static String getMacByChipId()
    {
        String chipId = ESPchipId();
        String mac = "D0:";

        for (int i = 0; i < 5; i++)
        {
            mac += String(chipId[i], HEX);
            if (i < 4)
            {
                mac += ':';
            }
        }
        return mac;
    }

    // 获取指定长度随机字符串
    static String randomString(int length)
    {
        String s;
        for (int i = 0; i < length; i++)
        {
            s += (char)(rand() % 26 + 'a');
        }
        return s;
    }

};