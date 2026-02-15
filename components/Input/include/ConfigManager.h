#pragma once
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include <cstdint>
#include <array>

// Logical Guitar Hero inputs
enum class GHInput : uint8_t {
    FRET_0,
    FRET_1,
    FRET_2,
    FRET_3,
    FRET_4,
    STRUM_UP,
    STRUM_DOWN,
    WHAMMY
};

// ============ DIGITAL INPUTS ============
// GPIO → Logical GH input mapping for digital (button) inputs
struct DigitalInputMap {
    gpio_num_t gpio;
    GHInput input;
};

// ============ ANALOG INPUTS ============
// ADC configuration for analog (continuous) inputs like whammy
struct AnalogInputConfig {
    adc_unit_t unit = ADC_UNIT_1;
    adc_channel_t channel = ADC_CHANNEL_6;
    gpio_num_t gpio = GPIO_NUM_34;
    GHInput input = GHInput::WHAMMY;
};

// ============ CONFIG MANAGER ============
class ConfigManager {
public:
    ConfigManager();

    // Digital input configuration (7 buttons)
    const std::array<DigitalInputMap, 7>& getDigitalInputMapping() const;
    void setDigitalInputPin(uint8_t index, gpio_num_t gpio);

    // Analog input configuration (1 whammy lever)
    const AnalogInputConfig& getAnalogInputConfig() const;

    // Debouncing parameters (for digital inputs only)
    uint8_t getDebounceConfirmSamples() const { return _debounceConfirmSamples; }
    
    // Polling rate (milliseconds)
    uint32_t getInputTaskDelayMs() const { return _inputTaskDelayMs; }

private:
    std::array<DigitalInputMap, 7> _digitalInputs;
    AnalogInputConfig _analogInput;
    
    // Debouncing: requires 2 consecutive stable samples for acceptance
    uint8_t _debounceConfirmSamples = 2;
    
    // Input task polling rate in milliseconds
    uint32_t _inputTaskDelayMs = 10;  // 100Hz
};



