import arff
import pickle

attributes = []
nFFT = 64
baseStr = "FFT_"
for i in range(nFFT):
    name = baseStr + str(i)
    attributes.append((name,u'REAL'))
attributes.append(("Ang",u'REAL'))
#print(attributes)
relation = "Dist_FFT"

dataF = []
samples = 120
for i in range(samples):
    filename = str(i+1) + ".p"
    f = open(filename,"rb")
    p = pickle.load(f)
    f.close()
    p.pop(-2)
    dataF.append(p)

data = {u'attributes': attributes, u'data': dataF, u'description': u'', u'relation': relation}

f = open("myData2.arff","w")
f.write(arff.dumps(data))
f.close()
