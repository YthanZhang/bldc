//
// Created by ythan on 2021/4/7.
//

#ifndef BLDC_CUSTOM_COMM_CAN_H
#define BLDC_CUSTOM_COMM_CAN_H

#include "ch.h"
#include "custom_types.h"
#include "hal.h"
#include "stdint.h"


extern CANRxFrame customRxFrame;
extern ControlStruct* comDecipherPtr;

typedef enum enum_TransferType
{
    _blank              = 0x00,
    ReportMode          = 0x01 << 0,
    ReportPosition      = 0x01 << 1,
    ReportSpeed         = 0x01 << 2,
    ReportTorque        = 0x01 << 3,
    ReportBreakStrength = 0x01 << 4,
    SetMode             = 0x01 << 5,
    SetPosition         = 0x01 << 6,
    SetSpeed            = 0x01 << 7,
    SetTorque           = 0x01 << 8,
    SetBreakStrength    = 0x01 << 9
} TransferType;


void customCanStart(void);

msg_t canVescTransmit(uint16_t id,
                      TransferType transType,
                      uint8_t dataLen,
                      uint64_t data);

#endif    // BLDC_CUSTOM_COMM_CAN_H
