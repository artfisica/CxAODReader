#!/bin/bash

echo `hostname`

source /afs/cern.ch/project/eos/installation/atlas/etc/setup.sh

source ../rcSetup.sh;

rm -f ${process};
hsg5frameworkReadCxAOD ${process} ${processNum};

cp -r ${process} ${directory}
