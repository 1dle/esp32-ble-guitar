#include "XboxGamepadDevice.h"
#include "esp_log.h"
static const char *LOG_TAG = "XboxGamepadDevice";

template<typename T, typename U, typename V>
static inline T constrain(T v, U lo, V hi) {
    return (v < (T)lo) ? (T)lo : (v > (T)hi) ? (T)hi : v;
}



XboxGamepadCallbacks::XboxGamepadCallbacks(XboxGamepadDevice* device) : _device(device)
{
}

void XboxGamepadCallbacks::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
{    
    // An example packet we might receive from XInput might look like 0x0300002500ff00ff
    XboxGamepadOutputReportData vibrationData = pCharacteristic->getValue<uint64_t>();
    
    ESP_LOGD(LOG_TAG, "XboxGamepadCallbacks::onWrite, Size: %d, DC enable: %d, magnitudeWeak: %d, magnitudeStrong: %d, duration: %d, start delay: %d, loop count: %d", 
        pCharacteristic->getValue().size(),
        vibrationData.dcEnableActuators, 
        vibrationData.weakMotorMagnitude, 
        vibrationData.strongMotorMagnitude, 
        vibrationData.duration, 
        vibrationData.startDelay, 
        vibrationData.loopCount
    );

    _device->onVibrate.fire(vibrationData);
}

void XboxGamepadCallbacks::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
{
    ESP_LOGD(LOG_TAG, "XboxGamepadCallbacks::onRead");
}

void XboxGamepadCallbacks::onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue)
{
    ESP_LOGD(LOG_TAG, "XboxGamepadCallbacks::onSubscribe");
}

void XboxGamepadCallbacks::onStatus(NimBLECharacteristic* pCharacteristic, int code)
{
    ESP_LOGD(LOG_TAG, "XboxGamepadCallbacks::onStatus, code: %d", code);
}

XboxGamepadDevice::XboxGamepadDevice() :
    _extra_input(nullptr),
    _callbacks(nullptr),
    _config(new XboxSeriesXControllerDeviceConfiguration())
{
}

// XboxGamepadDevice methods
XboxGamepadDevice::XboxGamepadDevice(XboxSeriesXControllerDeviceConfiguration* config) :
    _extra_input(nullptr),
    _callbacks(nullptr),
    _config(config)
{
}

XboxGamepadDevice::~XboxGamepadDevice() {
    if (_output && _callbacks) {
        _output->setCallbacks(nullptr);
        delete _callbacks;
        _callbacks = nullptr;
    }

    if (_extra_input) {
        delete _extra_input;
        _extra_input = nullptr;
    }

    if (_config) {
        delete _config;
        _config = nullptr;
    }
}

void XboxGamepadDevice::init(NimBLEHIDDevice* hid) {
    /// Create input characteristic to send events to the computer
    auto input = hid->getInputReport(XBOX_INPUT_REPORT_ID);
    //_extra_input = hid->getInputReport(XBOX_EXTRA_INPUT_REPORT_ID);

    // Create output characteristic to handle events coming from the computer
    auto output = hid->getOutputReport(XBOX_OUTPUT_REPORT_ID);
    _callbacks = new XboxGamepadCallbacks(this);
    output->setCallbacks(_callbacks);

    // store local characteristic pointers
    _input = input;
    _output = output;
}

const XboxSeriesXControllerDeviceConfiguration* XboxGamepadDevice::getDeviceConfig() const {
    // Return the device configuration
    return _config;
}

void XboxGamepadDevice::resetInputs() {
    std::lock_guard<std::mutex> lock(_mutex);
    memset(&_inputReport, 0, sizeof(XboxGamepadInputReportData));

    _inputReport.x = XBOX_AXIS_CENTER_OFFSET;
    _inputReport.y = XBOX_AXIS_CENTER_OFFSET;
    _inputReport.z = XBOX_AXIS_CENTER_OFFSET;
    _inputReport.rz = XBOX_AXIS_CENTER_OFFSET;
}

void XboxGamepadDevice::press(uint16_t button) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Avoid double presses
    if ((_inputReport.buttons & button) != button) {
        _inputReport.buttons |= button;
        ESP_LOGD(LOG_TAG, "XboxGamepadDevice::press, button: %d", button);
    }
}

void XboxGamepadDevice::release(uint16_t button) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Avoid double presses
    if ((_inputReport.buttons & button) == button) {
        _inputReport.buttons ^= button;
        ESP_LOGD(LOG_TAG, "XboxGamepadDevice::release, button: %d", button);
    }
}

bool XboxGamepadDevice::isPressed(uint16_t button) {
    std::lock_guard<std::mutex> lock(_mutex);
    return (bool)((_inputReport.buttons & button) == button);
}

void XboxGamepadDevice::setLeftThumb(int16_t x, int16_t y) {
    x = constrain(x, XBOX_STICK_MIN, XBOX_STICK_MAX);
    y = constrain(y, XBOX_STICK_MIN, XBOX_STICK_MAX);

    std::lock_guard<std::mutex> lock(_mutex);
    _inputReport.x = (uint16_t)(x + XBOX_AXIS_CENTER_OFFSET);
    _inputReport.y = (uint16_t)(y + XBOX_AXIS_CENTER_OFFSET);
}

