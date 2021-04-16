 // Public Library
//-------------------------
#include <Wire.h>
#include <ZumoShield.h>

//Local Library
//-------------------------
#include "Movement.h"
#include "Timer.h"
#include "Sens.h"

//Named Local Library
//-------------------------
Movement mov;
Sens IR_R;
Timer rev_timer;
Timer turn_timer;
Timer test_timer;

//Name of Public Library
//-------------------------
ZumoBuzzer buzzer;
ZumoMotors flip;
Pushbutton button(ZUMO_BUTTON); // pushbutton on pin 12
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN);

//=======================================================================
//Constants and Variables

const int LED = 13;


const int rev_L = 1001;
const int rev_R = 1002;
const int turn_L = 1003;
const int turn_R = 1004;
const int forward = 1005;
const int search = 1006;

int currentState = search;


const int NUM_SENSORS = 6;
unsigned int sensor_values[NUM_SENSORS];

const int IR_SENS_PIN = A0;
const int IRLimit = 200;



// the treshold for the floor sensors
// this might need to be tuned for different lighting conditions, surfaces, etc.
const int QTR_THRESHOLD = 1500; // microseconds

//motor speed
// these might need to be tuned for different motor types
const int REVERSE_SPEED = 200; // 0 is stopped, 400 is full speed
const int TURN_SPEED = 200;
const int FORWARD_SPEED = 200;
const int REVERSE_DURATION = 1000; // ms
const int TURN_DURATION = 2000; // ms
const int TEST_DURATION = 10000; // ms

//========================================================================
// functions

void waitForButtonAndCountDown()
{
  digitalWrite(LED, HIGH);
  button.waitForButton();
  digitalWrite(LED, LOW);

  // play audible countdown
  for (int i = 0; i < 3; i++)
    {
    delay(1000);
    buzzer.playNote(NOTE_G(3), 200, 15);
    }
  delay(1000);
  buzzer.playNote(NOTE_G(4), 500, 15);
  delay(1000);
  //test_timer.getTimer(TEST_DURATION);
}

void changeStateTo (int newState)
  {
  currentState = newState;
  }

void sensorValues()
  {
  Serial.print("Sensor 0 detects: ");
  Serial.println(sensor_values[0]);
  Serial.print("Sensor 5 detects: ");
  Serial.println(sensor_values[5]);
  Serial.println("");  
  }


//========================================================================

void setup()
{
  Serial.begin(9600);
  flip.flipLeftMotor(true);
  flip.flipRightMotor(false);
  pinMode(LED, HIGH);
  waitForButtonAndCountDown();
}

void loop()
  {  
  // Initialize speeds, and puts them inside of Movement class
  mov.initSpeed(FORWARD_SPEED, REVERSE_SPEED, TURN_SPEED, REVERSE_DURATION, TURN_DURATION);
  /*

  if(test_timer.timerHasExpired())
    {
    mov.wait();  
    }*/
  if (button.isPressed())
    {
    // if button is pressed, stop and wait for another press to go again
    mov.wait();
    button.waitForRelease();
    waitForButtonAndCountDown();
    test_timer.getTimer(TEST_DURATION);
    }

  // Checking what values sensors are detecting, printing them to Serialmonitor,
  // so that we can adjust QTR_THRESHHOLD for the enviorment 
  sensors.read(sensor_values); 
  // sensorValues();

//================================================================================
//Actual actions after initializing

  int ir = IR_R.readIR(IR_SENS_PIN, IRLimit); // Infra red sensor reading

//================================================================================
// movement controll
  if(sensor_values[5] > QTR_THRESHOLD) 
    {
    rev_timer.getTimer(REVERSE_DURATION);
    changeStateTo(rev_R);
    }
  else if(sensor_values[0] > QTR_THRESHOLD)
    {
    rev_timer.getTimer(REVERSE_DURATION);
    changeStateTo(rev_L); 
    }
  
  else if (currentState > 1002 and ir)
  {
    changeStateTo(forward);
    }  
  else if (currentState > 1004 and !ir)
    {
    if(IR_R.readIR(IR_SENS_PIN, IRLimit))
      {
      changeStateTo(forward);
      }
    else
      {
      changeStateTo(search);
      }
    }
//================================================================================
// motor controll
   
  switch(currentState)
    {
    case forward:
      mov.forward();
      break;

    case rev_L:
      mov.rev();
      if(rev_timer.timerHasExpired())
        {
        turn_timer.getTimer(TURN_DURATION);
        changeStateTo(turn_R); 
        }
      break;

    case rev_R:
      mov.rev();
      if(rev_timer.timerHasExpired())
        {
        turn_timer.getTimer(TURN_DURATION);
        changeStateTo(turn_L);  
        }
      break;

    case turn_R:
      mov.turn_R();
      if(turn_timer.timerHasExpired())
        {
        changeStateTo(search);
          
        }
      break;

    case turn_L:
      mov.turn_L();
      if(turn_timer.timerHasExpired())
        {
        changeStateTo(search);  
        
        }

        break;

     case search:
     mov.wait();
     break;
    }
    
    
}
