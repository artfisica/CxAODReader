#!/bin/bash

#SBATCH --partition=atlas_all
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=10

echo `hostname`
echo ${SLURM_SUBMITDIR} ${SLURM_JOBID} ${process} ${processNum};

cd $SLURM_SUBMIT_DIR;

#export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase;
#source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh;
#localSetupROOT;

source rcSetup.sh;

#SCRATCHDIR=/scratch/anelson/$SLURM_JOBID;

rm -f ${process};
hsg5frameworkReadCxAOD ${process} ${processNum};
