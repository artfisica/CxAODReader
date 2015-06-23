#!/usr/bin/env python

import os,sys
import subprocess as sp

process = ("ZnunuB","ZnunuC","ZnunuL","ZeeB","ZeeC","ZeeL",
           "ZmumuB","ZmumuC","ZmumuL","ZtautauB","ZtautauC","ZtautauL",
           "WenuB","WenuC","WenuL","WmunuB","WmunuC","WmunuL",
           "WtaunuB","WtaunuC","WtaunuL","dijet","ttbar",
           "singletop_Wt","singletop_s","singletop_t")
processNum = (22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,
              43,44,45,46,47)

if os.getcwd().split("/")[-1]=="CxAODReader":
    print "Run submit script from top level directory (CxAODFramework)"
    sys.exit(0)

if not os.access("run_batch_LSF.sh",os.X_OK):
    print "Shell script must be executable. Run:"
    print "chmod 755 run_batch_LSF.sh"
    sys.exit(0)

for p, pn in zip(process,processNum):
    sp.call(("export process={0}; export processNum={1}; export directory={2}; "
             "bsub -M 16000 -q 1nh -W 450  "
             "run_batch_LSF.sh").format(p,pn,os.getcwd()), shell=True)
