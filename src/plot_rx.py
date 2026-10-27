import numpy as np
import matplotlib.pyplot as plt

name="../build/rxdata.pcm"

data=np.fromfile(name,dtype="<i2")

real=data[0::2].astype(float)
imag=data[1::2].astype(float)
count=np.arange(1,len(real)+1)
iq=real+1j*imag

plt.figure(1)
plt.plot(count,imag,color="red")
plt.plot(count,real,color="blue")
plt.plot(count,np.abs(iq))
plt.show()