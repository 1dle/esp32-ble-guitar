#include <Arduino.h>

/*
 * Test all gamepad buttons, axes and dpad 
*/

#include <BleGamepad.h> 

#define numOfButtons 64
#define numOfHatSwitches 0

BleGamepad *bleGamepad = nullptr;


const byte no_buttons = 13;
byte buttons[no_buttons] = {13, 15, 14, 27, 26, 33, 32, 16, 17, 18, 19, 21, 22};
byte led = 23;
byte whammy = 36;

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
  bleGamepad->begin(numOfButtons,numOfHatSwitches);        //Simulation controls are disabled by default

  //digitalWrite(led, HIGH);

}

void whammy_input(){
  int whammy_value = analogRead(whammy);
  int adjusted_value = map(whammy_value, 1920, 0, 32737, -32737);
  bleGamepad->setX(adjusted_value);
}

void loop(){
  //handle_button_inputs();
  //whammy_input();

  if(bleGamepad != nullptr && bleGamepad->isConnected()) 
  {    

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

  whammy_input();

  delay(20);
}