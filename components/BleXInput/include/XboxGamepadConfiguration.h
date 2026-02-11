#pragma once

#include "XboxDescriptors.h"
#include "BleHostConfiguration.h"

class XboxSeriesXControllerDeviceConfiguration {
public:
    XboxSeriesXControllerDeviceConfiguration(uint8_t reportId = XBOX_INPUT_REPORT_ID);
    virtual ~XboxSeriesXControllerDeviceConfiguration() = default;

    bool getAutoReport() const { return _autoReport; }
    void setAutoReport(bool v) { _autoReport = v; }

    void setAutoDefer(bool v) { _autoDefer = v; }
    bool getAutoDefer() const { return _autoDefer; }

    uint8_t getReportId() const { return _reportId; }
    void setHidReportId(uint8_t id) { _reportId = id; }

    virtual const char* getDeviceName() const { return "XboxSeriesX"; }
    virtual BleHostConfiguration getIdealHostConfiguration() const;
    virtual uint8_t getDeviceReportSize() const;
    virtual size_t makeDeviceReport(uint8_t* buffer, size_t bufferSize) const;

private:
    bool _autoReport;
    bool _autoDefer;
    uint8_t _reportId;
};