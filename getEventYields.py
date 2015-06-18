#!/usr/bin/env python

import sys, glob, re
import ROOT as r

channels = ["dijet","gamma","singletop","ttbarallhad","ttbarnonall","Wev",
    "Wmuv","Wtauv","Zee","Zmumu","Ztautau","Zvv","monoWjjIsrConD5m1300",
    "monoWjjIsrConD5m50","monoWjjIsrDesD1m1300","monoWjjIsrDesD1m50",
    "monoWjjIsrDesD5m1300","monoWjjIsrDesD5m50","monoWjjIsrDesD9m1300",
    "monoWjjIsrDesD9m50","monoWjjSimConSsdMed100m50",
    "monoWjjSimConSsdMed1500m1300","monoWjjSimConSsdMed1500m50",
    "monoWjjSimConSvdMed100m50","monoWjjSimConSvdMed1500m1300",
    "monoWjjSimConSvdMed1500m50","monoWjjSimDesSsdMed100m50",
    "monoWjjSimDesSsdMed1500m1300","monoWjjSimDesSsdMed1500m50",
    "monoWjjSimDesSvdMed100m50","monoWjjSimDesSvdMed1500m1300",
    "monoWjjSimDesSvdMed1500m50","monoWjjWwxxm1300","monoWjjWwxxm50",
    "monoZjjIsrD1m1300","monoZjjIsrD1m50","monoZjjIsrD5m1300","monoZjjIsrD5m50",
    "monoZjjIsrD9m1300","monoZjjIsrD9m50","monoZjjSimSsdMed100m50",
    "monoZjjSimSsdMed1500m1300","monoZjjSimSsdMed1500m50",
    "monoZjjSimSvdMed100m50","monoZjjSimSvdMed1500m1300",
    "monoZjjSimSvdMed1500m50","monoZjjZzxxm1300","monoZjjZzxxm50",]

for channel in channels:
  sampleNums = list()
  for d in glob.glob("{0}/user.anelson.mc14_13TeV.*".format(channel)):
    d = re.sub(r".*user.anelson.mc14_13TeV.(......).*",r"\1",d)
    sampleNums.append(d)
  sampleNums = list(set(sampleNums))
  #print "sampleIDs:", channel, " ", sampleNums
  
  for sampleNum in sampleNums:
    #print glob.glob("{0}/user.anelson.mc14_13TeV.{1}.*".format(channel,sampleNum))
    hist = r.TH1D()
    for ctr,d in enumerate(glob.glob("{0}/user.anelson.mc14_13TeV.{1}.*/*.root*".format(channel,sampleNum))):
      f = r.TFile(d)
      if ctr==0:
        f.GetObject("MetaData_EventCount",hist)
        hist.SetDirectory(0)
      else:
        histTemp = r.TH1D()
        f.GetObject("MetaData_EventCount",histTemp)
        hist.Add(histTemp)
      f.Close()

    print "{1}\t\t{2}\t\t{3}".format(channel, sampleNum, hist.GetBinContent(1), hist.GetBinContent(4))
