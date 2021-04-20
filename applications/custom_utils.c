//
// Created by ythan on 2021/4/8.
//


#include "custom_utils.h"


void uint32ToUint8(uint32_t in, uint8_t* out)
{
    for (uint8_t i = 0; i < 4; i++) { out[i] = (in >> (i * 8)) & 0xFF; }
}
void uint8ToUint32(const uint8_t* in, uint32_t* out)
{
    *out = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        *out = *out | ((uint32_t)in[i] << (i * 8));
    }
}

void floatToUint8(float in, uint8_t* out)
{
    union
    {
        float f;
        uint32_t ui;
    } u = {.f = in};

    uint32ToUint8(u.ui, out);
}
void uint8ToFloat(const uint8_t* in, float* out)
{
    uint32_t ui;
    uint8ToUint32(in, &ui);

    union
    {
        float f;
        uint32_t ui;
    } u = {.ui = ui};

    *out = u.f;
}


float uPow(float base, int exp)
{
    float result = 1.0f;
    if (exp > 0)
    {
        for (int i = 0; i < exp; ++i) { result *= base; }
    }
    else if (exp < 0)
    {
        for (int i = 0; i > exp; --i) { result *= base; }
        result = 1.0f / result;
    }

    return result;
}
