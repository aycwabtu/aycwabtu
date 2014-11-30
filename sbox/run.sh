#!/bin/bash
gcc -o csablocksbox2bool csablocksbox2bool.c || exit 1
./csablocksbox2bool | python qmpipe.py | perl qm2bs.pl >qm.c
