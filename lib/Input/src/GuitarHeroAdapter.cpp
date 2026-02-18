#include "GuitarHeroAdapter.h"
#include "esp_timer.h"

GuitarHeroAdapter::GuitarHeroAdapter(XboxGamepadDevice& device)
    : _device(device)
{}

void GuitarHeroAdapter::setFret(uint8_t index, bool pressed) {
    if (index >= 5) return;
    if (_fretState[index] != pressed) {
        _fretState[index] = pressed;
        uint16_t xboxButton = XBOX_BUTTON_A;
        switch (index) {
            case 0: xboxButton = XBOX_BUTTON_A; break;
            case 1: xboxButton = XBOX_BUTTON_B; break;
            case 2: xboxButton = XBOX_BUTTON_Y; break;
            case 3: xboxButton = XBOX_BUTTON_X; break;
            case 4: xboxButton = XBOX_BUTTON_RS; break;
            default: return;
        }

        pressed ? _device.press(xboxButton) : _device.release(xboxButton);
        _dirty = true;
        _device.markLastInputTimestamp(esp_timer_get_time());
    }
}

void GuitarHeroAdapter::setStrumUp(bool pressed) {
    if (_strumUp != pressed) {
        _strumUp = pressed;
    pressed ? _device.press(XBOX_STRUM_UP)
        : _device.release(XBOX_STRUM_UP);
    _dirty = true;
    _device.markLastInputTimestamp(esp_timer_get_time());
    }
}

void GuitarHeroAdapter::setStrumDown(bool pressed) {
    if (_strumDown != pressed) {
        _strumDown = pressed;
    pressed ? _device.press(XBOX_STRUM_DOWN)
        : _device.release(XBOX_STRUM_DOWN);
    _dirty = true;
    _device.markLastInputTimestamp(esp_timer_get_time());
    }
}

void GuitarHeroAdapter::setWhammy(uint16_t value) {
    if (_whammy != value) {
        _whammy = value;
        _device.setRightTrigger(value);
        _dirty = true;
        _device.markLastInputTimestamp(esp_timer_get_time());
    }
}

void GuitarHeroAdapter::setInput(GHInput input, bool pressed) {
    switch (input) {
        case GHInput::FRET_0: setFret(0, pressed); break;
        case GHInput::FRET_1: setFret(1, pressed); break;
        case GHInput::FRET_2: setFret(2, pressed); break;
        case GHInput::FRET_3: setFret(3, pressed); break;
        case GHInput::FRET_4: setFret(4, pressed); break;
        case GHInput::STRUM_UP: setStrumUp(pressed); break;
        case GHInput::STRUM_DOWN: setStrumDown(pressed); break;
        case GHInput::STAR: /* TODO */ break;
        case GHInput::SELECT: /* TODO */ break; 
        default: break;
    }
}

bool GuitarHeroAdapter::stateChanged() const { return _dirty; }
void GuitarHeroAdapter::clearDirty() { _dirty = false; }
