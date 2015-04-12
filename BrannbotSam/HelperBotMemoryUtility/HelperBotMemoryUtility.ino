#include <EEPROM.h>

byte IDENTITY;
byte IS_MASTER;

void setup()
{
  Serial.begin(9600);
  Serial.println("------------------------------------------------------");
  Serial.println("Please make sure you're using 'Newline' (\\n)");
  Serial.println("This is the status of this helper-bot (255 = not set):");
  IDENTITY = EEPROM.read(0);
  IS_MASTER = EEPROM.read(sizeof(byte));
  Serial.print("Number:\t");
  Serial.println(IDENTITY);
  Serial.print("Is master-bot:\t");
  Serial.println(IS_MASTER);
  
  Serial.println();
  Serial.println("Hit enter if you want to change anything.");
  
  while (Serial.available() == 0)
  { delay(50);
  }
  while (Serial.read() != '\n') {}
  
  Serial.println("Which identity/number should this helper-bot be? [1-4]");
  int newIdentity = 0;
  
  do {
    newIdentity = Serial.parseInt();
    if (newIdentity != 0 && (newIdentity < 1 || newIdentity > 4))
    {
      Serial.println("The identity/number must be 1, 2, 3 or 4.");
      newIdentity = 0;
    }
  } while (newIdentity == 0);
  Serial.read(); // throw away newline
  
  
  Serial.println("Shall this be the master-bot (the one communicating with Sam)? y or n");
  boolean validAnswer = false;
  boolean newIsMaster;
  
  while (!validAnswer)
  {
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
  Serial.println("Please confirm the following information:");
  delay(1000);
  Serial.print("New identity/number:\t");
  Serial.println(newIdentity);
  delay(1000);
  Serial.print("New master:\t");
  Serial.println(newIsMaster);
  
  delay(1000);
  while (Serial.available() > 0)
  {
    Serial.read();
  }
  
  
  Serial.println("Press enter to write these changes to memory.");
  Serial.println("Press the red reset-button if there's something wrong.");
  
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
  
}
