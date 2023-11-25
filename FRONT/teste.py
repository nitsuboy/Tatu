import numpy as np

paredes = np.array([(0,1),(1,1),(2,1)])

if (0,0) in list(paredes[:,0]):
    print(True)