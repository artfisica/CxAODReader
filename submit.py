#!/usr/bin/env python

import sys
import subprocess as sp

process = ("Zvv", "Wev", "Wmuv", "Wtauv", "ttbarallhad", "ttbarnonall", 
           "dijet", "singletop", "gamma", "Zee", "Zmumu", "Ztautau", "signal")
processNum = (1, 2, 3, 4, 5, 6, 8, 10, 12, 19, 20, 21, 11)
#process = ("signal",)
#processNum = (11,)

for p, pn in zip(process,processNum):
    sp.call(("export process={0}; export processNum={1}; sbatch --mem 16000 "
             "-t 450 -x c-12-39,c-12-35 --export=process,processNum -J gen_{0} "
             "-o log_{0} run_batch.sh").format(p,pn), shell=True)
