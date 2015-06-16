# CxAODReader

General information on the CxAODFramework is on 
https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/CxAODFramework.

For DC14 samples setup your working directory like this:

# setup ATLAS environment and make working directory
setupATLAS
mkdir CxAODFramework
cd CxAODFramework/

svn co svn+ssh://svn.cern.ch/reps/atlasoff/PhysicsAnalysis/HiggsPhys/Run2/Hbb/CxAODFramework/FrameworkSub/tags/FrameworkSub-00-09-02 FrameworkSub
source FrameworkSub/bootstrap/setup-tag.sh

# replace the CxAODReader directory with the mono-W/Z/H  version
rm -r CxAODReader
git clone git@github.com:noslenwerdna/CxAODReader.git

# compile all packages
rc build
