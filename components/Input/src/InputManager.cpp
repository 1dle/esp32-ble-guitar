#include "InputManager.h"
#include "esp_timer.h"
#include "esp_log.h"



static const char* LOG_TAG = "InputManager";

InputManager::InputManager(GuitarHeroAdapter& adapter, ConfigManager& config)
    : _adapter(adapter), _config(config)
{}

void InputManager::init() {
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // Build combined pin mask for all digital GPIO pins
    uint64_t pin_mask = 0;
    auto digitalMapping = _config.getDigitalInputMapping();
    for (auto& inputMap : digitalMapping) {
        pin_mask |= (1ULL << inputMap.gpio);
    }
    
    io_conf.pin_bit_mask = pin_mask;
    gpio_config(&io_conf);

    // Initialize ADC for analog whammy input
    auto analogConfig = _config.getAnalogInputConfig();
    adc_oneshot_unit_init_cfg_t init_config = {};
    init_config.unit_id = analogConfig.unit;
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &_adcHandle));

    adc_oneshot_chan_cfg_t channel_config = {};
    channel_config.atten = ADC_ATTEN_DB_12;      // 0–3.3V
    channel_config.bitwidth = ADC_BITWIDTH_DEFAULT;

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            _adcHandle,
            analogConfig.channel,
            &channel_config
        )
    );

    // Initialize EMA with a mid-point
    _filteredWhammy = 2048; // ~half of 12-bit max
    
    ESP_LOGI(LOG_TAG, "InputManager initialized - Digital: 7 buttons, Analog: whammy (debounce samples: %d)", 
             _config.getDebounceConfirmSamples());
}

uint32_t InputManager::readRawDigital() {
    uint32_t state = 0;
    auto digitalMapping = _config.getDigitalInputMapping();
    for (size_t i = 0; i < digitalMapping.size(); ++i) {
        if (!gpio_get_level(digitalMapping[i].gpio))
            state |= (1 << i);
    }
    return state;
}

void InputManager::processDigitalInputs(uint32_t rawState) {
    auto digitalMapping = _config.getDigitalInputMapping();
    uint8_t requiredSamples = _config.getDebounceConfirmSamples();

    // Process all 7 digital button inputs
    for (size_t i = 0; i < digitalMapping.size(); ++i) {
        bool rawPressed = (rawState & (1 << i)) != 0;
        bool stablePressed = (_stableState & (1 << i)) != 0;
        bool lastRawPressed = (_lastRawState & (1 << i)) != 0;

        // Majority voting debouncing:
        // - If raw state differs from last, reset counter (new noise)
        // - If raw state matches last but differs from stable, increment counter
        // - Once counter reaches threshold, accept and commit the change

        if (rawPressed != lastRawPressed) {
            // Raw state changed - reset counter (potential noise burst)
            _sampleCount[i] = 0;
        } else if (rawPressed != stablePressed) {
            // Raw state same as last, but differs from stable - count it
            _sampleCount[i]++;
            
            // Debounce threshold met - accept the change
            if (_sampleCount[i] >= requiredSamples) {
                if (rawPressed) {
                    _stableState |= (1 << i);
                } else {
                    _stableState &= ~(1 << i);
                }
                _sampleCount[i] = 0;  // Reset counter after acceptance

                // Forward logical event to adapter
                switch (digitalMapping[i].input) {
                    case GHInput::FRET_0: _adapter.setFret(0, rawPressed); break;
                    case GHInput::FRET_1: _adapter.setFret(1, rawPressed); break;
                    case GHInput::FRET_2: _adapter.setFret(2, rawPressed); break;
                    case GHInput::FRET_3: _adapter.setFret(3, rawPressed); break;
                    case GHInput::FRET_4: _adapter.setFret(4, rawPressed); break;
                    case GHInput::STRUM_UP: _adapter.setStrumUp(rawPressed); break;
                    case GHInput::STRUM_DOWN: _adapter.setStrumDown(rawPressed); break;
                    case GHInput::WHAMMY: break; // Should not reach here (analog input)
                }
            }
        } else {
            // State is stable - reset counter
            _sampleCount[i] = 0;
        }
    }

    _lastRawState = rawState;
}

void InputManager::processAnalogInput() {
    auto analogConfig = _config.getAnalogInputConfig();
    
    int retries = 3; // retry up to 3 times if ADC busy
    int adc_value = 0;
    while (retries--) {
        esp_err_t ret = adc_oneshot_read(_adcHandle, analogConfig.channel, &adc_value);
        if (ret == ESP_OK) break;          // success
    }
    
    // Apply exponential moving average filter
    _filteredWhammy = (_filteredWhammy * 3 + adc_value) / 4;
    uint16_t mapped = (_filteredWhammy * 1023) / 4095;
    _adapter.setWhammy(mapped);
}


void InputManager::update() {
    static int analogDivider = 0;
    uint32_t raw = readRawDigital();
    processDigitalInputs(raw);
    if (++analogDivider >= 10) { // run analog input ~100Hz at 100Hz task rate
        processAnalogInput();
        analogDivider = 0;
    }
}
