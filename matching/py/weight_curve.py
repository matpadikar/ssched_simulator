import sys
import matplotlib.pyplot as plt

with open(sys.argv[1], "r") as fin:
    scales = [float(x) for x in fin.readline().split()]
    #batch2 = [float(x) for x in fin.readline().split()]
    batch1 = [float(x) for x in fin.readline().split()]
    serena = [float(x) for x in fin.readline().split()]
    mwm = [float(x) for x in fin.readline().split()]

#plt.plot(scales, batch2, 'o-b', label="SB-QPS, 2 rounds per frame")
plt.plot(scales, batch1, 'x-m', label="SB-QPS, 1 round per frame")
plt.plot(scales, serena, 'h-g', label="r(QPS-SERENA)")
plt.plot(scales, mwm, '-k', label= "Maximum Weighted Matching")
plt.legend()
plt.xlabel("frame size")
if sys.argv[1][:6] == "score":
    plt.ylabel("weight")
elif sys.argv[1][:4] == "time":
    plt.ylabel("time per matching (s)")
plt.show()
