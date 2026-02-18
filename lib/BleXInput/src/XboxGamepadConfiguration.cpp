#include "XboxGamepadConfiguration.h"
#include "XboxGamepadDevice.h"
#include "NimBLEHIDDevice.h"


XboxSeriesXControllerDeviceConfiguration::XboxSeriesXControllerDeviceConfiguration(uint8_t reportId) {
    _reportId = reportId;
    _autoDefer = false;
}

BleHostConfiguration XboxSeriesXControllerDeviceConfiguration::getIdealHostConfiguration() const {
    BleHostConfiguration config;
    config.setHidType(HID_GAMEPAD);
    config.setVidSource(VENDOR_USB_SOURCE);
    config.setVid(XBOX_VENDOR_ID);
    config.setPid(XBOX_1914_PRODUCT_ID);
    config.setGuidVersion(XBOX_1914_BCD_DEVICE_ID);
    config.setSerialNumber(XBOX_1914_SERIAL);
    return config;
}

uint8_t XboxSeriesXControllerDeviceConfiguration::getDeviceReportSize() const {
    return sizeof(XboxGamepadInputReportData);
}

size_t XboxSeriesXControllerDeviceConfiguration::makeDeviceReport(uint8_t* buffer, size_t bufferSize) const {
    size_t hidDescriptorSize = sizeof(XboxOneS_1914_HIDDescriptor);
    if (hidDescriptorSize < bufferSize) {
        memcpy(buffer, XboxOneS_1914_HIDDescriptor, hidDescriptorSize);
        return hidDescriptorSize;
    }
    return 0;
}