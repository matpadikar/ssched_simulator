import sys
import random
import numpy as np

experiments = 100
size = 1000
filename = "uniform1000.txt"
low = 1000
high = 10000

rng = np.random.default_rng()
random_entries = rng.integers(low, high, [experiments, size, size], dtype=np.int32)
with open(filename, "w") as fout:
    fout.write(str(experiments) + "\n")
    for m in range(experiments):
        fout.write("{}\n".format(size))
        np.savetxt(fout, random_entries[m], fmt="%d", delimiter="\t")
