#!/usr/bin/env python

import sys
import subprocess as sp

#process = ("Zvv", "Wev", "Wmuv", "Wtauv", "ttbarallhad", "ttbarnonall", 
#           "dijet", "singletop", "gamma", "Zee", "Zmumu", "Ztautau", "signal")
#processNum = (1, 2, 3, 4, 5, 6, 8, 10, 12, 19, 20, 21, 11)
process = ("ZnunuL",)
processNum = (24,)

for p, pn in zip(process,processNum):
    sp.call(("export process={0}; export processNum={1}; bsub -M 16000 "
             "-q 8nh -W 450 --export=process,processNum "
             "run_batch.sh").format(p,pn), shell=True)
