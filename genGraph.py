#!/usr/bin/env python3

import sys
import getopt
from numpy.random import random_integers

def getint(a):
    try:
        tmp = int(a)
    except ValueError:
        print("Arguments must be integers")
        sys.exit(2)
    return tmp


def genGraph(n, low, high, filename):
    f = open(filename, 'w')

    f.write('%d\n' % n)         # write the number of nodes
    
    for i in range(n):
        for j in range(n):
            f.write('%d %d %d\n' % (i, j, random_integers(low, high)))

    f.close()


def main():
    try:
        opts, args = getopt.getopt(sys.argv[1: ], 'n:l:h:f:')
    except getopt.GetoptError as err:
        print(err)
        sys.exit(2)

    low = 1
    high = 100
    n = 100
    filename = 'graph.in'

    for o, a in opts:
        if o == '-n':
            n = getint(a)
        elif o == '-l':
            low = getint(a)
        elif o == '-h':
            high = getint(a)
        elif o == '-f':
            filename = a
        else:
            print("Unknown options")
            sys.exit(2)

    genGraph(n, low, high, filename)


if __name__ == '__main__':
    main()
