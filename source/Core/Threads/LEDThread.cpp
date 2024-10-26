/*
 * LEDThread.cpp
 *
 *  Created on: 26 Oct 2024
 *      Author: BlockWorker
 */
extern "C" {
#include "FreeRTOSConfig.h"
}
#include "OperatingModeUtilities.h"
#include "OperatingModes.h"
#include "Settings.h"
#include "main.hpp"
#include "settingsGUI.hpp"
#include "stdlib.h"
#include "string.h"
// File local variables

// minimum acceptable voltage for enabling LED ring (x 100mV)
#define LEDRingMinVoltageOn 115
// minimum acceptable voltage before disabling LED ring (x 100mV)
#define LEDRingMinVoltageOff 105

extern OperatingMode currentMode;

// whether the voltage is acceptable for LED operation (if too low: physically can't drive LEDs)
static bool acceptableLEDVoltage = false;

/* StartLEDTask function */
void startLEDTask(void const *argument) {
  (void)argument;
  
  // initially disable LED ring
  setLEDRingPWM(0);

  // wait half a second to ensure we're booted up
  osDelay(5 * TICKS_100MS);
  
  for (;;) {
    // check whether input voltage is high enough for driving LEDs
    uint16_t voltage = getInputVoltageX10(getSettingValue(SettingsOptions::VoltageDiv), 0);
    if (acceptableLEDVoltage && voltage < LEDRingMinVoltageOff) {
      acceptableLEDVoltage = false;
    } else if (!acceptableLEDVoltage && voltage > LEDRingMinVoltageOn) {
      acceptableLEDVoltage = true;
    }

    // whether LED ring should be enabled in current state - requires acceptable voltage
    // (soldering/boost, adjusting/inspecting LED ring brightness option, or idle and holding front button)
    bool enable_light =
      acceptableLEDVoltage && (
        currentMode == OperatingMode::soldering ||
        currentMode == OperatingMode::boost ||
        (currentMode == OperatingMode::settings && isLEDRingSetting) ||
        (currentMode == OperatingMode::idle && getButtonA())
      );

    // write LED ring state to PWM
    if (enable_light) {
      setLEDRingPWM(getSettingValue(SettingsOptions::LightRingBrightness));
    } else {
      setLEDRingPWM(0);
    }

    // no need to update this too often
    osDelay(TICKS_100MS);
  }
}