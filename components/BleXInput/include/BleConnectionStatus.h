#pragma once

#include <NimBLEServer.h>
#include "NimBLECharacteristic.h"
#include "NimBLEConnInfo.h"

class BleConnectionStatus : public NimBLEServerCallbacks
{
public:
    BleConnectionStatus(void);
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override;
    //NimBLECharacteristic *inputGamepad;
    bool isConnected();
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
private:
    bool connected = false;
};
