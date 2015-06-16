# CxAODReader

General information on the CxAODFramework is on 
[CxAODFramework](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/CxAODFramework).

Follow these steps to set up your working area for DC14 samples. 

### Set up ATLAS environment and create working directory
`setupATLAS`

`mkdir CxAODFramework`

`cd CxAODFramework`

`svn co svn+ssh://svn.cern.ch/reps/atlasoff/PhysicsAnalysis/HiggsPhys/Run2/Hbb/CxAODFramework/FrameworkSub/tags/FrameworkSub-00-09-02 FrameworkSub`

`source FrameworkSub/bootstrap/setup-tag.sh`

### Replace CxAODReader directory with the mono-W/Z/H version

`rm -rI CxAODReader`

`git clone git@github.com:noslenwerdna/CxAODReader.git`

### Compile all packages

`rc build`