void XboxGamepadDevice::setRightThumb(int16_t z, int16_t rZ) {
    z = constrain(z, XBOX_STICK_MIN, XBOX_STICK_MAX);
    rZ = constrain(rZ, XBOX_STICK_MIN, XBOX_STICK_MAX);

    std::lock_guard<std::mutex> lock(_mutex);
    _inputReport.z = (uint16_t)(z + XBOX_AXIS_CENTER_OFFSET);
    _inputReport.rz = (uint16_t)(rZ + XBOX_AXIS_CENTER_OFFSET);
}

void XboxGamepadDevice::setLeftTrigger(uint16_t value) {
    value = constrain(value, XBOX_TRIGGER_MIN, XBOX_TRIGGER_MAX);

    std::lock_guard<std::mutex> lock(_mutex);
    _inputReport.brake = value;
}

void XboxGamepadDevice::setRightTrigger(uint16_t value) {
    value = constrain(value, XBOX_TRIGGER_MIN, XBOX_TRIGGER_MAX);

    std::lock_guard<std::mutex> lock(_mutex);
    _inputReport.accelerator = value;
}

void XboxGamepadDevice::setTriggers(uint16_t left, uint16_t right) {
    left = constrain(left, XBOX_TRIGGER_MIN, XBOX_TRIGGER_MAX);
    right = constrain(right, XBOX_TRIGGER_MIN, XBOX_TRIGGER_MAX);

    std::lock_guard<std::mutex> lock(_mutex);
    _inputReport.brake = left;
    _inputReport.accelerator = right;
}

void XboxGamepadDevice::pressDPadDirection(uint8_t direction) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Avoid double presses
    if (_inputReport.hat != direction) {
        ESP_LOGD(LOG_TAG, "Pressing dpad direction %s", dPadDirectionName(direction));
        _inputReport.hat = direction;
    }
}

void XboxGamepadDevice::pressDPadDirectionFlag(XboxDpadFlags direction) {
    // Filter opposite button presses
    if((direction & (XboxDpadFlags::NORTH | XboxDpadFlags::SOUTH)) == (XboxDpadFlags::NORTH | XboxDpadFlags::SOUTH)){
        ESP_LOGD(LOG_TAG, "Filtering opposite button presses - up down");
        direction = (XboxDpadFlags)(direction ^ (uint8_t)(XboxDpadFlags::NORTH | XboxDpadFlags::SOUTH));
    }
    if((direction & (XboxDpadFlags::EAST | XboxDpadFlags::WEST)) == (XboxDpadFlags::EAST | XboxDpadFlags::WEST)){
        ESP_LOGD(LOG_TAG, "Filtering opposite button presses - left right");
        direction = (XboxDpadFlags)(direction ^ (uint8_t)(XboxDpadFlags::EAST | XboxDpadFlags::WEST));
    }

    pressDPadDirection(dPadDirectionToValue(direction));
}

void XboxGamepadDevice::releaseDPad() {
    pressDPadDirection(XBOX_BUTTON_DPAD_NONE);
}

bool XboxGamepadDevice::isDPadPressed(uint8_t direction) {
    std::lock_guard<std::mutex> lock(_mutex);
    // Serial.print("Internal hat value:");
    // Serial.println(_inputReport.hat, HEX);
    return _inputReport.hat == direction;
    //return (bool)((_inputReport.hat & direction) == direction);
}

bool XboxGamepadDevice::isDPadPressedFlag(XboxDpadFlags direction) {
    std::lock_guard<std::mutex> lock(_mutex);

    if(direction == XboxDpadFlags::NORTH){
        return _inputReport.hat == XBOX_BUTTON_DPAD_NORTH;
    } else if(direction == (XboxDpadFlags::NORTH & XboxDpadFlags::EAST)){
        return _inputReport.hat == XBOX_BUTTON_DPAD_NORTHEAST;
    } else if(direction == XboxDpadFlags::EAST){
        return _inputReport.hat == XBOX_BUTTON_DPAD_EAST;
    } else if(direction == (XboxDpadFlags::SOUTH & XboxDpadFlags::EAST)){
        return _inputReport.hat == XBOX_BUTTON_DPAD_SOUTHEAST;
    } else if(direction == XboxDpadFlags::SOUTH){
        return _inputReport.hat == XBOX_BUTTON_DPAD_SOUTH;
    } else if(direction == (XboxDpadFlags::SOUTH & XboxDpadFlags::WEST)){
        return _inputReport.hat == XBOX_BUTTON_DPAD_SOUTHWEST;
    } else if(direction == XboxDpadFlags::WEST){
        return _inputReport.hat == XBOX_BUTTON_DPAD_WEST;
    } else if(direction == (XboxDpadFlags::NORTH & XboxDpadFlags::WEST)){
        return _inputReport.hat == XBOX_BUTTON_DPAD_NORTHWEST;
    }
    return false;
}


void XboxGamepadDevice::pressShare() {
    std::lock_guard<std::mutex> lock(_mutex);
    // Avoid double presses
    if (!(_inputReport.share & XBOX_BUTTON_SHARE)) {
        _inputReport.share |= XBOX_BUTTON_SHARE;
    }
}

void XboxGamepadDevice::releaseShare() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_inputReport.share & XBOX_BUTTON_SHARE) {
        _inputReport.share ^= XBOX_BUTTON_SHARE;
    }
}

void XboxGamepadDevice::sendGamepadReport(bool defer) {
    if (!_input)
        return;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t packedSize = sizeof(_inputReport);
        ESP_LOGD(LOG_TAG, "Sending gamepad report, size: %d", packedSize);
        _input->setValue((uint8_t*)&_inputReport, packedSize);
    }
    _input->notify();
}