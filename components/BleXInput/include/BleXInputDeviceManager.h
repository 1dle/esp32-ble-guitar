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
    bool isConnected();

private:
    NimBLEHIDDevice* _hid = nullptr;
    BleConnectionStatus _connStatus;
    XboxGamepadDevice* _device = nullptr;
    const char* _name;
    const char* _manufacturer;
    uint16_t _idleInterval;
};