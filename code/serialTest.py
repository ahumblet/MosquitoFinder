import serial
import arff
import pickle
import struct
NSAMP = 10
ser = serial.Serial('/dev/ttyACM0')
#input("Press Enter to Start...")
num = input("Start number: ")
num = int(num) - 1
dist = input("Distance: ")
angle = input("Angle: ")
for i in range(NSAMP):
        num = num + 1
        file = str(num)
        file = file + ".p"
        print(file)
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
        q.append(float(dist))
        q.append(float(angle))
        pickle.dump(q,open(file,"wb"))
