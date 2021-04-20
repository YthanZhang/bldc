//
// Created by ythan on 2021/4/8.
//

#ifndef BLDC_CUSTOM_UTILS_H
#define BLDC_CUSTOM_UTILS_H


#include "stdint.h"


/**
 * @brief Break 1 <b>uint32_t</b> into 4 <b>uint8_t</b> values in little-endian.
 * @param in A uint32_t value.
 * @param out A length 4 uint8_t array.
 */
void uint32ToUint8(uint32_t in, uint8_t* out);
/**
 * @brief Form 1 <b>uint32_t</b> from 4 <b>uint8_t</b> values in little-endian.
 * @param in A length 4 uint8_t array.
 * @param out A uint32_t pointer.
 */
void uint8ToUint32(const uint8_t* in, uint32_t* out);

/**
 * @brief Break 1 <b>float</b> into 4 <b>uint8_t</b> values in little-endian.
 * @param in A floating point value.
 * @param out A length 4 uint8_t array.
 */
void floatToUint8(float in, uint8_t* out);
/**
 * @brief Form 1 <b>float</b> from 4 <b>uint8_t</b> values in little-endian.
 * @param in A length 4 uint8_t array.
 * @param out A float pointer.
 */
void uint8ToFloat(const uint8_t* in, float* out);


/**
 * @brief Custom power function, the exponent must be integer.
 * @param base The base
 * @param exp The exponent
 * @return <b>base</b> ^ <b>exp</b>
 */
float uPow(float base, int exp);


#endif    // BLDC_CUSTOM_UTILS_H
