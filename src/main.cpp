#include <Arduino.h>
#include "esp_bt.h"
#include <BleGamepad.h> 

#define numOfButtons 64
#define numOfHatSwitches 0

BleGamepad bleGamepad;


const byte no_buttons = 13;
byte buttons[no_buttons] = {4, 15, 14, 27, 26, 33, 32, 16, 17, 18, 19, 21, 22};
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

  //bleGamepad = new BleGamepad("deli's guitar", "DELI");

  
  //Serial.println("Starting BLE work!");
  BleGamepadConfiguration bleGamepadConfig;
  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);  //CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepadConfig.setButtonCount(numOfButtons);
  bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);
  bleGamepad.begin(&bleGamepadConfig);       //Simulation controls are disabled by 

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

  bleGamepad.setX(map(averageWhammy, 1920, 0, 32737, -32737));
  return true;
}

void loop(){
  //handle_button_inputs();
  //whammy_input();
  if(bleGamepad.isConnected()) 
  { 
    bool reportRequired = whammy_input();

    for(byte i = 0; i < no_buttons; i++){
      //itearate through every input buttons
      currentButtonStates[i]  = digitalRead(buttons[i]);
    
      if (currentButtonStates[i] != previousButtonStates[i])
      {
        if(currentButtonStates[i] == LOW)
          bleGamepad.press(i+1);
        else
          bleGamepad.release(i+1);

        previousButtonStates[i] = currentButtonStates[i];
        reportRequired = true;
      } 
    }

    if (reportRequired)
    {
      bleGamepad.sendReport();
    }

  }
}