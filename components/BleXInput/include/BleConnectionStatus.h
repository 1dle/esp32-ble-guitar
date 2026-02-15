#pragma once
#include <NimBLEDevice.h>

class BleConnectionStatus : public NimBLEServerCallbacks {
public:
    BleConnectionStatus();

    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo);

    bool isConnected() const { return connected; }
    void update(); // call from loop()

private:
    enum class AdvState {
        Advertising,
        Connected,
        ReconnectWindow
    };

    AdvState _advState = AdvState::Advertising;
    uint16_t _activeConnHandle = BLE_HS_CONN_HANDLE_NONE;
    bool connected = false;
    bool _paramUpdatePending = false;
    bool _advertisingActive = false;

    uint32_t _lastAdvStopMs = 0;
    uint32_t _lastDisconnectMs = 0;
    uint32_t _connEstablishedMs = 0;

    static constexpr uint32_t ADV_RESTART_DELAY_MS = 1000; // safer delay
    static constexpr uint32_t RECONNECT_WINDOW_MS = 1000;
    static constexpr uint32_t PARAM_UPDATE_DELAY_MS = 800;
};
