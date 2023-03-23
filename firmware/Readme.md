# Setup

flash it to your adruino uno

# Test it

connect to arduino Serial Monitor
Unless specified otherwise, the serial connections should be set to: 115200 baudrate, 8 data bits, no parity, and 1 stop bit (115200 8N1).

send your first command
```#?:xxxx```
as a resonse you should get:
```
#?[0,"Oquam","0.1","Mar 23 2023 13-52-39"]:007d
```


the following commands are available:
- ```#m[0,0,0,0]:xxxx``` move to (0,0,0,0)
- ```#M[0,0,0,0]:xxxx``` move at (0,0,0,0)
- ```#V[0,0,0,0]:xxxx``` move at (0,0,0,0)
- ```#p:xxxx``` pause
- ```#c:xxxx``` continue
- ```#r:xxxx``` reset
- ```#z:xxxx``` zero
- ```#P:xxxx``` send position
- ```#I:xxxx``` idle
- ```#H:xxxx``` homing
- ```#h[0,0,0,0]:xxxx``` set homing (0,0,0,0)
- ```#E[1]:xxxx``` enable
- ```#E[0]:xxxx``` disable
- ```#S[1]:xxxx``` spindle on
- ```#S[0]:xxxx``` spindle off
- ```#T[1]:xxxx``` test on
- ```#T[0]:xxxx``` test off
- ```#?:xxxx``` info




# References
- https://github.com/romi/libromi/tree/arduino_libraries/RomiSerial/docs
- https://github.com/romi/romi-rover-build-and-test/blob/ci_dev/firmware/Oquam/Oquam.ino
