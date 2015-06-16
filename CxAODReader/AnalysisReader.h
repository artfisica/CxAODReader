#ifndef CxAODReader_AnalysisReader_H
#define CxAODReader_AnalysisReader_H

#include <EventLoop/Algorithm.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

#ifndef __MAKECINT__
#include "CxAODTools/SuperDecorator.h"
#include "CxAODTools/OverlapRemoval.h"
#include "CxAODTools/XSectionProvider.h"
#include "CxAODTools/sumOfWeightsProvider.h"
#include "CxAODReader/physicsObjects.h"
#endif // not __MAKECINT__

#include <TH1F.h>
#include <TLorentzVector.h>

#include <string>
#include <vector>
#include <map>

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

 private:
  EL::StatusCode do_Analysis ();
  void fillCutflow(std::string binName);

  int m_analysisType;
  int m_eventCounter; //!
  bool m_isMC; //!
  double m_weight; //!
  double m_MCweight; //!
  double m_sumOfWeights; //!  correct for the luminosity of the MC file
  TString m_comEnergy; 
  bool m_isSherpaVJets; // is Sherpa file
  bool m_isSherpaPt0VJets; //! is present sample SherpaVJets Pt0
  float m_SherpaPt0VJetsCut; //! cut Pt0 events overlapping with other PtV slices

  //monoWZH hists
  map<string,TH1D*> m_histMap; //!

  // general histograms
  TH1F* m_hist_VPtTruth; //!

#ifndef __MAKECINT__
  SuperDecorator m_superDecorator;//!
  OverlapRemoval* m_overlapRemoval; //!
  XSectionProvider* m_xSection; //!
  sumOfWeightsProvider* m_sumOfWeights_fix; //!

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

#endif
