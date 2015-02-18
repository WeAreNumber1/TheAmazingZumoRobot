#include <Wire.h>
#include <LSM303.h>

// initialize vector values
LSM303 compass;
LSM303::vector<int16_t> vectorOffset = {0, 0, 0};
LSM303::vector<int16_t> vectorValues = {0, 0, 0};
boolean collisionDetected = false;
#define COLLISION_THRESHOLD 1000   // MUST BE CHANGED/CALIBRATED

void setup() {
  // Start up accelerometer
  Wire.begin();
  compass.init();
  compass.enableDefault();
  // Collect the current values - simple calibration
  vectorOffset.x = compass.a.x;
  vectorOffset.y = compass.a.y;
  vectorOffset.z = compass.a.z;
}

void updateVectorValues()
{
  vectorValues.x = compass.a.x - vectorOffset.x;
  vectorValues.y = compass.a.y - vectorOffset.y;
  vectorValues.z = compass.a.z - vectorOffset.z;
  collisionDetected = (abs(vectorValues.x) + abs(vectorValues.y) > COLLISION_THRESHOLD);
}

void loop() {  
  // Read the new accelerometer values
  compass.read();
  
  // Collect the new values
  updateVectorValues();
  
  // How to detect collision:
  if (collisionDetected)
  {
    // do something
  }
  
  // BIG WARNING: the accelerometers will pick up acceleration caused by us turning around etc
  // only check for collision when we know we're holding the same speed, and not turning.
  
}
