#include "GuitarHeroAdapter.h"

GuitarHeroAdapter::GuitarHeroAdapter(XboxGamepadDevice& device)
    : _device(device)
{}

void GuitarHeroAdapter::setFret(uint8_t index, bool pressed) {
    if (index >= 5) return;
    if (_fretState[index] != pressed) {
        _fretState[index] = pressed;
        pressed ? _device.press(XBOX_BUTTON_MAPPING[index])
                : _device.release(XBOX_BUTTON_MAPPING[index]);
        _dirty = true;
    }
}

void GuitarHeroAdapter::setStrumUp(bool pressed) {
    if (_strumUp != pressed) {
        _strumUp = pressed;
        pressed ? _device.press(XBOX_STRUM_UP)
                : _device.release(XBOX_STRUM_UP);
        _dirty = true;
    }
}

void GuitarHeroAdapter::setStrumDown(bool pressed) {
    if (_strumDown != pressed) {
        _strumDown = pressed;
        pressed ? _device.press(XBOX_STRUM_DOWN)
                : _device.release(XBOX_STRUM_DOWN);
        _dirty = true;
    }
}

void GuitarHeroAdapter::setWhammy(uint16_t value) {
    if (_whammy != value) {
        _whammy = value;
        _device.setRightTrigger(value);
        _dirty = true;
    }
}

bool GuitarHeroAdapter::stateChanged() const { return _dirty; }
void GuitarHeroAdapter::clearDirty() { _dirty = false; }
