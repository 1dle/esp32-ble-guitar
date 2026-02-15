#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "XboxGamepadDevice.h"
#include "XboxGamepadConfiguration.h"
#include "GuitarHeroAdapter.h"
#include "ConfigManager.h"
#include "InputManager.h"
#include "BleXInputDeviceManager.h"

static const char* TAG = "Main";

// ---- Make these static so the task can access them ----
static ConfigManager config;
static XboxSeriesXControllerDeviceConfiguration* xconfig;
static XboxGamepadDevice* xbox;
static GuitarHeroAdapter* adapter;
static InputManager* input;
static BleXInputDeviceManager* bleManager;

static int advertisingUpdateCounter = 0;

static void inputTask(void* pvParameters)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t delayTicks = pdMS_TO_TICKS(config.getInputTaskDelayMs());

    ESP_LOGI(TAG, "Input task started on CPU %d at %ldms polling rate", 
             xPortGetCoreID(), config.getInputTaskDelayMs());

    while (true)
{
    input->update();

    if (adapter->stateChanged())
    {
        xbox->sendGamepadReport();
        adapter->clearDirty();
    }

    // Update BLE connection state / advertising safely
    bleManager->update();

    vTaskDelayUntil(&lastWakeTime, delayTicks);
}

}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting Guitar Hero BLE Gamepad");

    xconfig = new XboxSeriesXControllerDeviceConfiguration();
    xbox = new XboxGamepadDevice(xconfig);
    adapter = new GuitarHeroAdapter(*xbox);
    input = new InputManager(*adapter, config);

    input->init();

    bleManager = new BleXInputDeviceManager ("GH_BLE_Gamepad", "ESP32", 100);
    bleManager->setDevice(xbox);

    BleHostConfiguration hostConfig = xconfig->getIdealHostConfiguration();
    bleManager->begin(hostConfig);

    ESP_LOGI(TAG, "Initialization complete");

    // Create input task pinned to CPU 1 (NOT CPU 0)
    xTaskCreatePinnedToCore(
        inputTask,
        "input_task",
        4096,
        nullptr,
        1,
        nullptr,
        1
    );

    // app_main should now return or just idle
}
