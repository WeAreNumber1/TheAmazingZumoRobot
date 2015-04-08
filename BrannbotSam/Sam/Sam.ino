#include <PLab_IRremote.h>
#include <PLab_IRremoteInt.h>
#include <PLab_PushButton.h>

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <Pushbutton.h>


#include "PLab_IRremote.h"
/*-----( Declare Constants )-----*/
int receiver = 2; // pin 1 of IR receiver to Arduino digital pin 11

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);           // create instance of 'irrecv'
decode_results results;            // create instance of 'decode_results'
/*-----( Declare Variables )-----*/

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;
const int MAX_SPEED = 200;

// Define thresholds for border
#define BORDER_VALUE_LOW  400 // border low

int destination = 3; //Write the destination here.

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
  /*button.waitForButton();*/
}

int sensorAverage(int sensors[]){
  int sum = 0;
  for (int i = 0; i < 6; i++)
    sum += sensors[i];
  return sum / 6;
}

int constrain(int value, int min, int max){
  if (value < min){
    return min;
  } else if (value > max){
    return max;
  } else {
    return value;
  }
}

int cooldown = 0;
void loop() {
  //Main program
  unsigned int sensors[6];

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

  //Navigates to destination.
  cooldown --;
  if (cooldown <= 0 && sensorAverage(sensors) > BORDER_VALUE_LOW){ //sensors[0] > BORDER_VALUE_LOW && sensors[5] > BORDER_VALUE_LOW){
    motors.setSpeeds(0,0);
    delay(20);
    if (destination > 2 ){
      //Continue past the line
      motors.setSpeeds(200,200);
      destination -= 2;
    }else if (destination == 2){
      //Turn Left
      motors.setSpeeds(0,200);
    }else{
      //Turn Right
      motors.setSpeeds(200,0);
    }
    delay(500);
    cooldown = 200;
  }else{
    //Constraining and setting motor speeds.
    motors.setSpeeds(
      constrain(m1Speed, 0, MAX_SPEED),
      constrain(m2Speed, 0, MAX_SPEED));

    /*if ( destination < 2 ){
      delay(200);
      motors.setSpeeds(0,0);
    }*/
  }
}
