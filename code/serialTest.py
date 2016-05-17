import serial
import arff
import pickle
import struct

ser = serial.Serial('/dev/ttyACM0')
#input("Press Enter to Start...")
num = input("Sample number: ")
file = str(num)
file = file + ".p"
print(file)
dist = input("Distance: ")
angle = input("Angle: ")
startByte = b'a'
ser.write(startByte)
g = ser.read(4)
length = struct.unpack('i',g)
print("The length is ")
print(length[0])
q = []
for i in range(length[0]):
        print("Reading Index: ",i)
        g = ser.read(4)
        s = struct.unpack('f',g)
        q.append(s[0])
print(q)
pickle.dump(q,open(file,"wb"))
