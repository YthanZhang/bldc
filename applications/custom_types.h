//
// Created by yichy_000 on 2021/3/6.
//

#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "stdbool.h"
#include "stdint.h"

struct Button_handle;

/**
 * @brief Custom struct Button provide functionalities that deal with button
 * press
 *
 * @note Use function makeButton to create a button struct;
 */
struct Button
{
    bool pressed;              // private
    bool pressUsed;            // private
    uint32_t pressCounter;     // private
    uint32_t holdThreshold;    // private

    const struct Button_handle* mfunc;    // pointer to member functions
};


void privateButton_updatePressed(struct Button* button, uint32_t button_input)
{
    if (button_input)
    {
        if (button->pressed)
        {
            ++(button->pressCounter);
        }
        else
        {
            button->pressed = true;
        }
    }
    else
    {
        button->pressed      = false;
        button->pressUsed    = false;
        button->pressCounter = 0;
    }
}

bool privateButton_isPressed(struct Button* button) { return button->pressed; }

bool privateButton_isHold(struct Button* button)
{
    return button->pressCounter > button->holdThreshold;
}

uint32_t privateButton_getPressCounter(struct Button* button)
{
    return button->pressCounter;
}

uint32_t privateButton_setHoldThreshold(struct Button* button,
                                        uint32_t newHoldThreshold)
{
    return button->holdThreshold = newHoldThreshold;
}

uint32_t privateButton_getHoldThreshold(struct Button* button)
{
    return button->holdThreshold;
}

bool privateButton_pressUsed(struct Button* button)
{
    return button->pressUsed;
}
void privateButton_usePress(struct Button* button)
{
    button->pressUsed = button->pressed;
}


struct Button_handle
{
    /**
     * @brief update the status of button press.
     *
     * @param button
     * ptr to the Button struct you wish to update.
     *
     * @param button_input
     * Input to determine if the physical button is pressed.
     * 0 for not press, other value for pressed.
     */
    void (*updatePressed)(struct Button* button, uint32_t button_input);
    /**
     * @brief get press status of button.
     * @param button ptr to the Button struct you wish to know the status of.
     * @return true if button is pressed, false if not.
     */
    bool (*const isPressed)(struct Button* button);
    bool (*const isHold)(struct Button* button);
    uint32_t (*const getPressCounter)(struct Button* button);
    uint32_t (*const setHoldThreshold)(struct Button* button,
                                       uint32_t newHoldThreshold);
    uint32_t (*const getHoldThreshold)(struct Button* button);
    bool (*const pressUsed)(struct Button* button);
    void (*const usePress)(struct Button* button);
};

const struct Button_handle buttonHandler = {
    .updatePressed    = privateButton_updatePressed,
    .isPressed        = privateButton_isPressed,
    .isHold           = privateButton_isHold,
    .getPressCounter  = privateButton_getPressCounter,
    .setHoldThreshold = privateButton_setHoldThreshold,
    .getHoldThreshold = privateButton_getHoldThreshold,
    .pressUsed        = privateButton_pressUsed,
    .usePress         = privateButton_usePress,
};


struct Button makeButton(void)
{
    struct Button button = {
        .pressed       = false,
        .pressUsed     = false,
        .pressCounter  = 0,
        .holdThreshold = 0,

        .mfunc = &buttonHandler,
    };

    return button;
}


typedef enum enum_RunType : uint8_t
{
    _blankRun    = 0x00,
    TorqueMode   = 0x01,
    PositionMode = 0x01 << 1,
    SpeedMode    = 0x01 << 2,
    BreakMode    = 0x01 << 3
} RunType;


typedef struct struct_ControlStruct
{
    RunType runType;
    float targetTorque;
    float targetPosition;
    float targetSpeed;
    float targetBreakStrength;

} ControlStruct;


#endif    // BLDC_CUSTOM_TYPES_H
