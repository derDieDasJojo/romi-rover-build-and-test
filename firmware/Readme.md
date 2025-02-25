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
- ```#M[0,0,0,0]:xxxx``` move((dt,X,Y,Z): move for dt ms to (X,Y,Z)
- ```#V[1,1,1]:xxxx``` move at speed(1,1,1)
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

now let us check the what the current position is:
send: ```#P:xxxx```
response: ```#P[0,0,0,0]:00af```
this means the current position is (0,0,0,0)

To enable the stepper motors send: ```#E:xxxx```
response: #E[0]:0033

now let us move the Z-axis by 1cm to (0,0,1000) with a speed of 500mm/sec: ```#m[500,0,0,1000]:xxx```
now let us move the Z-axis back with 1000mm/sec: ```#m[1000,0,0,0]:xxx```

## Troubleshooting

possible error codes are:

                // Errors generated by the EnvelopeParser   
                kEnvelopeTooLong = -1,
                kEnvelopeInvalidId = -2,
                kEnvelopeInvalidCrc = -3,
                kEnvelopeCrcMismatch = -4,
                kEnvelopeExpectedEnd = -5,
                kEnvelopeMissingMetadata = -6,
                kEnvelopeInvalidDummyMetadata = -7,
        
                // Errors generated by the MessageParser 
                kUnexpectedChar = -8,
                kVectorTooLong = -9,
                kValueOutOfRange = -10,
                kStringTooLong = -11,
                kInvalidString = -12,
                kTooManyStrings = -13,
                kInvalidOpcode = -14,

                // Errors generated by RomiSerial 
                kDuplicate = -15,
                kUnknownOpcode = -16,
                kBadNumberOfArguments = -17,
                kMissingString = -18,
                kBadString = -19,
                kBadHandler = -20,

                // Errors generated by RomiSerialClient 
                kClientInvalidOpcode = -21,
                kClientTooLong = -22,
                kEmptyResponse = -23,
                kEmptyRequest = -24,
                kConnectionTimeout = -25,        
                kInvalidJson = -26,
                kInvalidResponse = -27,
                kInvalidErrorResponse = -28,
        
                kLastError = -29

# References
- https://github.com/romi/libromi/tree/arduino_libraries/RomiSerial/docs
- https://github.com/romi/romi-rover-build-and-test/blob/ci_dev/firmware/Oquam/Oquam.ino
