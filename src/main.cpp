#include <Arduino.h>
#include "esp_bt.h"
#include <BleXInputDeviceManager.h>
#include <XboxGamepadDevice.h>
#include <GuitarHeroAdapter.h>
#include <ConfigManager.h>

#define numOfButtons 13

XboxGamepadDevice xboxGamepad;
BleXInputDeviceManager bleDeviceManager("GH BLE Xinput guitar", "delelele");
GuitarHeroAdapter guitarAdapter(xboxGamepad);


const byte no_buttons = 13;
byte buttons[no_buttons] = {4, 15, 14, 27, 26, 33, 32, 16, 17, 18, 19, 21, 22};
const ButtonMapping buttonMappings[] = {
  {0, GHInput::FRET_0},
  {1, GHInput::FRET_1},
  {2, GHInput::FRET_2},
  {3, GHInput::FRET_3},
  {4, GHInput::FRET_4},
  {5, GHInput::STRUM_UP},
  {6, GHInput::STRUM_DOWN}
};

const byte led = 23;
const byte whammy = 36;

const byte pot_samples = 5;
const uint32_t whammy_sample_interval_ms = 4;

int whammyAccumulated = 0;
byte whammySampleCount = 0;
uint32_t lastWhammySampleAt = 0;

byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];

void setup() 
{
  //init button inputs.
  for(byte i = 0; i < no_buttons; i++){
    pinMode(buttons[i], INPUT_PULLUP);
    previousButtonStates[i] = HIGH;
    currentButtonStates[i] =  HIGH;
  }
  pinMode(led, OUTPUT);

  BleHostConfiguration hostConfig = xboxGamepad.getDeviceConfig()->getIdealHostConfiguration();
  bleDeviceManager.setDevice(&xboxGamepad);
  bleDeviceManager.begin(hostConfig);

  //inrease transmit power
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);

  //digitalWrite(led, HIGH);

}

bool whammy_input(){
  uint32_t now = millis();
  if (now - lastWhammySampleAt < whammy_sample_interval_ms) {
    return false;
  }

  lastWhammySampleAt = now;
  whammyAccumulated += analogRead(whammy);
  whammySampleCount++;

  if (whammySampleCount < pot_samples) {
    return false;
  }

  int averageWhammy = whammyAccumulated / pot_samples;
  whammyAccumulated = 0;
  whammySampleCount = 0;

  uint16_t whammyValue = (uint16_t)constrain(map(averageWhammy, 1920, 0, 0, 1023), 0, 1023);
  guitarAdapter.setWhammy(whammyValue);
  return guitarAdapter.stateChanged();
}

void loop(){
  //handle_button_inputs();
  //whammy_input();
  if(bleDeviceManager.isConnected()) 
  { 
    bool reportRequired = whammy_input();

    for(byte i = 0; i < no_buttons; i++){
      //itearate through every input buttons
      currentButtonStates[i]  = digitalRead(buttons[i]);
    
      if (currentButtonStates[i] != previousButtonStates[i])
      {
        bool pressed = currentButtonStates[i] == LOW;
        for (const auto& mapping : buttonMappings) {
          if (mapping.buttonIndex == i) {
            guitarAdapter.setInput(mapping.input, pressed);
            break;
          }
        }

        previousButtonStates[i] = currentButtonStates[i];
        reportRequired = reportRequired || guitarAdapter.stateChanged();
      } 
    }

    if (reportRequired && guitarAdapter.stateChanged())
    {
      guitarAdapter.clearDirty();
      bleDeviceManager.enqueueReport();
    }

  }
}