#include <Arduino.h>
#include "esp_bt.h"
#include <BleGamepad.h> 

#define numOfButtons 64
#define numOfHatSwitches 0

BleGamepad *bleGamepad = nullptr;


const byte no_buttons = 13;
byte buttons[no_buttons] = {13, 15, 14, 27, 26, 33, 32, 16, 17, 18, 19, 21, 22};
const byte led = 23;
const byte whammy = 36;

const byte pot_samples = 5;

byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];

void setup() 
{
  //init button inputs.
  for(byte i = 0; i < no_buttons; i++){
    pinMode(buttons[i], INPUT_PULLUP);
    previousButtonStates[no_buttons] = HIGH;
    currentButtonStates[no_buttons] =  HIGH;
  }
  pinMode(led, OUTPUT);

  bleGamepad = new BleGamepad("deli's guitar", "DELI");

  
  //Serial.println("Starting BLE work!");
  bleGamepad->setAutoReport(false);
  bleGamepad->setControllerType(CONTROLLER_TYPE_GAMEPAD);  //CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepad->begin(numOfButtons,numOfHatSwitches);        //Simulation controls are disabled by 

  //inrease transmit power
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  //esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);

  //digitalWrite(led, HIGH);

}

void whammy_input(){
  int sum_whammy_val = 0;
  for(byte i = 0; i < pot_samples; i++){
    sum_whammy_val = analogRead(whammy);
    delay(4);
  }
  bleGamepad->setX(map(sum_whammy_val / pot_samples, 1920, 0, 32737, -32737));
}

void loop(){
  //handle_button_inputs();
  //whammy_input();
  if(bleGamepad != nullptr && bleGamepad->isConnected()) 
  { 
    whammy_input(); // = delay 20 (5 samples 4ms delay between them)

    for(byte i = 0; i < no_buttons; i++){
      //itearate through every input buttons
      currentButtonStates[i]  = digitalRead(buttons[i]);
    
      if (currentButtonStates[i] != previousButtonStates[i])
      {
        if(currentButtonStates[i] == LOW)
          bleGamepad->press(i+1);
        else
          bleGamepad->release(i+1);
      } 
    }
    
    if (currentButtonStates != previousButtonStates)
    {
      for (byte i = 0 ; i < numOfButtons ; i++)
      {
        previousButtonStates[i] = currentButtonStates[i]; 
      }
      bleGamepad->sendReport();
    }

  }
}