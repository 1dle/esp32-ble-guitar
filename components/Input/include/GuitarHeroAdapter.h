#pragma once
#include "XboxGamepadDevice.h"
#include "ConfigManager.h"

class GuitarHeroAdapter {
public:
    explicit GuitarHeroAdapter(XboxGamepadDevice& device);

    void setFret(uint8_t index, bool pressed);
    void setStrumUp(bool pressed);
    void setStrumDown(bool pressed);
    void setWhammy(uint16_t value);

    bool stateChanged() const;
    void clearDirty();

private:
    XboxGamepadDevice& _device;
    bool _dirty = false;

    // Keep state to avoid double-press
    bool _fretState[5]{};
    bool _strumUp = false;
    bool _strumDown = false;
    uint16_t _whammy = 0;

    // Hardcoded GH → Xbox button mapping
    static constexpr uint16_t XBOX_BUTTON_MAPPING[5] = {
        XBOX_BUTTON_A,
        XBOX_BUTTON_B,
        XBOX_BUTTON_Y,
        XBOX_BUTTON_X,
        XBOX_BUTTON_RS
    };

    static constexpr uint16_t XBOX_STRUM_UP = XBOX_BUTTON_DPAD_NORTH;
    static constexpr uint16_t XBOX_STRUM_DOWN = XBOX_BUTTON_DPAD_SOUTH;
};
