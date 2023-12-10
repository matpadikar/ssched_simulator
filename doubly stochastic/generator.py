import numpy as np
import sys

sz = int(sys.argv[2])
num = int(sys.argv[3])

def generate(sz):
    x = np.random.random((sz,sz))

    rsum = np.full(sz, 2)
    csum = np.full(sz, 2)
    # i = 0

    while (np.any(rsum > 1 + 1e-9)) | (np.any(csum > 1 + 1e-9)) | (np.any(rsum < 1 - 1e-9)) | (np.any(csum < 1 - 1e-9)):
        x /= x.sum(0)
        x = x / x.sum(1)[:, np.newaxis] # change to column vector
        rsum = x.sum(1)
        csum = x.sum(0)

        # i += 1
        #if (i % 100 == 99):
        #    print(rsum, csum)
    return x


with open(sys.argv[1], "w") as f:
    f.write("{0}\t{1}\n".format(num, sz))
    for i in range(num):
        x = generate(sz)
        for row in x:
            for item in row:
                f.write(str(item) + "\t")
            f.write("\n")

