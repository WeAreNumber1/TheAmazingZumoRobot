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

int cooldown = 0;
int destination = 4; //Write the destination here.
void loop() {
  //Main program
  unsigned int sensors[6];

  // Get the position of the line.  Note that we *must* provide the "sensors"
  // argument to readLine() here, even though we are not interested in the
  // individual sensor readings
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

//TODO!
//Stop at perpendicular lines... Blah!
  cooldown --;
  if (cooldown <= 0 && sensors[0] > BORDER_VALUE_LOW && sensors[5] > BORDER_VALUE_LOW){
    motors.setSpeeds(0,0);
    delay(20);
    if (destination > 2 ){
      motors.setSpeeds(200,200);
      destination -= 2;
     }else if (destination == 2){
      motors.setSpeeds(0,200);
    }else{
      motors.setSpeeds(200,0);
    }
    delay(500);
    cooldown = 200;
  }else{
  // Here we constrain our motor speeds to be between 0 and MAX_SPEED.
  // Generally speaking, one motor will always be turning at MAX_SPEED
  // and the other will be at MAX_SPEED-|speedDifference| if that is positive,
  // else it will be stationary.  For some applications, you might want to
  // allow the motor speed to go negative so that it can spin in reverse.
  if (m1Speed < 0)
    m1Speed = 0;
  if (m2Speed < 0)
    m2Speed = 0;
  if (m1Speed > MAX_SPEED)
    m1Speed = MAX_SPEED;
  if (m2Speed > MAX_SPEED)
    m2Speed = MAX_SPEED;

  motors.setSpeeds(m1Speed, m2Speed);
  }
}
