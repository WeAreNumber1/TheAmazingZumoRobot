
#include <ZumoMotors.h>
#include <Pushbutton.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#define LED 13

#define QTR_THRESHOLD  1800 // 

#define REVERSE_SPEED     400 // 0 is stopped, 400 is full speed
#define TURN_SPEED        400
#define FORWARD_SPEED     200
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     300 // ms
#define ATTACK_SPEED     400 
#include <NewPing.h>

const int echoPin = 3;
const int triggerPin = 4;
unsigned int time =0;
float distance =0;
// Maks lengde (in cm)
// Max distance (in cm)
const int maxDistance = 300;
NewPing sonar(triggerPin, echoPin, maxDistance);

ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON); // pushbutton on pin 12

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

ZumoReflectanceSensorArray sensors;

void setup()
{
  Serial.begin(9600);
  sensors.init();
  button.waitForButton();
}

void loop()
{
  time = sonar.ping();
  distance = sonar.convert_cm(time);
  Serial.println(distance);
  sensors.read(sensor_values);

  if (sensor_values[0] > QTR_THRESHOLD)
  {
    rightupdown();
  }
  else if (sensor_values[5] > QTR_THRESHOLD)
  {
    leftupdown();
  }
  else
  {
    time = sonar.ping();
    distance = sonar.convert_cm(time);

    /*while(distance<20){
     attack();
     }*/

    motors.setSpeeds(200, 200);

  }
}

void rightupdown()
{
  // if leftmost sensor detects line, reverse and turn to the right
  motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
  delay(REVERSE_DURATION);
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  delay(TURN_DURATION);
  motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
}

void leftupdown()
{
  // if rightmost sensor detects line, reverse and turn to the left
  motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
  delay(REVERSE_DURATION);
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  delay(TURN_DURATION);
  motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
}

void attack(){
  motors.setSpeeds(ATTACK_SPEED, ATTACK_SPEED);
  time = sonar.ping();
  distance = sonar.convert_cm(time);
  if (sensor_values[0] > QTR_THRESHOLD)
  {
    rightupdown();
  }
  else if (sensor_values[5] > QTR_THRESHOLD)
  {
    leftupdown();
  }
}
