#!/usr/bin/env python

from qm import QuineMcCluskey
import sys

qm = QuineMcCluskey()
#qm = QuineMcCluskey(use_xor = True) 

#Example:
#            ones = [2, 6, 10, 14]
#            dc = []
#
#            This will produce the ouput: ['--10']
#            This means x = b1 & ~b0, (bit1 AND NOT bit0)
# 
#
#Example:
#            ones = [1, 2, 5, 6, 9, 10, 13, 14]
#            dc = []
#
#            This will produce the ouput: ['--^^'].
#            In other words, x = b1 ^ b0, (bit1 XOR bit0).
#
#ones = [1, 2, 5, 6, 9, 10, 13, 14]

#ones = [1, 2, 9, 10]
#0001
#0010
#1001
#1010
#set(['-0^^'])

#ones = [9, 10, 13, 14]
#1001
#1010
#1101
#1110
#set(['1-^^'])

#ones = [5,6,9, 10]
#0101
#0110
#1001
#1010
#set(['^^01', '^^10'])

#ones = [1,2,4,8,7,11,13,14]   # odd number of ones
#0001
#0010
#0100
#1000
#0111
#1011
#1101
#1110
#set(['^^^^'])

ones = [0,3,5,6]
#000
#011
#101
#110

#print(qm.simplify(ones, []))

for line in sys.stdin:
    print(qm.simplify([int(i) for i in line.split()], []))
