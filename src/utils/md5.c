/**
 * @file md5.c
 * @brief Implementação do algoritmo MD5
 */

#include "md5.h"
#include <string.h>

// Constantes para transformação MD5
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// Funções auxiliares
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define FF(a, b, c, d, x, s, ac)                        \
    {                                                   \
        (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac); \
        (a) = ROTATE_LEFT((a), (s));                    \
        (a) += (b);                                     \
    }

#define GG(a, b, c, d, x, s, ac)                        \
    {                                                   \
        (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac); \
        (a) = ROTATE_LEFT((a), (s));                    \
        (a) += (b);                                     \
    }

#define HH(a, b, c, d, x, s, ac)                        \
    {                                                   \
        (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac); \
        (a) = ROTATE_LEFT((a), (s));                    \
        (a) += (b);                                     \
    }

#define II(a, b, c, d, x, s, ac)                        \
    {                                                   \
        (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac); \
        (a) = ROTATE_LEFT((a), (s));                    \
        (a) += (b);                                     \
    }

// Constantes da tabela T
static const uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

static void MD5Transform(uint32_t state[4], const uint8_t block[64]);
static void Encode(uint8_t *output, const uint32_t *input, size_t len);
static void Decode(uint32_t *output, const uint8_t *input, size_t len);

