#include "BLEHostConfiguration.h"
#include "NimBLEHIDDevice.h"

BleHostConfiguration::BleHostConfiguration() :
    _vidSource(0x01),
    _vid(0xe502),
    _pid(0xbbab),
    _guidVersion(0x0110),
    _hid_type(GENERIC_HID),
    _modelNumber("1.0.0"),
    _softwareRevision("1.0.0"),
    _serialNumber("0123456789"),
    _firmwareRevision("1.0.0"),
    _hardwareRevision("1.0.0"),
    _systemID(""),
    _deferSendRate(240),
    _threadedAutoSend(false)
{               
}

uint16_t BleHostConfiguration::getVidSource(){ return _vidSource; }
uint16_t BleHostConfiguration::getVid(){ return _vid; }
uint16_t BleHostConfiguration::getPid(){ return _pid; }
uint16_t BleHostConfiguration::getGuidVersion(){ return _guidVersion; }
uint16_t BleHostConfiguration::getHidType(){ return _hid_type; }

const char* BleHostConfiguration::getModelNumber(){ return _modelNumber.c_str(); }
const char* BleHostConfiguration::getSoftwareRevision(){ return _softwareRevision.c_str(); }
const char* BleHostConfiguration::getSerialNumber(){ return _serialNumber.c_str(); }
const char* BleHostConfiguration::getFirmwareRevision(){ return _firmwareRevision.c_str(); }
const char* BleHostConfiguration::getHardwareRevision(){ return _hardwareRevision.c_str(); }
const char* BleHostConfiguration::getSystemID(){ return _systemID.c_str(); }

void BleHostConfiguration::setVidSource(uint8_t value) { _vidSource = value; }
void BleHostConfiguration::setVid(uint16_t value) { _vid = value; }
void BleHostConfiguration::setPid(uint16_t value) { _pid = value; }
void BleHostConfiguration::setGuidVersion(uint16_t value) { _guidVersion = value; }
void BleHostConfiguration::setHidType(uint16_t value) { _hid_type = value; }

void BleHostConfiguration::setModelNumber(const char *value) { _modelNumber = std::string(value); }
void BleHostConfiguration::setSoftwareRevision(const char *value) { _softwareRevision = std::string(value); }
void BleHostConfiguration::setSerialNumber(const char *value) { _serialNumber = std::string(value); }
void BleHostConfiguration::setFirmwareRevision(const char *value) { _firmwareRevision = std::string(value); }
void BleHostConfiguration::setHardwareRevision(const char *value) { _hardwareRevision = std::string(value); }

void BleHostConfiguration::setQueueSendRate(uint32_t value) { _deferSendRate = value; }
uint32_t BleHostConfiguration::getQueueSendRate() const { return _deferSendRate; }

void BleHostConfiguration::setQueuedSending(bool value) { _threadedAutoSend = value; }
bool BleHostConfiguration::getQueuedSending() const { return _threadedAutoSend; }