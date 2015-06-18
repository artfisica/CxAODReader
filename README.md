# CxAODReader

General information on the CxAODFramework is on 
[CxAODFramework](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/CxAODFramework).

Follow these steps to set up your working area for DC14 samples. 

### Setting up Github

If you already have a github account skip this step. 

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

### Download the samples

Download my background and signal samples from the grid.

`user.anelson.mc14_13TeV.*.CAOD_HIGG5D1.v09-03-02_outputLabel*/`

They will need to be put in a directory (above it was 
`/gdata/atlas/anelson/CxAOD.SubStructure/`), and put the samples 
in directories according to their type. For example, all the 
Znunu samples go in a directory called Zvv.

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

### Plotting

The plotting code is in stack_hists. Documentation not written yet.

### Additional scripts

I included a few extra scripts that you might find useful

* `submit.py` and `run_batch.sh` will submit all signal and background samples to the 
batch system (SLURM syntax).

* `getEventYields.py` will recalculate the `yields.0lep.13TeV.txt` configuration file if 
your background or signal samples change.
