//
// Created by ythan on 2021/4/7.
//

#include "custom_comm_can.h"

#include "ch.h"
#include "custom_utils.h"
#include "hal.h"


CANRxFrame customRxFrame;

static MUTEX_DECL(mtxCanReceive);
static MUTEX_DECL(mtxCanTransmit);

static CANConfig cancfg = {CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
                           CAN_BTR_SJW(3) | CAN_BTR_TS2(2) | CAN_BTR_TS1(9) |
                               CAN_BTR_BRP(5)};


static THD_WORKING_AREA(waCanReceiveThread, 128);
static THD_FUNCTION(canReceiveThread, arg);

static void decipherMsg(ControlStruct* decipherPtr, CANRxFrame* rxFrame);


void customCanStart()
{
    chMtxObjectInit(&mtxCanReceive);
    chMtxObjectInit(&mtxCanTransmit);

    palSetPadMode(HW_CANRX_PORT,
                  HW_CANRX_PIN,
                  PAL_MODE_ALTERNATE(HW_CAN_GPIO_AF) |
                      PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(HW_CANTX_PORT,
                  HW_CANTX_PIN,
                  PAL_MODE_ALTERNATE(HW_CAN_GPIO_AF) |
                      PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

    canStart(&HW_CAN_DEV, &cancfg);

    chThdCreateStatic(waCanReceiveThread,
                      sizeof(waCanReceiveThread),
                      NORMALPRIO + 10,
                      canReceiveThread,
                      NULL);
}


static THD_FUNCTION(canReceiveThread, arg)
{
    (void)arg;

    event_listener_t el;
    chEvtRegister(&HW_CAN_DEV.rxfull_event, &el, 1);

    CANRxFrame newReceive;

    while (!chThdShouldTerminateX())
    {
        chEvtWaitAny(EVENT_MASK(1));

        while (canReceive(&HW_CAN_DEV,
                          CAN_ANY_MAILBOX,
                          &newReceive,
                          TIME_IMMEDIATE) == MSG_OK)
        {
            chMtxLock(&mtxCanReceive);
            decipherMsg(comDecipherPtr, &newReceive);
            chMtxUnlock(&mtxCanReceive);
        }
    }
}


/**
 * @brief Send a message through CAN bus that is read by vesc or its host.
 * @note <b>id</b> and <b>transType</b> are combined into <b>EID</b> with
 *       <b>id</b> taking the first 16-bits, <b>transType</b> taking the last
 *       16-bits.
 *
 * @param id The first 16-bits of CAN EID of the message.
 * @param transType The last 16-bits of CAN EID of the message.
 * @param dataLen length of the data, min <b>0</b>, max <b>8</b>.
 * @param data The data to send. Its length should not exceed (8 *
 *             <b>dataLen</b>).
 *
 * @return The result of transmit. <b>MSG_OK</b> means the transmit was
 *         successful, <b>MSG_TIMEOUT</b> means the transmit timed out before
 *         it was able to send the message.
 */
msg_t canVescTransmit(uint16_t id,
                  TransferType transType,
                  uint8_t dataLen,
                  uint64_t data)
{
    dataLen > 8 ? (dataLen = 8) : 0;    // max CAN bus data length is 8 bytes

    CANTxFrame txFrame;
    txFrame.IDE = CAN_IDE_EXT;
    txFrame.EID = (uint16_t)(id << 16) | (uint16_t)(transType);
    txFrame.DLC = dataLen;

    dataLen ? (txFrame.RTR = CAN_RTR_DATA) : (txFrame.RTR = CAN_RTR_REMOTE);

    for (uint8_t i = 0; i < dataLen; i++)
    {
        /*The data is split in little-endian order*/
        txFrame.data8[i] = (uint8_t)((data >> (i * 8)) & 0xFF);
    }

    chMtxLock(&mtxCanTransmit);
    msg_t rslt = canTransmit(&HW_CAN_DEV, CAN_ANY_MAILBOX, &txFrame, US2ST(500));
    chMtxUnlock(&mtxCanTransmit);

    return rslt;
}


void decipherMsg(ControlStruct* decipherPtr, CANRxFrame* rxFrame)
{
    switch (rxFrame->EID & 0xFF)
    {
    case (SetMode): {
        decipherPtr->runType = rxFrame->data8[0];
        break;
    }
    case (SetPosition): {
        float temp;
        uint8ToFloat(rxFrame->data8, &temp);
        decipherPtr->targetPosition = temp;
        
        break;
    }
    case (SetSpeed):{
        float temp;
        uint8ToFloat(rxFrame->data8, &temp);
        decipherPtr->targetSpeed = temp;
        
        break;
    }
    case (SetTorque):{
        float temp;
        uint8ToFloat(rxFrame->data8, &temp);
        decipherPtr->targetTorque = temp;
        
        break;
    }
    case(SetBreakStrength):{
        float temp;
        uint8ToFloat(rxFrame->data8, &temp);
        decipherPtr->targetBreakStrength = temp;
        
        break;
    }
    default:
        break;
    }
}
