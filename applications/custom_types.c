//
// Created by ythan on 2021/4/17.
//


#include "custom_types.h"


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


/**Private methods for Button**************************************************/
static void privateButton_updatePressed(struct Button* button,
                                        uint32_t button_input)
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

static bool privateButton_isPressed(struct Button* button)
{
    return button->pressed;
}

static bool privateButton_isHold(struct Button* button)
{
    return button->pressCounter > button->holdThreshold;
}

static uint32_t privateButton_getPressCounter(struct Button* button)
{
    return button->pressCounter;
}

static uint32_t privateButton_setHoldThreshold(struct Button* button,
                                               uint32_t newHoldThreshold)
{
    return button->holdThreshold = newHoldThreshold;
}

static uint32_t privateButton_getHoldThreshold(struct Button* button)
{
    return button->holdThreshold;
}

static bool privateButton_pressUsed(struct Button* button)
{
    return button->pressUsed;
}
static void privateButton_usePress(struct Button* button)
{
    button->pressUsed = button->pressed;
}


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
