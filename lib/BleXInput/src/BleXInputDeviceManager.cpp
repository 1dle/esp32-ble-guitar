#include "BleXInputDeviceManager.h"

static const char *LOG_TAG = "BleXInputDeviceManager";

#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_timer.h"

BleXInputDeviceManager::BleXInputDeviceManager(const char* name, const char* manufacturer, uint16_t idleInterval)
    : _name(name), _manufacturer(manufacturer), _idleInterval(idleInterval)
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
    NimBLEDevice::setSecurityAuth(true, true, true);  // bonding + MITM + SC
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);


    auto server = NimBLEDevice::createServer();
    server->setCallbacks(&_connStatus);

    _hid = new NimBLEHIDDevice(server);
    _hid->setManufacturer(std::string(_manufacturer));
    _hid->setPnp(0x02, hostConfig.getVid(), hostConfig.getPid(), hostConfig.getGuidVersion());
    _hid->setHidInfo(0x00, 0x01);

    if (_device) {
        _device->init(_hid);
        auto cfg = _device->getDeviceConfig();
        if (cfg) {
            uint8_t reportMap[1024];
            size_t mapSize = cfg->makeDeviceReport(reportMap, sizeof(reportMap));
            if (mapSize > 0) {
                _hid->setReportMap(reportMap, (uint16_t)mapSize);
            }
        }
    }

    _hid->startServices();
    _hid->setBatteryLevel(100, false);

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setName(_name);
    adv->setAppearance(HID_GAMEPAD);
    adv->addServiceUUID(_hid->getHidService()->getUUID());
    adv->addServiceUUID(_hid->getBatteryService()->getUUID());

    ESP_LOGI(LOG_TAG, "BLE HID started: %s", _name);

    // Single-slot queue coalesces report requests and avoids burst backlog.
    _reportQueue = xQueueCreate(1, sizeof(uint8_t));
    auto bleTask = [](void* ctx) {
        BleXInputDeviceManager* mgr = (BleXInputDeviceManager*)ctx;
        uint8_t token;
        while (true) {
            // wait for a report request or timeout to run periodic update
            if (mgr->_reportQueue && xQueueReceive(mgr->_reportQueue, &token, pdMS_TO_TICKS(20)) == pdTRUE) {
                if (mgr->_device) {
                    mgr->_device->sendGamepadReport();
                }
            }
            // Always update connection state and advertising
            mgr->update();
        }
    };

    xTaskCreatePinnedToCore(bleTask, "ble_task", 4096, this, 2, &_bleTaskHandle, 0);
}

bool BleXInputDeviceManager::isConnected() const
{
    return _connStatus.isConnected();
}

void BleXInputDeviceManager::update()
{
    _connStatus.update();
}

void BleXInputDeviceManager::enqueueReport() {
    if (!_reportQueue) return;
    uint8_t token = 1;
    xQueueOverwrite(_reportQueue, &token);
}