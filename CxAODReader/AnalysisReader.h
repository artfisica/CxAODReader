#ifndef CxAODReader_AnalysisReader_H
#define CxAODReader_AnalysisReader_H

#include <vector>

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#ifndef __MAKECINT__
#include "CxAODTools/CommonProperties.h"
#include "CxAODTools/EventSelection.h"
#include "CxAODTools/OverlapRemoval.h"
#include "CxAODTools/XSectionProvider.h"
#include "CxAODTools/sumOfWeightsProvider.h"
#include "CxAODTools/ReturnCheck.h"
#include "CxAODReader/physicsObjects.h"
#endif // not __MAKECINT__

#include "CxAODTools/ConfigStore.h"

#include <TH1D.h>
#include <TLorentzVector.h>

#include <string>
#include <map>

#include "CxAODReader/ObjectReader.h"

using namespace std;

class AnalysisReader : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
public:
  // float cutValue;

  xAOD::TEvent *m_event;  //!

  enum Type {oneLep, twoLep, zvv, top, qcd, monoWZH};

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
  // Tree *myTree; //!
  // TH1 *myHist; //!

  // this is a standard constructor
  AnalysisReader ();

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  void SetAnalysisType(int analysistype) {m_analysisType=analysistype;}
  int GetAnalysisType() {return m_analysisType;}

  void SetCOMEnergy(TString com) {m_comEnergy=com;}

  virtual EL::StatusCode initializeEvent();
  virtual EL::StatusCode initializeReader();
  virtual EL::StatusCode initializeSelection();
  virtual EL::StatusCode initializeIsMC();
  virtual EL::StatusCode initializeXSections();
  virtual EL::StatusCode initializeSumOfWeights();

  
  virtual EL::StatusCode setObjectsForOR(const xAOD::ElectronContainer* electrons,
                                         const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons,
                                         const xAOD::TauJetContainer* taus,
                                         const xAOD::JetContainer* jets);

  EL::StatusCode getLumiWeight(double & weight);

  void setConfig(std::string configPath) { m_configPath = configPath; }

  float m_luminosity; //! for rescaling the MC to a particular value of the luminosity (default is 1 pb-1)

  bool m_debug; //!
  bool m_passThroughOR; //!
  bool m_applyEventPreSelection; //!

  ConfigStore * m_config; //!
  std::string m_configPath;

 private:
  EL::StatusCode do_Analysis ();
  void fillCutflow(std::string binName);

  int m_analysisType;
  int m_eventCounter; //!
  bool m_isMC; //!
  double m_weight; //!
  double m_sumOfWeights; //!  correct for the luminosity of the MC file
  TString m_comEnergy; 
  bool m_isSherpaVJets; // is Sherpa file
  bool m_isSherpaPt0VJets; //! is present sample SherpaVJets Pt0
  float m_SherpaPt0VJetsCut; //! cut Pt0 events overlapping with other PtV slices

  //monoWZH hists
  map<string,TH1D*> m_histMap; //!

  // general histograms
  TH1F* m_hist_VPtTruth; //!

  std::vector<ObjectReaderBase*> m_objectReader; //!
  ObjectReader<xAOD::EventInfo>* m_eventInfoReader; //!
  ObjectReader<xAOD::MissingETContainer>* m_METReader; //!
  ObjectReader<xAOD::ElectronContainer>* m_electronReader; //!
  ObjectReader<xAOD::PhotonContainer>* m_photonReader; //!
  ObjectReader<xAOD::MuonContainer>* m_muonReader; //!
  ObjectReader<xAOD::TauJetContainer>* m_tauReader; //!
  ObjectReader<xAOD::JetContainer>* m_jetReader; //!
  ObjectReader<xAOD::JetContainer>* m_fatJetReader; //!
  ObjectReader<xAOD::TruthParticleContainer>* m_truthParticleReader; //!

  EL::StatusCode OverlapPassThrough(const xAOD::ElectronContainer* electrons,
                                    const xAOD::PhotonContainer* photons,
                                    const xAOD::MuonContainer* muons,
                                    const xAOD::TauJetContainer* taus,
                                    const xAOD::JetContainer* jets);

#ifndef __MAKECINT__
  template <class containerType>
  ObjectReader< containerType >* registerReader(std::string name);

  const xAOD::EventInfo*          m_eventInfo; //!
  const xAOD::MissingET *         m_met;       //!
  const xAOD::MissingETContainer* m_metCont;   //!
  const xAOD::ElectronContainer*  m_electrons; //!
  const xAOD::PhotonContainer*    m_photons;   //!
  const xAOD::MuonContainer*      m_muons;     //!
  const xAOD::TauJetContainer*    m_taus;      //!
  const xAOD::JetContainer*       m_jets;      //!
  const xAOD::JetContainer*       m_fatJets;   //!
  const xAOD::TruthParticleContainer* m_truthParts; //!

  OverlapRemoval* m_overlapRemoval; //!
  XSectionProvider* m_xSectionProvider; //!
  sumOfWeightsProvider* m_sumOfWeightsProvider; //!

  EL::StatusCode fill_oneLep (physicsObjects &po); 
  EL::StatusCode fill_twoLep (physicsObjects &po); 
  EL::StatusCode fill_zvv (physicsObjects &po); 
  EL::StatusCode fill_top (physicsObjects &po, TLorentzVector &fjet); 
  EL::StatusCode fill_qcd (physicsObjects &po); 
  EL::StatusCode fill_monoWZ (physicsObjects &po, TLorentzVector &fjet);
#endif // not __MAKECINT_

  // this is needed to distribute the algorithm to the workers
  ClassDef(AnalysisReader, 1);
};

#ifndef __MAKECINT__
#include "CxAODReader/AnalysisReader.icc"
#endif // not __MAKECINT__

#endif
