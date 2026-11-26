import numpy as np
import matplotlib.pyplot as plt

import sys

mod="bpsk"
if len(sys.argv)>1:
    mod=sys.argv[1]
name="../build/rxdata_"+mod+".pcm"

data=np.fromfile(name,dtype="<i2")

real=data[0::2].astype(float)
imag=data[1::2].astype(float)
count=np.arange(1,len(real)+1)
iq=real+1j*imag

start=len(real)//2
n=20000

fig,ax=plt.subplots(1,2,figsize=(14,6))

ax[0].plot(count,imag,color="red")
ax[0].plot(count,real,color="blue")
ax[0].plot(count,np.abs(iq))
ax[0].set_xlabel("номер сэмпла")
ax[0].grid()

ax[1].plot(real[start:start+n],imag[start:start+n],".",markersize=1)
ax[1].set_xlabel("i")
ax[1].set_ylabel("q")
ax[1].axis("equal")
ax[1].grid()

plt.show()
