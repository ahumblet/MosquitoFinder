import arff
import pickle

attributes = []
nFFT = 64
baseStr = "FFT_"
for i in range(nFFT):
    name = baseStr + str(i)
    attributes.append(name)
attributes.append("Dist")
relation = "Dist_FFT"

data = []
samples = 120
for i in range(samples):
    filename = str(i+1) + ".p"
    f = open(filename,"rb")
    p = pickle.load(f)
    f.close()
    p.pop()
    data.append(p)
print(data)
