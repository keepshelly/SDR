import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

path=Path(__file__).resolve().parent.parent/"build"/"txdata.pcm"
data=np.fromfile(path,dtype="<i2")

I=data[0::2]
Q=data[1::2]
n=np.arange(len(I))

plt.plot(n,I,label="I")
plt.plot(n,Q,label="Q")
plt.xlabel("отсчёт")
plt.ylabel("амплитуда")
plt.grid()
plt.legend()
plt.show()