#include <math.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "BleConnectionStatus.h"
#include "BleXinputDeviceManager.h"
#include "XboxGamepadDevice.h"


static const char *TAG = "ESP32-HID";

static const gpio_num_t LED_PIN = GPIO_NUM_5;

XboxGamepadDevice *gamepad;
BleXInputDeviceManager bleDeviceManager("ESP32 SeriesX Controller", "Mystfit", 100);

static void OnVibrateEvent(XboxGamepadOutputReportData data)
{
    if (data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0) {
        gpio_set_level(LED_PIN, 0);
    } else {
        gpio_set_level(LED_PIN, 1);
    }

    ESP_LOGI(TAG, "Vibration event. Weak motor: %d Strong motor: %d",
             data.weakMotorMagnitude, data.strongMotorMagnitude);
}

static void testButtons()
{
    uint16_t buttons[] = {
        XBOX_BUTTON_A,
        XBOX_BUTTON_B,
        XBOX_BUTTON_X,
        XBOX_BUTTON_Y,
        XBOX_BUTTON_LB,
        XBOX_BUTTON_RB,
        XBOX_BUTTON_START,
        XBOX_BUTTON_SELECT,
        XBOX_BUTTON_LS,
        XBOX_BUTTON_RS
    };

    for (uint16_t button : buttons)
    {
        ESP_LOGI(TAG, "Pressing button %d", button);
        gamepad->press(button);
        gamepad->sendGamepadReport();
        vTaskDelay(pdMS_TO_TICKS(500));

        gamepad->release(button);
        gamepad->sendGamepadReport();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    gamepad->pressShare();
    gamepad->sendGamepadReport();
    vTaskDelay(pdMS_TO_TICKS(500));
    gamepad->releaseShare();
    gamepad->sendGamepadReport();
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void testPads()
{
    XboxDpadFlags directions[] = {
        XboxDpadFlags::NORTH,
        XboxDpadFlags((uint8_t)XboxDpadFlags::NORTH | (uint8_t)XboxDpadFlags::EAST),
        XboxDpadFlags::EAST,
        XboxDpadFlags((uint8_t)XboxDpadFlags::EAST | (uint8_t)XboxDpadFlags::SOUTH),
        XboxDpadFlags::SOUTH,
        XboxDpadFlags((uint8_t)XboxDpadFlags::SOUTH | (uint8_t)XboxDpadFlags::WEST),
        XboxDpadFlags::WEST,
        XboxDpadFlags((uint8_t)XboxDpadFlags::WEST | (uint8_t)XboxDpadFlags::NORTH)
    };

    for (XboxDpadFlags direction : directions)
    {
        ESP_LOGI(TAG, "Pressing DPad: %d", (int)direction);
        gamepad->pressDPadDirectionFlag(direction);
        gamepad->sendGamepadReport();
        vTaskDelay(pdMS_TO_TICKS(500));

        gamepad->releaseDPad();
        gamepad->sendGamepadReport();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void testTriggers()
{
    for (int16_t val = XBOX_TRIGGER_MIN; val <= XBOX_TRIGGER_MAX; val++)
    {
        if (val % 8 == 0)
            ESP_LOGI(TAG, "Setting trigger value to %d", val);

        gamepad->setLeftTrigger(val);
        gamepad->setRightTrigger(val);
        gamepad->sendGamepadReport();
        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

static void testThumbsticks()
{
    int startTime = (int)esp_timer_get_time() / 1000;
    int reportCount = 0;

    while ((esp_timer_get_time() / 1000) - startTime < 8000)
    {
        reportCount++;
        int16_t x = cos((float)esp_timer_get_time() / 1000000.0f) * XBOX_STICK_MAX;
        int16_t y = sin((float)esp_timer_get_time() / 1000000.0f) * XBOX_STICK_MAX;

        gamepad->setLeftThumb(x, y);
        gamepad->setRightThumb(x, y);
        gamepad->sendGamepadReport();

        if (reportCount % 8 == 0)
            ESP_LOGI(TAG, "Setting left thumb to %d, %d", x, y);

        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //gpio_pad_select_gpio(LED_PIN);
    //gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    XboxSeriesXControllerDeviceConfiguration *config = new XboxSeriesXControllerDeviceConfiguration();

    BleHostConfiguration hostConfig = config->getIdealHostConfiguration();

    ESP_LOGI(TAG, "Using VID source: 0x%04x", hostConfig.getVidSource());
    ESP_LOGI(TAG, "Using VID: 0x%04x", hostConfig.getVid());
    ESP_LOGI(TAG, "Using PID: 0x%04x", hostConfig.getPid());
    ESP_LOGI(TAG, "Using GUID version: 0x%02x", hostConfig.getGuidVersion());
    //ESP_LOGI(TAG, "Using serial number: %s", hostConfig.getSerialNumber().c_str());

    gamepad = new XboxGamepadDevice(config);

    //FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
    //gamepad->onVibrate.attach(vibrationSlot);

    bleDeviceManager.setDevice(gamepad);

    ESP_LOGI(TAG, "Starting composite HID device...");
    bleDeviceManager.begin(hostConfig);

    while (true)
    {
        if (bleDeviceManager.isConnected())
        {
            //testButtons();
            //testPads();
            //testTriggers();
            //testThumbsticks();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}