void MD5_Init(MD5_CTX *context)
{
    if (!context)
        return;

    context->count[0] = context->count[1] = 0;

    // Carrega constantes mágicas
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

void MD5_Update(MD5_CTX *context, const void *input, size_t inputLen)
{
    if (!context || !input)
        return;

    const uint8_t *in = (const uint8_t *)input;
    size_t i, index, partLen;

    // Calcula número de bytes mod 64
    index = (size_t)((context->count[0] >> 3) & 0x3F);

    // Atualiza número de bits
    if ((context->count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((uint32_t)inputLen >> 29);

    partLen = 64 - index;

    // Transforma quantas vezes possível
    if (inputLen >= partLen)
    {
        memcpy(&context->buffer[index], in, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform(context->state, &in[i]);

        index = 0;
    }
    else
        i = 0;

    // Buffer restante
    memcpy(&context->buffer[index], &in[i], inputLen - i);
}

void MD5_Final(unsigned char digest[16], MD5_CTX *context)
{
    if (!context || !digest)
        return;

    static uint8_t PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t bits[8];
    size_t index, padLen;

    // Salva número de bits
    Encode(bits, context->count, 8);

    // Padding até 56 mod 64
    index = (size_t)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5_Update(context, PADDING, padLen);

    // Append length
    MD5_Update(context, bits, 8);

    // Store state in digest
    Encode(digest, context->state, 16);

    // Zera informação sensível
    memset(context, 0, sizeof(*context));
}

static void MD5Transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    // Round 1
    FF(a, b, c, d, x[0], S11, T[0]);   // 1
    FF(d, a, b, c, x[1], S12, T[1]);   // 2
    FF(c, d, a, b, x[2], S13, T[2]);   // 3
    FF(b, c, d, a, x[3], S14, T[3]);   // 4
    FF(a, b, c, d, x[4], S11, T[4]);   // 5
    FF(d, a, b, c, x[5], S12, T[5]);   // 6
    FF(c, d, a, b, x[6], S13, T[6]);   // 7
    FF(b, c, d, a, x[7], S14, T[7]);   // 8
    FF(a, b, c, d, x[8], S11, T[8]);   // 9
    FF(d, a, b, c, x[9], S12, T[9]);   // 10
    FF(c, d, a, b, x[10], S13, T[10]); // 11
    FF(b, c, d, a, x[11], S14, T[11]); // 12
    FF(a, b, c, d, x[12], S11, T[12]); // 13
    FF(d, a, b, c, x[13], S12, T[13]); // 14
    FF(c, d, a, b, x[14], S13, T[14]); // 15
    FF(b, c, d, a, x[15], S14, T[15]); // 16

    // Round 2
    GG(a, b, c, d, x[1], S21, T[16]);  // 17
    GG(d, a, b, c, x[6], S22, T[17]);  // 18
    GG(c, d, a, b, x[11], S23, T[18]); // 19
    GG(b, c, d, a, x[0], S24, T[19]);  // 20
    GG(a, b, c, d, x[5], S21, T[20]);  // 21
    GG(d, a, b, c, x[10], S22, T[21]); // 22
    GG(c, d, a, b, x[15], S23, T[22]); // 23
    GG(b, c, d, a, x[4], S24, T[23]);  // 24
    GG(a, b, c, d, x[9], S21, T[24]);  // 25
    GG(d, a, b, c, x[14], S22, T[25]); // 26
    GG(c, d, a, b, x[3], S23, T[26]);  // 27
    GG(b, c, d, a, x[8], S24, T[27]);  // 28
    GG(a, b, c, d, x[13], S21, T[28]); // 29
    GG(d, a, b, c, x[2], S22, T[29]);  // 30
    GG(c, d, a, b, x[7], S23, T[30]);  // 31
    GG(b, c, d, a, x[12], S24, T[31]); // 32

    // Round 3
    HH(a, b, c, d, x[5], S31, T[32]);  // 33
    HH(d, a, b, c, x[8], S32, T[33]);  // 34
    HH(c, d, a, b, x[11], S33, T[34]); // 35
    HH(b, c, d, a, x[14], S34, T[35]); // 36
    HH(a, b, c, d, x[1], S31, T[36]);  // 37
    HH(d, a, b, c, x[4], S32, T[37]);  // 38
    HH(c, d, a, b, x[7], S33, T[38]);  // 39
    HH(b, c, d, a, x[10], S34, T[39]); // 40
    HH(a, b, c, d, x[13], S31, T[40]); // 41
    HH(d, a, b, c, x[0], S32, T[41]);  // 42
    HH(c, d, a, b, x[3], S33, T[42]);  // 43
    HH(b, c, d, a, x[6], S34, T[43]);  // 44
    HH(a, b, c, d, x[9], S31, T[44]);  // 45
    HH(d, a, b, c, x[12], S32, T[45]); // 46
    HH(c, d, a, b, x[15], S33, T[46]); // 47
    HH(b, c, d, a, x[2], S34, T[47]);  // 48

    // Round 4
    II(a, b, c, d, x[0], S41, T[48]);  // 49
    II(d, a, b, c, x[7], S42, T[49]);  // 50
    II(c, d, a, b, x[14], S43, T[50]); // 51
    II(b, c, d, a, x[5], S44, T[51]);  // 52
    II(a, b, c, d, x[12], S41, T[52]); // 53
    II(d, a, b, c, x[3], S42, T[53]);  // 54
    II(c, d, a, b, x[10], S43, T[54]); // 55
    II(b, c, d, a, x[1], S44, T[55]);  // 56
    II(a, b, c, d, x[8], S41, T[56]);  // 57
    II(d, a, b, c, x[15], S42, T[57]); // 58
    II(c, d, a, b, x[6], S43, T[58]);  // 59
    II(b, c, d, a, x[13], S44, T[59]); // 60
    II(a, b, c, d, x[4], S41, T[60]);  // 61
    II(d, a, b, c, x[11], S42, T[61]); // 62
    II(c, d, a, b, x[2], S43, T[62]);  // 63
    II(b, c, d, a, x[9], S44, T[63]);  // 64

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    // Zera informação sensível
    memset(x, 0, sizeof(x));
}

static void Encode(uint8_t *output, const uint32_t *input, size_t len)
{
    size_t i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j] = (uint8_t)(input[i] & 0xff);
        output[j + 1] = (uint8_t)((input[i] >> 8) & 0xff);
        output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
        output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
    }
}

static void Decode(uint32_t *output, const uint8_t *input, size_t len)
{
    size_t i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) |
                    (((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
}
