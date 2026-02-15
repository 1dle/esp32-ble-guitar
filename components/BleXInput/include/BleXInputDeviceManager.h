#pragma once

#include "BLEHostConfiguration.h"
#include "BleConnectionStatus.h"
#include "XboxGamepadDevice.h"

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEAdvertising.h>

class BleXInputDeviceManager {
public:
    BleXInputDeviceManager(const char* name, const char* manufacturer, uint16_t idleInterval = 100);
    ~BleXInputDeviceManager();

    void setDevice(XboxGamepadDevice* device);
    void begin(BleHostConfiguration hostConfig);
    bool isConnected() const;
    void update();
    BleConnectionStatus _connStatus;

private:
    const char* _name;
    const char* _manufacturer;
    NimBLEHIDDevice* _hid = nullptr;
    XboxGamepadDevice* _device = nullptr;
    uint16_t _idleInterval;
};
