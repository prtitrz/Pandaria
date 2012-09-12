#!/usr/bin/env python
import os
import numpy as np
import pylab as pl

def main():
    n = [0]
    i = 0
    j = 1
    print("Generate the IOPS_count...")
    with open("/home/rz/Trace/WebSearch2.spc") as f:
        for line in f:
            num, offsize, length, op, sec = line.split(',')
            s = float(sec)
            if (s < j):
                n[i] += 1
            else:
                i += 1
                j += 1
                n.append(1)
    with open("IOPS_count", "r+") as g:
        for k in range(len(n)):
            s = '{0} {1}\n'.format(k, n[k])
            g.write(s)
    print("IOPS_count is generated...")
    print("Painting...")
    data = np.loadtxt('IOPS_count')
    pl.plot(data[:,0], data[:,1])
    pl.show()

if __name__ == '__main__':
    main()
