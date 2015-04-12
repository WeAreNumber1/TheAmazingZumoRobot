#Sam is our hero!
- He puts out all the fires.


Make the Sam program here.

#Explanation of BluetoothCommunication
Make sure this is inside setup():
```c++
  btSerial.begin(9600);
```

##How to use
- **void BT_update()**: Execute in loop. Does most of the work.
- **boolean BT_hasNewDestination()**: Use when Sam is waiting for a mission. Returns true if a destination has been received.
- **int BT_getDestination()**: Get the destination (the bot which is 'burning'). Returns 0 if no destination has been received.
- **boolean BT_shallReturn()**: Use when Sam is trying to put out the fire, to know if the fire has been put out. True: go home. False: stay.
- **void BT_sendHasReturned()**: Use when Sam has returned home. This way, the master-bot knows if you're ready for a new mission.

#Things to do:
1. Make Sam walk the line.
  1. ~~Follow line~~
  2. ~~Go to places [1,2,3,4]~~
    + Go back to start.
    + /Or maybe: Find quickest way to next robot in distress.
2. Communication with other Arduinos
  + Implement IRRecieve and go to given place.
