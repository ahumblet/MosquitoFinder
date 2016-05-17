import serial
import arff
import pickle
import struct

ser = serial.Serial('/dev/ttyACM0')
input("Press Enter to Start...")
startByte = b'a'

ser.write(startByte)
g = ser.read(4)
length = struct.unpack('i',g)
print("The length is ")
print(length)

for i in range(length[0]):
        print("Reading Index: ",i)
        g = ser.read(4)
        s = struct.unpack('f',g)
        q = s[0]
        print(q)
