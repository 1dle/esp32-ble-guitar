#include "ConfigManager.h"

ConfigManager::ConfigManager() {
    // Configure 7 digital button inputs
    _digitalInputs = {{
        {GPIO_NUM_4, GHInput::FRET_0},
        {GPIO_NUM_5, GHInput::FRET_1},
        {GPIO_NUM_18, GHInput::FRET_2},
        {GPIO_NUM_19, GHInput::FRET_3},
        {GPIO_NUM_21, GHInput::FRET_4},
        {GPIO_NUM_22, GHInput::STRUM_UP},
        {GPIO_NUM_23, GHInput::STRUM_DOWN}
    }};
    
    // Configure analog whammy input (already initialized with defaults in header)
    // Can customize here if needed:
    // _analogInput.gpio = GPIO_NUM_34;
    // _analogInput.channel = ADC_CHANNEL_6;
    // _analogInput.unit = ADC_UNIT_1;
}

const std::array<DigitalInputMap, 7>& ConfigManager::getDigitalInputMapping() const {
    return _digitalInputs;
}

void ConfigManager::setDigitalInputPin(uint8_t index, gpio_num_t gpio) {
    if (index < _digitalInputs.size()) {
        _digitalInputs[index].gpio = gpio;
    }
}

const AnalogInputConfig& ConfigManager::getAnalogInputConfig() const {
    return _analogInput;
}
