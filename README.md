# CxAODReader

General information on the CxAODFramework is on 
[CxAODFramework](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/CxAODFramework).

Follow these steps to set up your working area for DC14 samples. 

### Setting up Github

If you already have a github account skip this step. 

* Install git on your computer

* Sign up on github.com

* Generate an ssh key following this [guide](https://help.github.com/articles/generating-ssh-keys/).

* Source this script when you start a new session: `setupGit.sh`

### Set up ATLAS environment, create working directory, and check out packages

`setupATLAS`

`mkdir CxAODFramework`

`cd CxAODFramework`

`svn co svn+ssh://svn.cern.ch/reps/atlasoff/PhysicsAnalysis/HiggsPhys/Run2/Hbb/CxAODFramework/FrameworkSub/tags/FrameworkSub-00-09-02 FrameworkSub`

`source FrameworkSub/bootstrap/setup-tag.sh`

### Replace CxAODReader directory with the mono-W/Z/H version

`rm -rI CxAODReader`

`git clone git@github.com:noslenwerdna/CxAODReader.git`

`cd CxAODReader`

`git checkout v0.4`

`cd ..`

### Replace files in other directories for compatibility with CxAODReader

Certain configuration files need to be updated:

`cp CxAODReader/XSections_13TeV.txt FrameworkSub/data`

`cp CxAODReader/yields.0lep.13TeV.txt FrameworkSub/data`

`cp CxAODReader/framework-read.cfg FrameworkExe/data`

`hsg5frameworkReadCxAOD.cxx` also needs to be updated 
with the names of our background and signal samples.
One line of this file you will have to change by hand:

`std::string afsdir("/gdata/atlas/anelson/CxAOD.SubStructure/");`

In the future this will be added to the configuration settings.

`cp CxAODReader/hsg5frameworkReadCxAOD.cxx FrameworkExe/util`

### Compile all packages

`rc build`

### Run over a sample

Run over the Zvv samples like:

`hsg5frameworkReadCxAOD Zvv 1`

### Cutflow

After running over the Zvv sample, verify your results by comparing your 
cutflow histogram with the cutflow below.

| Zvv cutflow |  Yield  |
|-------------|---------|
| All         | 1333223 |
| 250 MET     |   45220 |
| 1 fatjet    |   18055 |
| no lepton   |   17478 |
| jet veto    |   13041 |
| delta Phi   |   11133 |
| 500 MET     |     845 |
| mJet        |     223 |

The full cutflow is on a 
[google document](https://docs.google.com/spreadsheets/d/13CUvy5W5UqGjZphaJOqcg9gakGUXwZdkHscXt-d5nww/edit#gid=1605348284), 
please fill it in when you have 
processed all the samples (see next section).

### Process full samples

We are using the official CxAOD_00-09-01 samples on EOS. You can submit 
jobs to process all of them on lxbatch with the script `submit_LSF.py`

### Plotting

The plotting code is in stack_hists. Documentation not written yet.

### Additional scripts

I included a few extra scripts that you might find useful

* `submit.py` and `run_batch.sh` will submit all signal and background samples to the 
batch system (SLURM syntax).

* `getEventYields.py` will recalculate the `yields.0lep.13TeV.txt` configuration file if 
your background or signal samples change.
