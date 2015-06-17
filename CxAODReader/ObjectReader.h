// Dear emacs, this is -*-c++-*-
#ifndef CxAODReader_ObjectReader_H
#define CxAODReader_ObjectReader_H

#include "TString.h"
#include "TTree.h"

#include "CxAODReader/ObjectReaderBase.h"

#ifndef __MAKECINT__

#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#ifndef XAODMUON_MUONCONTAINER_H
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#endif
#include "xAODMissingET/MissingETContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "xAODRootAccess/TEvent.h"

#endif // not __MAKECINT__


template <class partContainer>
class ObjectReader : public ObjectReaderBase {

protected:

  // name of the container
  std::string m_containerName;

  // xAOD event
  xAOD::TEvent * m_event;
  
  std::vector<std::string> m_variations;
  
  bool m_haveNominal;
  bool m_nominalWasRead;
  
public:
  
  ObjectReader(const std::string& name, xAOD::TEvent * event);

  virtual ~ObjectReader() {}

  void discoverVariations(TTree* collectionTree);

  std::vector<std::string> getVariations() {return m_variations;}

  std::string getContainerName() {return m_containerName;}

#ifndef __MAKECINT__
  const partContainer* getObjects(std::string variation);
#endif // not __MAKECINT__

  void clearEvent();

};


 //=============================================================================
 // Define the implementation of the methods here in the header file.
 // This is done since we are dealing with a templated base class!
 //=============================================================================

#ifndef __MAKECINT__
#include "CxAODReader/ObjectReader.icc"
#endif // not __MAKECINT__

#endif
