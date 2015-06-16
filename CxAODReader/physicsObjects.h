#ifndef physicsObjects_H
#define physicsObjects_H

#include <vector>
#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "TLorentzVector.h"

using namespace std;

class physicsObjects {
  public:
  vector<const xAOD::Jet*> fatsigJets;
  vector<const xAOD::Electron*> looseElectrons;
  vector<const xAOD::Muon*> looseMuons;
  vector<const xAOD::Electron*> tightElectrons;
  vector<const xAOD::Muon*> tightMuons;
  vector<const xAOD::Jet*> sigJets;
  vector<const xAOD::Jet*> metJets;
  vector<const xAOD::Jet*> subJets;
  vector<const xAOD::Jet*> subBJets;
  TLorentzVector metVec;
};

#endif
