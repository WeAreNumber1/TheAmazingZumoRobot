#include <PLab_IRremote.h>
#include <PLab_IRremoteInt.h>
#include <PLab_PushButton.h>

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;
const int MAX_SPEED = 200;

// Define thresholds for border
#define BORDER_VALUE_LOW  400 // border low

int destination = 3; // Write the destination here.
unsigned int sensors[6];
bool doUpdate = false; // Is true if followLine was not called this cycle.
enum State { GOHOME, GOTODEST, STOPATLINE, EXTINGUISH, STOP };
enum State state = GOTODEST;

void setup() {
  //Setup Things...
  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Wait for the user button to be pressed and released
  button.waitForButton();
  delay(1000);
  int i;
  for (i = 0; i < 80; i++){
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-100, -100);
    else
      motors.setSpeeds(100, 100);
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(10);
  }
  motors.setSpeeds(200,0);
  delay(400);
  reflectanceSensors.readLine(sensors);
  /*button.waitForButton();*/
}

int sensorAverage(unsigned int sensors[]){
  int sum = 0;
  for (int i = 0; i < 6; i++)
    sum += sensors[i];
  return sum / 6;
}
int sensorMax(unsigned int sensors[]){
  int max = 0;
  for (int i = 0; i < 6; i++)
    max = (max > sensors[i] ? max : sensors[i]);
  return max;
}
bool perpLine(unsigned int sensors[]){ // Returns true if the sensors sense a perpendicular line.
  return sensors[0] > BORDER_VALUE_LOW && sensors[5] > BORDER_VALUE_LOW;
}
bool noLine(unsigned int sensors[]){ // Returns true if the sensors can't sense a line.
  return sensorMax(sensors) < 100;
}

void turnLeft (){
  motors.setSpeeds(-50,200);
  delay(500);
}
void turnRight(){
  motors.setSpeeds(200,-50);
  delay(500);
}
void turn180  (){
  motors.setSpeeds(200,-200);
  delay(750);
}

void updateSensors(){ // Updates the reflectance sensor array.
  reflectanceSensors.readLine(sensors);
}
void followLine(){
  //Main program
  //unsigned int sensors[6];

  // Get the position of the line.
  int position = reflectanceSensors.readLine(sensors);

  // Our "error" is how far we are away from the center of the line, which
  // corresponds to position 2500.
  int error = position - 2500;

  // Get motor speed difference using proportional and derivative PID terms
  // (the integral term is generally not very useful for line following).
  // Here we are using a proportional constant of 1/4 and a derivative
  // constant of 6, which should work decently for many Zumo motor choices.
  // You probably want to use trial and error to tune these constants for
  // your particular Zumo and line course.
  int speedDifference = error / 4 + 6 * (error - lastError);

  lastError = error;

  // Get individual motor speeds.  The sign of speedDifference
  // determines if the robot turns left or right.
  int m1Speed = MAX_SPEED + speedDifference;
  int m2Speed = MAX_SPEED - speedDifference;

  motors.setSpeeds(
    constrain(m1Speed, 0, MAX_SPEED),
    constrain(m2Speed, 0, MAX_SPEED));
  doUpdate = false;
}

void loop() {
  doUpdate = true;
  //Main program
  // unsigned int sensors[6];

  // Fills the sensor array.
  // int position = reflectanceSensors.readLine(sensors);

  switch (state){
    case GOHOME:
      while (!perpLine(sensors)) followLine();
      if (destination == 2)
        turnRight();
      else
        turnLeft();
      while (!noLine(sensors)) followLine();
      turn180();
      state = STOP;
      break;
    case GOTODEST:
      if (perpLine(sensors)){
        motors.setSpeeds(0,0);
        //delay(20);
        if (destination > 2){
          motors.setSpeeds(200, 200);
          delay(200);
          destination -= 2;
        }
        else if (destination == 2)
          turnLeft();
        else
          turnRight();
      }
      else if (noLine(sensors)){
        turn180();
        state = GOHOME;
      }
      else
        followLine();
      break;
    case STOPATLINE:

      break;
    case EXTINGUISH:

      break;
    case STOP:
      motors.setSpeeds(0, 0);
      break;
  }
  if (doUpdate)
    updateSensors();

  //Change states


}
