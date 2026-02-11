#include "BleXInputDeviceManager.h"

static const char *LOG_TAG = "BleXInputDeviceManager";

BleXInputDeviceManager::BleXInputDeviceManager(const char* name, const char* manufacturer, uint16_t idleInterval) :
    _name(name), _manufacturer(manufacturer), _idleInterval(idleInterval)
{
}

BleXInputDeviceManager::~BleXInputDeviceManager()
{
    if (_hid) {
        delete _hid;
        _hid = nullptr;
    }
}

void BleXInputDeviceManager::setDevice(XboxGamepadDevice* device)
{
    _device = device;
}

void BleXInputDeviceManager::begin(BleHostConfiguration hostConfig)
{
    NimBLEDevice::init(_name);
    auto server = NimBLEDevice::createServer();
    server->setCallbacks(&_connStatus);

    _hid = new NimBLEHIDDevice(server);
    _hid->setManufacturer(std::string(_manufacturer));

    // Basic HID info - CRITICAL: Use correct parameters
    _hid->setPnp(0x02, hostConfig.getVid(), hostConfig.getPid(), hostConfig.getGuidVersion());
    _hid->setHidInfo(0x00, 0x01);  // Correct parameters for HID info

    if (_device) {
        _device->init(_hid);
    }

    // Set report map from device configuration if available
    auto cfg = _device ? _device->getDeviceConfig() : nullptr;
    if (cfg) {
        // Allocate a buffer for the HID descriptor
        const size_t bufSize = 1024;
        uint8_t* reportMap = (uint8_t*)malloc(bufSize);
        if (reportMap) {
            size_t mapSize = cfg->makeDeviceReport(reportMap, bufSize);
            if (mapSize > 0) {
                _hid->setReportMap(reportMap, (uint16_t)mapSize);
            }
            free(reportMap);
        }
    }

    NimBLEDevice::setSecurityAuth(true, false, false);  // enable bonding, no MITM, no SC

    _hid->startServices();
    // Set initial battery level AFTER services start
    _hid->setBatteryLevel(100, false);

    // Configure and start BLE advertisement with proper settings
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    
    // Set device name in advertising - needed for proper device identification
    pAdvertising->setName(_name);
    
    pAdvertising->setAppearance(HID_GAMEPAD);
    pAdvertising->addServiceUUID(_hid->getHidService()->getUUID());
    pAdvertising->addServiceUUID(_hid->getBatteryService()->getUUID());
    
    pAdvertising->start();

    ESP_LOGI(LOG_TAG, "BLE HID started");
    ESP_LOGI(LOG_TAG, "Device: %s (HID Gamepad with security enabled)", _name);
}

bool BleXInputDeviceManager::isConnected()
{
    return _connStatus.isConnected();
}
