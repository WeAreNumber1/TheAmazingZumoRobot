#include <PLab_IRremote.h>
#include <PLab_IRremoteInt.h>
#include <PLab_PushButton.h>

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>

#include <NewServo.h>
#define SERVO_PIN 6
NewServo servo;
int pos = 180;

ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
ZumoBuzzer buzzer;
Pushbutton button(ZUMO_BUTTON);
int lastError = 0;
const int MAX_SPEED = 200;

// Define thresholds for border
#define BORDER_VALUE_LOW  400 // border low

int startDest = 0; //HOME
int destination = startDest; // Write the destination here.
unsigned int sensors[6];
bool doUpdate = false; // Is true if followLine was not called this cycle.
enum State { HOME, GOHOME, GOTODEST, STOPATLINE, EXTINGUISH, STOP };
enum State state = HOME;

void setup() {
  btSerial.begin(9600);
  //Setup Things...
  servo.attach(SERVO_PIN);
  servo.write(pos);

  // Initialize the reflectance sensors module
  reflectanceSensors.init();

  // Wait for the user button to be pressed and released
  button.waitForButton();
  delay(1000);
  int i;
  for (i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-100, -100);
    else
      motors.setSpeeds(120, 120);
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*10 = 800 ms.
    delay(10);
  }
  motors.setSpeeds(0, 0);
  for (int i = 0; i < 3; i++) {
    buzzer.playNote(NOTE_A(4), 500, 15);
    delay(1000);
  }
  buzzer.playNote(NOTE_A(5), 1000, 15);
  accelerateOver(0, 200, 500, true); //Should be over the first line after calibration. Set to false if this is not true.
  reflectanceSensors.readLine(sensors); // Setting initial value for sensor array.
}

void beepNumber(int number) { // Beeps an amount of times equal to number.
  motors.setSpeeds(0, 0);
  if (number == 0)
    buzzer.playNote(NOTE_A(4), 1000, 15);
  else
    for (int i = 0; i < number; i++) {
      buzzer.playNote(NOTE_A(5), 500, 15);
      delay(1000);
    }
}
int sign(int val) { // Returns -1, 0 or 1 based on the sign of val.
  return (val > 0) - (val < 0);
}

int sensorAverage(unsigned int sensors[]) { // Returns average value of sensor array.
  int sum = 0;
  for (int i = 0; i < 6; i++)
    sum += sensors[i];
  return sum / 6;
}
int sensorMax(unsigned int sensors[]) { // Returns highest value in sensor array.
  int max = 0;
  for (int i = 0; i < 6; i++)
    max = (max > sensors[i] ? max : sensors[i]);
  return max;
}
bool perpLine(unsigned int sensors[]) { // Returns true if the sensors sense a perpendicular line.
  return sensors[0] > BORDER_VALUE_LOW && sensors[5] > BORDER_VALUE_LOW;
}
bool noLine(unsigned int sensors[]) { // Returns true if the sensors can't sense a line.
  return sensorMax(sensors) < 100;
}

void accelerateOver(int startSpeed, int goalSpeed, float duration, bool doFollowLine) { // Accelerates from startSpeed to goalSpeed over duration.
  int diff = goalSpeed - startSpeed;
  for (float i = 0; i < duration; i++) {
    int speed = startSpeed + int((float)(i / duration) * diff);
    if (doFollowLine) followLine(speed);
    else {
      motors.setSpeeds(speed, speed);
      delay(1);
    }
  }
}
void turnLeft() {
  motors.setSpeeds(-100, 200);
  delay(500);
}
void turnRight() {
  motors.setSpeeds(200, -100);
  delay(500);
}
void turn180() {
  motors.setSpeeds(200, -200);
  delay(750);
}

void updateSensors() { // Updates the reflectance sensor array.
  reflectanceSensors.readLine(sensors);
}
void followLine   () {
  followLine(MAX_SPEED);
}
void followLine   (int maxSpeed) { // Primary line following function.
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
  int m1Speed = maxSpeed + speedDifference;
  int m2Speed = maxSpeed - speedDifference;

  motors.setSpeeds(
  constrain(m1Speed, 0, maxSpeed),
  constrain(m2Speed, 0, maxSpeed));
  doUpdate = false;
}

void putOutFire(){
  servo.write(80);              // tell servo to go to position in variable 'pos'
  delay(20);
  servo.write(80);    //Just making sure that the thing is down.
  delay(5000);

  servo.write(180);              // tell servo to go to position in variable 'pos'

}

void loop() {
  doUpdate = true;
  BT_update();
  switch (state) {
  case HOME:
    if (BT_hasNewDestination())
    {
      if(BT_getDestination() != 0){
        destination = BT_getDestination();
        state = GOTODEST;
      }
    }

    break;
  case GOHOME:
    while (!perpLine(sensors)) followLine();
    if (destination == 2)
      turnRight();
    else
      turnLeft();
    while (!noLine(sensors)) followLine();
    accelerateOver(200, 0, 250, false);
    turn180();
    startDest = 0;
    destination = startDest;
    beepNumber(destination); //This doesnt make sense anymore..
    if (startDest < 1) {
      state = HOME;
      BT_sendHasReturned();
    }
    else {
      state = GOTODEST;
      accelerateOver(0, 200, 250, true);
    }
    break;
  case GOTODEST:
    if (perpLine(sensors)) {
      motors.setSpeeds(0, 0);
      //delay(20);
      if (destination > 2) {
        motors.setSpeeds(200, 200);
        delay(200);
        destination -= 2;
      }
      else {
        if (destination == 2)
          turnLeft();
        else
          turnRight();
      }
      //state = STOPATLINE;
    }
    else if (noLine(sensors)) {
      motors.setSpeeds(0,0);
      state = EXTINGUISH;
    }
    else
      followLine();
    break;
  case STOPATLINE:
    while (!perpLine(sensors)) followLine();
    motors.setSpeeds(0, 0);
    break;
  case EXTINGUISH:
    // while (true){
    //   accelerateOver(0, 200, 1500, false);
    //   accelerateOver(150, 300, 3000, false);
    //   accelerateOver(250, 400, 4000, false);
    //   motors.setSpeeds(400, 400);
    //   delay(2000);
    //   // motors.setSpeeds(400, 400);
    //   // delay(1000);
    //   accelerateOver(400, 0, 750, false);
    //   delay(1000);
    //   // turn180();
    // }
    putOutFire();
    while (!BT_shallReturn()){
      putOutFire();
    }
    state = GOHOME;
    turn180();
    break;
  case STOP:
    motors.setSpeeds(0, 0);
    break;
  }
  if (doUpdate)
    updateSensors();
}
