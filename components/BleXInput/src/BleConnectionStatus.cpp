#include "BleConnectionStatus.h"

static const char* LOG_TAG = "BleConnectionStatus";

BleConnectionStatus::BleConnectionStatus()
{
    _advState = AdvState::Advertising;
    _advertisingActive = false;
    connected = false;
    _activeConnHandle = BLE_HS_CONN_HANDLE_NONE;
}


void BleConnectionStatus::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo)
{
    ESP_LOGI(LOG_TAG, "Connected");

    NimBLEDevice::getAdvertising()->stop();

    _activeConnHandle = connInfo.getConnHandle();
    _advState = AdvState::Connected;
    _connEstablishedMs = esp_timer_get_time() / 1000;
    _paramUpdatePending = true;
    connected = true;
    _advertisingActive = false;
}

void BleConnectionStatus::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
{
    ESP_LOGW(LOG_TAG, "Disconnected: %d", reason);
    _activeConnHandle = BLE_HS_CONN_HANDLE_NONE;
    connected = false;
    _paramUpdatePending = false;
    _lastDisconnectMs = esp_timer_get_time() / 1000;
    _advState = AdvState::ReconnectWindow;
    _advertisingActive = false;
}

void BleConnectionStatus::onAuthenticationComplete(NimBLEConnInfo& connInfo)
{
    ESP_LOGI(LOG_TAG, "Authentication complete");
    connected = true;
}

void BleConnectionStatus::update()
{
    uint32_t now = esp_timer_get_time() / 1000;

    switch (_advState)
    {
        case AdvState::Connected:
            if (_paramUpdatePending && now - _connEstablishedMs >= PARAM_UPDATE_DELAY_MS) {
               NimBLEDevice::getServer()->updateConnParams(
                    _activeConnHandle, 
                    6, //min interval (7.5ms)
                    12, //max interval (15ms)
                    0, 
                    400
                );
                _paramUpdatePending = false;
                ESP_LOGI(LOG_TAG, "Connection parameters updated");
            }
            break;

        case AdvState::ReconnectWindow:
            if (now - _lastDisconnectMs >= RECONNECT_WINDOW_MS) {
                _advState = AdvState::Advertising;
            }
            break;

        case AdvState::Advertising:
            if (connected) break;
            if (!_advertisingActive && now - _lastAdvStopMs >= ADV_RESTART_DELAY_MS) {
                ESP_LOGI(LOG_TAG, "Starting advertising");
                if (NimBLEDevice::getAdvertising()->start()) {
                    _advertisingActive = true;
                    _lastAdvStopMs = now;
                }
            }
            break;
    }
}
