#include <EEPROM.h>

byte IDENTITY;  // position in the world
byte IS_MASTER; // 1 if this is master-bot, 0 if not.

void setup()
{
  Serial.begin(9600);
  Serial.println("------------------------------------------------------");
  Serial.println("Please make sure you're using 'Newline'.");
  delay(1000);
  Serial.println("Current status (255 = not set):");
  // Read data from memory
  IDENTITY = EEPROM.read(0);
  IS_MASTER = EEPROM.read(sizeof(byte));  // the byte after IDENTITY
  Serial.print("Number: ");
  Serial.println(IDENTITY);
  Serial.print("Is master-bot: ");
  Serial.println(IS_MASTER);
  
  Serial.println();
  Serial.println("Hit [enter] to edit.");
  
  // Keep us occupied while waiting for response
  while (Serial.available() == 0)
  { delay(50);
  } // clean up the buffer
  while (Serial.read() != '\n') {}
  
  Serial.println("New number? [1-4]");
  int newIdentity = 0; // 0 = not valid
  
  do {
    newIdentity = Serial.parseInt();
    if (newIdentity != 0 && (newIdentity < 1 || newIdentity > 4))
    {
      Serial.println("The identity/number must be 1, 2, 3 or 4.");
      newIdentity = 0;
    }
  } while (newIdentity == 0);
  Serial.read(); // throw away newline
  
  
  Serial.println("Master-bot? [y or n]");
  boolean validAnswer = false;
  boolean newIsMaster;
  
  while (!validAnswer)
  {
    // keep us occupied - again - while waiting for answer
    while (Serial.available() == 0) {
      delay(50);
    }
    
    char answer = Serial.read();
    if (answer == 'y')
    {
      validAnswer = true;
      newIsMaster = true;
    } else if (answer == 'n')
    {
      validAnswer = true;
      newIsMaster = false;
    } else {
      Serial.println("Please type either 'y' (yes) or 'n' (no).");
    }
  }
      
  Serial.println();
  delay(500);
  Serial.println("Please confirm that this is correct:");
  delay(1000);
  Serial.print("New identity/number: ");
  Serial.println(newIdentity);
  delay(1000);
  Serial.print("Is master: ");
  Serial.println(newIsMaster); // true -> 1, false -> 0
  
  delay(1000);
  // empty buffer
  while (Serial.available() > 0)
  {
    Serial.read();
  }
  
  
  Serial.println("Press [enter] to write these changes to memory.");
  Serial.println("Press the red reset-button if there's something wrong.");
  
  // continue once data comes
  while (Serial.available() == 0)
  { delay(50);
  }
  
  // Write changes to disk
  if (newIdentity != IDENTITY)
  {
    EEPROM.write(0, newIdentity);
  }
  
  if (newIsMaster != IS_MASTER)
  {
    EEPROM.write(sizeof(byte), newIsMaster);
  }
  
  Serial.println("This helper-bot's identity and master-bot-state has been updated!");
  Serial.println("Press the red reset-button and check if everything's okay.");
  Serial.println();
  Serial.println();
  Serial.println();
  
}

void loop()
{
  delay(1000);
}
