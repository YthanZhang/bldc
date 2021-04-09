/*
    Copyright 2017 Benjamin Vedder	benjamin@vedder.se

    This file is part of the VESC firmware.

    The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

//#include "conf_general.h"

#include "custom_types.h"
#include "mc_interface.h"
#include "timeout.h"

#ifdef APP_CUSTOM_TO_USE
#include APP_CUSTOM_TO_USE
#endif

#include "ch.h"
#include "hal.h"
#include "hw.h"
#include "mc_interface.h"
#include "timeout.h"


/**GPIO port for position reset sensor*/
#define CT_POS_RESET_PORT HW_ICU_GPIO
/**GPIO pin for position reset sensor*/
#define CT_POS_RESET_PIN HW_ICU_PIN

/**GPIO port for weight increase button*/
#define CT_BUTTON_UP_PORT HW_UART_RX_PORT
/**GPIO pin for weight increase button*/
#define CT_BUTTON_UP_PIN HW_UART_RX_PIN

/**GPIO port for start stop button*/
#define CT_BUTTON_START_STOP_PORT HW_UART_TX_PORT
/**GPIO pin for start stop button*/
#define CT_BUTTON_START_STOP_PIN HW_UART_TX_PIN

/**Time in sys tick how often the tread should be invoked*/
const systime_t threadPeriod = MS2ST(20);

const float distanceMax  = 20;    // max allowed distance for motor to travel
const float breakCurrent = 2;

bool customWeightLiftStopThd    = false;
bool customWeightLiftThdRunning = false;

/**
 * @brief Custom thread for weight lifting.
 * @param args empty
 */
static THD_FUNCTION(customWeightLiftThread, args);
/**size of working area(wa) of customWeightLiftThread*/
static THD_WORKING_AREA(customWeightLiftThread_wa, 2048);


void customWeightLiftInit(void)
{
    // set pos reset button pin to gpio input pull down mode
    palSetPadMode(CT_POS_RESET_PORT, CT_POS_RESET_PIN, PAL_MODE_INPUT_PULLDOWN);

    // set button up pin to gpio input pull down mode
    palSetPadMode(CT_BUTTON_UP_PORT, CT_BUTTON_UP_PIN, PAL_MODE_INPUT_PULLDOWN);

    // set pin for start stop button to input pull down mode
    palSetPadMode(CT_BUTTON_START_STOP_PORT,
                  CT_BUTTON_START_STOP_PIN,
                  PAL_MODE_INPUT_PULLDOWN);

    customWeightLiftStopThd = false;

    chThdCreateStatic(customWeightLiftThread_wa,
                      sizeof(customWeightLiftThread_wa),
                      NORMALPRIO,
                      customWeightLiftThread,
                      NULL);
}


void customWeightLiftStop(void)
{
    customWeightLiftStopThd = true;
    while (customWeightLiftThdRunning) { chThdSleepMilliseconds(10); }
}


static THD_FUNCTION(customWeightLiftThread, args)
{
    (void)args;    // no argument

    chRegSetThreadName("Weight_Lift_Thd");
    customWeightLiftThdRunning = true;


    mc_interface_set_brake_current(breakCurrent);

    RunType runType           = BreakMode;
    struct Button buttonReset = makeButton();
    struct Button buttonUp    = makeButton();
    struct Button buttonRun   = makeButton();
    bool running              = false;
    float targetTorque        = 0;
    float distanceReference   = 0;
    float distanceTracking    = 0;

    while (!customWeightLiftStopThd)
    {
        systime_t tick = chVTGetSystemTime();


        buttonReset.mfunc->updatePressed(&buttonReset,
                                         palReadPad(CT_POS_RESET_PORT,
                                                    CT_POS_RESET_PIN));
        buttonUp.mfunc->updatePressed(&buttonUp,
                                      palReadPad(CT_BUTTON_UP_PORT,
                                                 CT_BUTTON_UP_PIN));
        buttonRun.mfunc->updatePressed(&buttonRun,
                                       palReadPad(CT_BUTTON_START_STOP_PORT,
                                                  CT_BUTTON_START_STOP_PIN));

        // update reference distance to calculate the relative distance between
        // push rod and reset point
        if (buttonReset.mfunc->isHold(&buttonReset) &&
            !buttonReset.mfunc->pressUsed(&buttonReset))
        {
            buttonReset.mfunc->usePress(&buttonReset);
            distanceReference = mc_interface_get_distance();
        }
        // calculate the relative distance between current and reference
        distanceTracking = mc_interface_get_distance() - distanceReference;

        // increase target torque per button press
        if (buttonUp.mfunc->isHold(&buttonUp) &&
            !buttonUp.mfunc->pressUsed(&buttonUp))
        {
            buttonUp.mfunc->usePress(&buttonUp);
            targetTorque += 0.1f;
        }

        // toggle running every button press
        if (buttonRun.mfunc->isHold(&buttonRun) &&
            !buttonRun.mfunc->pressUsed(&buttonRun))
        {
            buttonRun.mfunc->usePress(&buttonRun);
            running = !running;
        }

        // reset targetTorque when running is false
        if (!running)
        {
            if (runType != BreakMode)
            {
                targetTorque = 0;
                mc_interface_set_brake_current(breakCurrent);
                runType = BreakMode;
            }
        }
        else if (distanceTracking < distanceMax)
        {
            if (runType != TorqueMode)
            {
                mc_interface_set_current(targetTorque);
                runType = TorqueMode;
            }
        }
        else if (runType != PositionMode)
        {
            mc_interface_set_pid_pos(mc_interface_get_pid_pos_now());
            runType = PositionMode;
        }


        systime_t delay = threadPeriod - chVTTimeElapsedSinceX(tick);
        chThdSleep(delay);
        // Use tick gotten from the beginning of the loop to determine how long
        // should the thread sleep

        timeout_reset();    // reset timeout so vesc runs properly
    }

    customWeightLiftThdRunning = false;
}
