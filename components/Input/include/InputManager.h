#pragma once
#include "GuitarHeroAdapter.h"
#include "ConfigManager.h"
#include "driver/gpio.h"
#include <cstdint>
#include "esp_adc/adc_oneshot.h"

class InputManager {
public:
    InputManager(GuitarHeroAdapter& adapter, ConfigManager& config);

    void init();
    void update();  // call at configured polling rate

private:
    GuitarHeroAdapter& _adapter;
    ConfigManager& _config;

private:

    uint32_t _stableState = 0;
    uint32_t _lastRawState = 0;
    uint8_t _sampleCount[7] = {0};  // Tracks consecutive stable samples per button
    uint16_t _filteredWhammy = 2048;

    adc_oneshot_unit_handle_t _adcHandle;
    uint32_t readRawDigital();
    void processDigitalInputs(uint32_t rawState);
    void processAnalogInput();
};
