#!/bin/bash

if [[ "$1" == ""  ]] 
  then WORK_DIR=Workdir_13TeV
  else WORK_DIR=$1
fi

BASE_DIR=/gdata/atlas/anelson/MonoWZH/CxAODFramework
BIN_DIR=${BASE_DIR}/stack_hists
SAMPLE_DIR=${BASE_DIR}/${WORK_DIR}

mkdir -p stacked_plots
rm stacked_plots/*
cd stacked_plots

$BIN_DIR/stack_hists all_samples Sample \
$SAMPLE_DIR/hist-ttbarallhad.root ttbar_allhad 1 \
$SAMPLE_DIR/hist-ttbarnonall.root ttbar_nonall 1 \
$SAMPLE_DIR/hist-singletop.root singletop 1 \
$SAMPLE_DIR/hist-Wev.root Wev 1 \
$SAMPLE_DIR/hist-Wmuv.root Wmu 1 \
$SAMPLE_DIR/hist-Wtauv.root Wtauv 1 \
$SAMPLE_DIR/hist-Zee.root Zee 1 \
$SAMPLE_DIR/hist-Zmumu.root Zmumu 1 \
$SAMPLE_DIR/hist-Ztautau.root Ztautau 1 \
$SAMPLE_DIR/hist-Zvv.root Zvv 1 \
$SAMPLE_DIR/hist-gamma.root gamma 1 \
$SAMPLE_DIR/hist-dijet.root dijet 1 \
$SAMPLE_DIR/hist-monoWjjIsrConD5m50.root monoWjjIsrConD5m50x40 0 \

cd ..
