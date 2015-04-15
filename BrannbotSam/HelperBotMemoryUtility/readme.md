#HelperBotMemoryUtility

##What is it?
All Arduinos have a memory which isn't cleared upon resetting, programming, power-off...
The helper bot program will use this memory to figure out which position it has, and if it should also be a master-bot.
This program lets you set the position and master-bot-status.

##How to use
1. Upload this program to the helper-bot in question.
2. Open the serial monitor.
3. Make sure you've chosen "Newline", not "No line ending".
4. Follow the instructions on-screen.

##Explanation of number, master-bot

### Number
- Position in the "world" (as indicated on the board)
- Can be 1, 2, 3 or 4.
- Only one robot per number.

### Master-bot
- Receives IR-messages from the other helper-bots.
- Takes care of communicating this info (where there's fire) with Sam through BlueTooth.
- Precisely one of the helper bots should be master-bot.
- 1: is master bot. 0: is not master bot.
