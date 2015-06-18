#include "EventLoop/Job.h"
#include "EventLoop/StatusCode.h"
#include "EventLoop/Worker.h"
#include "EventLoop/OutputStream.h"
#include <CxAODReader/AnalysisReader.h>

#include "xAODEventInfo/EventInfo.h"
#include "xAODBTagging/BTagging.h"

#include "TLorentzVector.h"
#include "TSystem.h"
#include "TFile.h"

// this is needed to distribute the algorithm to the workers
ClassImp(AnalysisReader)
  AnalysisReader :: AnalysisReader () :
    m_debug(false),
    m_applyEventPreSelection(false),
    m_isMC(false),
    m_weight(1.),
    m_sumOfWeights(1.),
    m_isSherpaVJets(0),
    m_isSherpaPt0VJets(0),
    m_SherpaPt0VJetsCut(70000),
    m_eventInfoReader(nullptr),
    m_METReader(nullptr),
    m_electronReader(nullptr),
    m_photonReader(nullptr),
    m_muonReader(nullptr),
    m_tauReader(nullptr),
    m_jetReader(nullptr),
    m_fatJetReader(nullptr),
    m_truthParticleReader(nullptr),
    m_eventInfo(nullptr),
    m_met(nullptr),
    m_metCont(nullptr),
    m_electrons(nullptr),
    m_photons(nullptr),
    m_muons(nullptr),
    m_taus(nullptr),
    m_jets(nullptr),
    m_fatJets(nullptr),
    m_truthParts(nullptr),
    m_overlapRemoval(nullptr),
    m_xSectionProvider(nullptr),
    m_sumOfWeightsProvider(nullptr)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}

EL::StatusCode AnalysisReader :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.
  job.useXAOD ();

  // let's initialize the algorithm to use the xAODRootAccess package
  xAOD::Init( "MyxAODAnalysis" ).ignore(); // call before opening first file

  return EL::StatusCode::SUCCESS;
}

void AnalysisReader :: fillCutflow(std::string binName) {
  m_histMap["Cutflow"]->Fill(m_histMap["Cutflow"]->GetXaxis()->FindBin(binName.c_str()), m_weight);
  m_histMap["Cutflow_noweight"]->Fill(m_histMap["Cutflow"]->GetXaxis()->FindBin(binName.c_str()));
}

EL::StatusCode AnalysisReader :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  // In absence of hist manager suggest m_hist_Xlep_histname for pointers
  // name on file can be the same

  TH1::SetDefaultSumw2();

  // some histograms useful for all analyses
  // Truth pt
  m_hist_VPtTruth = new TH1F("VptTruth","V pt truth; V pt truth [GeV]; Events",100,0,1000);
  m_hist_VPtTruth->Sumw2();
  wk()->addOutput(m_hist_VPtTruth);

  m_histMap = {
    // preselection
    {"preSelMET", new TH1D("Pre-selection E_{T}^{miss}", "Pre-selection E_{T}^{miss}; E_{T}^{Miss} [GeV]; Normalized Events / 30 GeV ",60,200.,2000.)},
    {"preSelMETPhi", new TH1D("Pre-selection #phi(MET)", "Pre-selection #phi(MET); #phi(MET); Normalized events",16,-4.,4.)},
    {"preSelFatjetMult", new TH1D("Pre-selection Fatjet Multiplicity", "Pre-selection Fatjet Multiplicity; Multiplicity; Normalized Events",7,-0.5,6.5)}, 
    {"preSelFatjetMass", new TH1D("Pre-selection Fatjet Mass", "Pre-selection Fatjet Mass; Mass_{fatjet} [GeV]; Normalized Events",20,0.,400.)}, 
    {"preSelFatjetPt", new TH1D("Pre-selection Fatjet p_{T}", "Pre-selection Fatjet p_{T}; p_{T,fatjet} [GeV]; Normalized Events",50,0.,1000.)},
    {"preSelFatjetEta", new TH1D("Pre-selection Fatjet Eta", "Pre-selection Fatjet Eta; #eta_{fatjet}; Normalized Events",10,-1.,1.)},
    {"preSelJetMass", new TH1D("Pre-selection Jet Mass", "Pre-selection Jet Mass; Mass_{jet} [GeV]; Normalized Events",20,0.,100.)},
    {"preSelJetPt", new TH1D("Pre-selection Jet p_{T}", "Pre-selection Jet p_{T}; p_{T,jet} [GeV]; Normalized Events",25,0.,500.)},
    {"preSelJetEta", new TH1D("Pre-selection Jet Eta", "Pre-selection Jet Eta; #eta_{jet}; Normalized Events",10,-1.,1.)},
    {"preSelD2", new TH1D("Pre-selection D2", "Pre-selection D2; D2; Normalized Events",50,0.,10.)},
    {"preSelTau21", new TH1D("Pre-selection #tau_{21}", "Pre-selection #tau_{21}; #tau_{21}; Normalized Events",50,0.,1.5)},

    // MET cutflow
    {"fatjetCutflowMET", new TH1D("fatjet cutflow MET", "fatjet cutflow E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 30 GeV",60,200.,2000.)},
    {"jetvetoCutflowMET", new TH1D("jetveto cutflow MET", "jetveto cutflow E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 30 GeV",60,200.,2000.)},
    {"500METCutflowMET", new TH1D("500MET cutflow MET", "500MET cutflow E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 30 GeV",60,200.,2000.)},
    {"massFatjetCutflowMET", new TH1D("m(fatjet) cutflow MET", "m(fatjet) cutflow E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 30 GeV",60,200.,2000.)},
    {"signalFatjetCutflowMult", new TH1D("Signal Fatjet Multiplicity", "Signal Fatjet Multiplicity; Signal FatJet Multiplicity; Normalized Events",7, -0.5, 7.5)},
    {"deltaRFatjetCutflowJet", new TH1D("#Delta R(fatjet ,addjet)", "#Delta R(fatjet ,addjet)",10,0.,3.)},
    {"monoWZSRD2", new TH1D("MonoWZ SR: D2", "MonoWZ SR: D2;D2;Normalized Events",60,0.,6.)},
    {"monoWZSRTau21", new TH1D("MonoWZ SR: #tau_{21}", "MonoWZ SR: #tau_{21};#tau_{21};Normalized Events",50,0.,1.5)},

    // n-1
    {"fatjetMultN1", new TH1D("AKt10Trimmed jet Multiplicity onefatjet n-1", "AKt10Trimmed jet Multiplicity onefatjet n-1; Multiplicity_{fatjet}; Normalized Events",6,-0.5,5.5)},
    {"fatjetPtN1", new TH1D("p_{T,fatjet} onefatjet n-1", "p_{T,fatjet} onefatjet n-1;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",60,250.,1750.)},
    {"fatjetMassN1", new TH1D("Mass_{fatjet} onefatjet n-1", "Mass_{fatjet} onefatjet n-1;Mass_{fatjet} [GeV]; Normalized Events / 5 GeV",20,50.,150.)},
    {"fatjetEtaN1", new TH1D("#eta_{fatjet} onefatjet n-1", "#eta_{fatjet} onefatjet n-1;#eta_{fatjet}; Normalized Events / 0.25",20,-2.5,2.5)},
    {"fatjetMultMETN1", new TH1D("AKt10Trimmed jet Multiplicity MET n-1", "AKt10Trimmed jet Multiplicity met n-1; Multiplicity_{fatjet}; Normalized Events",6,-0.5,5.5)},
    {"fatjetPtMETN1", new TH1D("p_{T,fatjet} MET n-1", "p_{T,fatjet} met n-1;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",60,250.,1750.)},
    {"fatjetMassMETN1", new TH1D("Mass_{fatjet} MET n-1", "Mass_{fatjet} met n-1;Mass_{fatjet} [GeV]; Normalized Events / 5 GeV",20,50.,150.)},
    {"fatjetEtaMETN1", new TH1D("#eta_{fatjet} MET n-1", "#eta_{fatjet} met n-1;#eta_{fatjet}; Normalized Events / 0.25",20,-2.5,2.5)},
    {"addjetvetoMultN1", new TH1D("addjetveto n-1 sig addjet Multiplicity", "addjetveto n-1 sig addjet Multiplicity; Multiplicity; Normalized Events",10,-0.5,9.5)},
    {"METN1", new TH1D("E_{T}^{Miss} MET n-1", "E_{T}^{Miss} MET n-1; E_{T}^{Miss} [GeV]; Normalized Events / 50 GeV",30,200.,1700.)},
    {"massN1", new TH1D("Mass_{fatjet} n-1", "Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 5 GeV",40,0.,200.)},
    {"deltaPhiMETJetN1", new TH1D("#Delta#phi(MET,AKt4 jets) n-1", "#Delta#phi(MET,AKt4 jets) n-1; #Delta#phi(MET,AKt4 jets); Normalized Events",15,0,TMath::Pi())},

    // W->lv control region
    {"ptWe", new TH1D("W CR e:  p_{T,W}", "W CR e: p_{T,W} ; p_{T,W} [GeV]; Normalized Events / 25 GeV",28,50.,750.)},
    {"mtWe", new TH1D("W CR e:  M_{T,lep}", "W CR e: M_{T,lep} ; M_{T,lep} [GeV]; Normalized Events / 10 GeV",40,0.,400.)},
    {"fatjetMassWe", new TH1D("W CR e:  Mass_{fatjet}", "W CR e: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtWe", new TH1D("W CR e: p_{T,fatjet}", "W CR e: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaWe", new TH1D("W CR e: #eta_{fatjet}", "W CR e: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"bjetMultWe", new TH1D("W CR e: AKt4 b-jet Multiplicity", "W CR e: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},
    {"ptWm", new TH1D("W CR #mu: p_{T,W}", "W CR #mu: p_{T,W} ; p_{T,W} [GeV]; Normalized Events / 25 GeV",28,50.,750.)},
    {"mtWm", new TH1D("W CR #mu: M_{T,lep}", "W CR #mu: M_{T,lep} ; M_{T,lep} [GeV]; Normalized Events / 10 GeV",40,0.,400.)},
    {"fatjetMassWm", new TH1D("W CR #mu: Mass_{fatjet}", "W CR #mu: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtWm", new TH1D("W CR #mu: p_{T,fatjet}", "W CR #mu: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaWm", new TH1D("W CR #mu: #eta_{fatjet}", "W CR #mu: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"bjetMultWm", new TH1D("W CR #mu: AKt4 b-jet Multiplicity", "W CR #mu: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},

    // Z->ll control region
    {"ptZe", new TH1D("Zll CR e:  p_{T,Z}", "Zll CR e: p_{T,Z} ; p_{T,Z} [GeV]; Normalized Events / 20 GeV",25,250.,750.)},
    {"fatjetMassZe", new TH1D("Zll CR e:  Mass_{fatjet}", "Zll CR e: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 5 GeV",30,50.,200.)},
    {"fatjetPtZe", new TH1D("Zll CR e: p_{T,fatjet} fatjet", "Zll CR e: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaZe", new TH1D("Zll CR e: #eta_{fatjet}", "Zll CR e: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0,1.2)},
    {"bjetMultZe", new TH1D("Zll CR e: AKt4 b-jet Multiplicity", "Zll CR e: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},
    {"massZe", new TH1D("Zll CR e:  Mass_{ll}", "Zll CR e: Mass_{ll} ; Mass_{ll} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"ptZm", new TH1D("Zll CR #mu:  p_{T,Z}", "Zll CR #mu: p_{T,Z} ; p_{T,Z} [GeV]; Normalized Events / 20 GeV",25,250.,750.)},
    {"fatjetMassZm", new TH1D("Zll CR #mu:  Mass_{fatjet}", "Zll CR #mu: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 5 GeV",30,50.,200.)},
    {"fatjetPtZm", new TH1D("Zll CR #mu: p_{T,fatjet} fatjet", "Zll CR #mu: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaZm", new TH1D("Zll CR #mu: #eta_{fatjet}", "Zll CR #mu: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0,1.2)},
    {"bjetMultZm", new TH1D("Zll CR #mu: AKt4 b-jet Multiplicity", "Zll CR #mu: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},
    {"massZm", new TH1D("Zll CR #mu:  Mass_{ll}", "Zll CR #mu: Mass_{ll} ; Mass_{ll} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},

    // Z->vv control region
    {"fatjetMassZv", new TH1D("Zvv CR: Mass_{fatjet}", "Zvv CR: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtZv", new TH1D("Zvv CR: p_{T,fatjet} fatjet", "Zvv CR: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 50 GeV",10,250.,750.)},
    {"fatjetEtaZv", new TH1D("Zvv CR: #eta_{fatjet}", "Zvv CR: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"METZv", new TH1D("Zvv CR: E_{T}^{Miss}", "Zvv CR: E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 50 GeV",20,100.,1100.)},
    {"bjetMultZv", new TH1D("Zvv CR: AKt4 b-jet Multiplicity", "Zvv CR: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},

    // top control region
    {"deltaPhiMETJetTope", new TH1D("Top CR e: #Delta#phi(MET,AKt4 jets)", "Top CR e: #Delta#phi(MET,AKt4 jets) ; #Delta#phi(MET,AKt4 jets); Normalized Events/ 0.1",10,0.,1.)},
    {"fatjetMassTope", new TH1D("Top CR e:  Mass_{fatjet}", "Top CR e: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtTope", new TH1D("Top CR e: p_{T,fatjet} fatjet", "Top CR e: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaTope", new TH1D("Top CR e: #eta_{fatjet}", "Top CR e: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"METTope", new TH1D("Top CR e: E_{T}^{Miss}", "Top CR e: E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 50 GeV",20,200.,1200.)},
    {"bjetMultTope", new TH1D("Top CR e: AKt4 b-jet Multiplicity", "Top CR e: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},
    {"deltaPhiMETJetTopm", new TH1D("Top CR #mu: #Delta#phi(MET,AKt4 jets)", "Top CR #mu: #Delta#phi(MET,AKt4 jets) ; #Delta#phi(MET,AKt4 jets); Normalized Events/ 0.1",10,0.,1.)},
    {"fatjetMassTopm", new TH1D("Top CR #mu:  Mass_{fatjet}", "Top CR #mu: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtTopm", new TH1D("Top CR #mu: p_{T,fatjet} fatjet", "Top CR #mu: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 25 GeV",20,250.,750.)},
    {"fatjetEtaTopm", new TH1D("Top CR #mu: #eta_{fatjet}", "Top CR #mu: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"METTopm", new TH1D("Top CR #mu: E_{T}^{Miss}", "Top CR #mu: E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 50 GeV",20,200.,1200.)},
    {"bjetMultTopm", new TH1D("Top CR #mu: AKt4 b-jet Multiplicity", "Top CR #mu: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},

    // QCD control region
    {"deltaPhiMETJetQcd", new TH1D("QCD CR: #Delta#phi(MET,AKt4 jet)", "QCD CR: #Delta#phi(MET,AKt4 jets) ; #Delta#phi(MET,AKt4 jets); Normalized Events / 0.025",16,0.,0.4)},
    {"fatjetMassQcd", new TH1D("QCD CR: Mass_{fatjet}", "QCD CR: Mass_{fatjet} ; Mass_{fatjet} [GeV]; Normalized Events / 10 GeV",25,0.,250.)},
    {"fatjetPtQcd", new TH1D("QCD CR: p_{T,fatjet} fatjet", "QCD CR: p_{T,fatjet} ;p_{T, fatjet} [GeV]; Normalized Events / 50 GeV",10,250.,750.)},
    {"fatjetEtaQcd", new TH1D("QCD CR: #eta_{fatjet}", "QCD CR: #eta_{fatjet};#eta_{fatjet}; Normalized Events / 0.05",24,0.,1.2)},
    {"METQcd", new TH1D("QCD CR: E_{T}^{Miss}", "QCD CR: E_{T}^{Miss}; E_{T}^{Miss} [GeV]; Normalized Events / 50 GeV",22,100.,1200.)},
    {"bjetMultQcd", new TH1D("QCD CR: AKt4 b-jet Multiplicity", "QCD CR: AKt4 b-jet Multiplicity; Multiplicity_{AKt4 b-jet}; Normalized Events",6,-0.5,5.5)},
  };

  // setup histograms
  map<string,TH1D*>::iterator histItr = m_histMap.begin();
  for( ; histItr!=m_histMap.end(); ++histItr) {
    wk()->addOutput(histItr->second);
  }

  //cutflow
  const int nCuts = 12;
  static string cuts [nCuts] = {"All", "pre-selection",  "250met", "350met", "one fatjet", "no electron", "no muon", "no photon", "addjetveto", "phi metjet", "500met" ,"mJ_{W/Z}"};

  m_histMap["Cutflow"] = new TH1D("Cutflow","Cutflow", nCuts, 0.5, (float)(nCuts)+0.5);
  m_histMap["Cutflow_noweight"] = new TH1D("Cutflow_noweight","Cutflow_noweight", nCuts, 0.5, (float)(nCuts)+0.5);
  for(unsigned int i=0; i<nCuts; i++) {
    m_histMap["Cutflow"]->GetXaxis()->SetBinLabel(i+1,cuts[i].c_str());
    m_histMap["Cutflow_noweight"]->GetXaxis()->SetBinLabel(i+1,cuts[i].c_str());
  }
  wk()->addOutput(m_histMap["Cutflow"]);
  wk()->addOutput(m_histMap["Cutflow_noweight"]);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed

  // For everyfile we need to find the original sum of weights from the xAOD stored in the meta-data histogram
  TFile* inputfile=wk()->inputFile();

  TH1* metahist=(inputfile) ?  (TH1*)inputfile->Get("MetaData_EventCount") : 0;

  m_sumOfWeights=1.0;
  if (metahist) {
    m_sumOfWeights=metahist->GetBinContent(8);
  }

  // Flag if is Pt0 slice, to kill overlapping events. PtV slices only in 13TeV xAOD samples
  TString filename(inputfile->GetName());
  m_isSherpaPt0VJets= (filename.Contains("Pt0")) ? true : false;
  if(filename.Contains("Sherpa_CT10_W") && filename.Contains("Pt0")  && m_comEnergy=="13TeV") m_SherpaPt0VJetsCut = 40000 ;

  // general Sherpa flag
  m_isSherpaVJets = (filename.Contains("Sherpa")) ? true : false;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: changeInput (bool firstFile)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initialize ()
{
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  EL_CHECK("initialize()",initializeEvent());
  EL_CHECK("initialize()",initializeReader());
  EL_CHECK("initialize()",initializeSelection());
  EL_CHECK("initialize()",initializeIsMC());
  EL_CHECK("initialize()",initializeXSections());
  EL_CHECK("initialize()",initializeSumOfWeights());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initializeEvent()
{

  Info("initializeEvent()", "Initialize event.");

  m_event = wk()->xaodEvent();

  // as a check, let's see the number of events in our xAOD
  Info("initializeEvent()", "Number of events on first file = %lli", m_event->getEntries() ); // print long long int

  m_eventCounter = 0;

  // get configuration from steerfile
  //----------------------------------
  m_config = ConfigStore::createStore(m_configPath, false);
  if ( ! m_config ) {
    Error("initializeEvent()","Couldn't create ConfigStore from file : %s",m_configPath.c_str());
    return EL::StatusCode::FAILURE;
  }
  m_config->getif< bool >("debug", m_debug);
  
  // luminosity (for rescaling of MC to desired lumi, in fb-1)
  // default is 0.001 fb-1, which means no rescaling of the MC inputs (already scaled to 1 pb-1)
  m_luminosity = 0.001; 
  m_config->getif<float>("luminosity", m_luminosity);
  Info("initializeEvent()", "Luminosity for normalisation of MC = %f fb-1", m_luminosity);

  m_config->getif<bool>("passThroughOR", m_passThroughOR);
  if(m_passThroughOR) Warning("AnalysisReader :: AnalysisReader ()","Set OR to pass-through mode!");

  m_config->getif< bool >("applyEventPreSelection", m_applyEventPreSelection);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initializeReader()
{
  Info("initializeReader()", "Initialize object reader.");

  // note: the names of the readers determine which collections are
  //       read, see also framework-read.cfg

  m_eventInfoReader = registerReader<xAOD::EventInfo>("eventInfo");
  m_METReader       = registerReader<xAOD::MissingETContainer>("MET");
  m_electronReader  = registerReader<xAOD::ElectronContainer>("electron");
  m_photonReader    = registerReader<xAOD::PhotonContainer>("photon");
  m_muonReader      = registerReader<xAOD::MuonContainer>("muon");
  m_tauReader       = registerReader<xAOD::TauJetContainer>("tau");
  m_jetReader       = registerReader<xAOD::JetContainer>("jet");
  m_fatJetReader    = registerReader<xAOD::JetContainer>("fatJet");
  m_truthParticleReader = registerReader<xAOD::TruthParticleContainer>("truthParticle");
 
  return EL::StatusCode::SUCCESS;
}
 
EL::StatusCode AnalysisReader :: initializeSelection()
{
  Info("initializeSelection()", "Initialize selection.");
  
  m_overlapRemoval = new OverlapRemoval(*m_config);
  EL_CHECK("AnalysisReader::initializeSelection()", m_overlapRemoval -> initialize());

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initializeIsMC()
{
  Info("initializeIsMC()", "Initialize isMC.");
  // get nominal event info
  //-------------------------------------------------------------
  const xAOD::EventInfo* eventInfo = m_eventInfoReader->getObjects("Nominal");
  if(!eventInfo) return EL::StatusCode::FAILURE;

  // get MC flag - different info on data/MC files
  //-----------------------------------------------
  m_isMC = Props::isMC.get(eventInfo);
  Info("initializeIsMC()", "isMC = %i", m_isMC );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initializeXSections()
{
  Info("initializeXSections()", "Initialize cross sections.");

  if (!m_isMC) {
    return EL::StatusCode::SUCCESS;
  }

  // COM energy
  std::string comEnergy = m_config->get<std::string>("COMEnergy");
  
  std::string xSectionFile = gSystem->Getenv("ROOTCOREBIN");
  xSectionFile += "/data/FrameworkSub/XSections_";
  xSectionFile += comEnergy;
  xSectionFile += ".txt";
  m_xSectionProvider = new XSectionProvider(xSectionFile);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: initializeSumOfWeights()
{
  Info("initializeSumOfWeights()", "Initialize sum of weights.");

  if (!m_isMC) {
    return EL::StatusCode::SUCCESS;
  }

  // COM energy
  std::string comEnergy = m_config->get<std::string>("COMEnergy");

  std::string sumOfWeightsFile = gSystem->Getenv("ROOTCOREBIN");
  sumOfWeightsFile += "/data/FrameworkSub/yields.0lep.";
  sumOfWeightsFile += comEnergy;
  sumOfWeightsFile += ".txt";
  m_sumOfWeightsProvider = new sumOfWeightsProvider(sumOfWeightsFile);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader ::  setObjectsForOR(const xAOD::ElectronContainer* /*electrons*/, 
						  const xAOD::PhotonContainer* /*photons*/,	
						  const xAOD::MuonContainer* /*muons*/,		
						  const xAOD::TauJetContainer* /*taus*/,
						  const xAOD::JetContainer* /*jets*/){
return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  // print every 10000 events
  if( (m_eventCounter % 10000) ==0 ) Info("execute()", "Event number = %i", m_eventCounter );
  m_eventCounter++;

  // detemine if it is a Sherpa Pt0 sample, and remove events overlapping with slice (flag set for 13TeV only)
  float vpt_truth(-999);
  if (m_isSherpaVJets) {
    const xAOD::TruthParticleContainer* truthParts = 0;
    if( ! m_event->retrieve( truthParts, "TruthParticle___Nominal").isSuccess() ){
      Error("execute()", "Failed to retrieve  TruthParticle collection. Exiting." );
      return EL::StatusCode::FAILURE;
    }
    const xAOD::TruthParticle* lep0(0);
    const xAOD::TruthParticle* lep1(0);
    for (unsigned int iPart(0) ; iPart < truthParts->size(); ++iPart) {
      const xAOD::TruthParticle* part = truthParts->at(iPart);
      // assume leptons are the only status 3 particles in SherpaV+jets
      if (part->status()==3) {
        if (!lep0) lep0=part;
        else lep1=part;
      }
    }  
    if (lep0 && lep1) {
      TLorentzVector V(lep0->p4());
      V+=lep1->p4();
      vpt_truth=V.Pt();
      if (m_isSherpaPt0VJets &&  vpt_truth > m_SherpaPt0VJetsCut) 
      {
        if(m_debug) std::cout << "rejected: " << vpt_truth << std::endl;
        return EL::StatusCode::SUCCESS;
      }
    }
  }

  // reset event weight
  m_weight=1.0; 

  // Query - reading the cross section every event.
  // This is because on the first event fileExecute and changeInput are called before initialize
  // so there is no event available to read datasetid
  if (m_isMC) {
    EL_CHECK("AnalysisReader::execute()", getLumiWeight(m_weight));
  }

  if(m_debug && m_eventCounter<50) std::cout << m_weight << std::endl;

  // Sherpa Vpt  
  if (m_isSherpaVJets) 
  {
    m_hist_VPtTruth->Fill(vpt_truth/1000, m_weight);
  }

  m_eventInfo = m_eventInfoReader->getObjects("Nominal");
  m_metCont   = m_METReader->getObjects("Nominal");
  m_electrons = m_electronReader->getObjects("Nominal");
  m_photons   = m_photonReader->getObjects("Nominal");
  m_muons     = m_muonReader->getObjects("Nominal");
  m_taus      = m_tauReader->getObjects("Nominal");
  m_jets      = m_jetReader->getObjects("Nominal");
  m_fatJets   = m_fatJetReader->getObjects("Nominal");
  m_truthParts = m_truthParticleReader->getObjects("Nominal");

  m_met = nullptr;
  if(m_metCont->size() > 0) m_met = m_metCont->at(0);

  setObjectsForOR(m_electrons, m_photons, m_muons, m_taus, m_jets);
  if(m_passThroughOR) {
    OverlapPassThrough(m_electrons, m_photons, m_muons, m_taus, m_jets);
  } else {
    m_overlapRemoval->removeOverlap(m_electrons, m_photons, m_muons, m_taus, m_jets);
  }

  do_Analysis();

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AnalysisReader::getLumiWeight(double & weight) {
  
  weight = 1.;

  const xAOD::EventInfo* eventInfo = m_eventInfoReader->getObjects("Nominal");
  if (!eventInfo) {
    Error("getLumiWeight()", "EventInfo not found!");
    return EL::StatusCode::FAILURE;
  }

  // Query - reading the cross section every event.
  // This is because on the first event fileExecute and changeInput are called before initialize
  // so there is no event available to read datasetid
  int datasetid = eventInfo->mcChannelNumber();
  
  // TODO move sumOfWeights to changeInput()?
  // TODO need to know if we were running on DxAOD
  double sumOfWeights = 1;
  if (false) {
    // CxAOD was produced on DxAOD
    TFile* inputfile=wk()->inputFile();
    TH1* metahist = (TH1*) inputfile->Get("MetaData_EventCount");
    if (!metahist) {
      Error("getLumiWeight()", "SumOfWeights provider not initialized!");
      return EL::StatusCode::FAILURE;
    }
    //double nevents = metahist->GetBinContent(7);
    sumOfWeights = metahist->GetBinContent(8);
    //std::cout << " Number of Events/Sum of weights on xAOD " << nevents << " " << m_sumOfWeights << std::endl;
  } else {
    // CxAOD was produced on xAOD
    if (!m_sumOfWeightsProvider) {
      Error("getLumiWeight()", "SumOfWeights provider not initialized!");
      return EL::StatusCode::FAILURE;
    }
    sumOfWeights = m_sumOfWeightsProvider->getsumOfWeights(datasetid);
  }
  
  if (!m_xSectionProvider) {
    Error("getLumiWeight()", "XSection provider not initialized!");
    return EL::StatusCode::FAILURE;
  }

  float sigmaEff = m_xSectionProvider->getXSection(datasetid);
  //std::cout << "Cross section times eff. for dataset id " << datasetid << " = " << sigmaEff << std::endl;

  // we are normalising to MC lumi, sumOfWeights calculated per datasetid
  weight = (sumOfWeights) ? sigmaEff / sumOfWeights : 1.0;

  // multiply by MC generator weight 
  weight *= Props::MCEventWeight.get(eventInfo);

  // scale to desired luminosity
  weight *= (m_luminosity * 1e3); //the MC is scaled to 1pb-1 but m_luminosity is in fb-1

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: fill_oneLep(physicsObjects &po)
{
  //CR
  //W+jets CR
  //Only one muon or Only electron
  //1 fanjet with pT>250GeV and |eta|<1.2
  //pTW (vector sum of lepton + MET) >40GeV
  //MT > 40GeV
  //N b-tagged akt4 jet ==0

  bool pass_WCR = false;
  bool pass_onelep = false;
  bool pass_pTW = false;
  bool pass_MT = false;
  bool pass_zerobJet = false;
  bool oneElec = false;
  bool oneMuon =false;
  bool pass_fatjet = po.fatsigJets.size() > 0;

  TLorentzVector WVec;
  vector<const xAOD::Jet*> CRbJets;
  double MT = 0;

  if(po.tightElectrons.size() == 1) oneElec = true;
  if(po.tightMuons.size()==1) oneMuon = true;
  if((po.tightElectrons.size()+po.tightMuons.size()) == 1 ){
    pass_onelep = true;
    TLorentzVector lep;
    if(oneElec) lep.SetPtEtaPhiM(po.tightElectrons.at(0)->pt(), po.tightElectrons.at(0)->eta(), po.tightElectrons.at(0)->phi(), po.tightElectrons.at(0)->m());
    if(oneMuon) lep.SetPtEtaPhiM(po.tightMuons.at(0)->pt(), po.tightMuons.at(0)->eta(), po.tightMuons.at(0)->phi(), po.tightMuons.at(0)->m());
    WVec = lep + po.metVec;
    //transverse mass of lep  
    MT = TMath::Sqrt(2*lep.Pt()*(po.metVec.Pt())*(1-TMath::Cos(lep.DeltaPhi(po.metVec))))/1000.;    //GeV

  }
  //b-taged AntiKt4jet 
  for(unsigned int iJet(0); iJet < po.sigJets.size();iJet++){
    const xAOD::Jet* Jet = po.sigJets.at(iJet);  
    //if(!(m_superDecorator.get(Jet, JetFloatProps::MV1) > 0.971966))  continue; // 70% operation point 
    CRbJets.push_back(Jet);
  }
  if(WVec.Pt()/1000. > 40) pass_pTW = true;
  if(MT > 40) pass_MT = true;
  if(CRbJets.size() ==0) pass_zerobJet = true;
  if(pass_onelep && pass_pTW && pass_MT && pass_zerobJet && pass_fatjet) pass_WCR = true;

  //Fill W CR histogram
  if(pass_WCR){
    if(oneElec){
      m_histMap["ptWe"]->Fill(WVec.Pt()/1000. ,m_weight);  
      m_histMap["mtWe"]->Fill(MT, m_weight);
      m_histMap["bjetMultWe"]->Fill(CRbJets.size(), m_weight);
      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassWe"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtWe"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);  
        m_histMap["fatjetEtaWe"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);  
      }
    }
    if(oneMuon)  {
      m_histMap["ptWm"]->Fill(WVec.Pt()/1000. ,m_weight);  
      m_histMap["mtWm"]->Fill(MT, m_weight);
      m_histMap["bjetMultWm"]->Fill(CRbJets.size(), m_weight);
      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassWm"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtWm"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
        m_histMap["fatjetEtaWm"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);  
      }
    }
  }

  return EL::StatusCode::SUCCESS;
} 

EL::StatusCode AnalysisReader :: fill_twoLep(physicsObjects &po)
{
  //Zll CR
  //two opposite signed muon or two opposite signed electron with mini-isolation cut
  //if dR(l1,l2)<0.2, ptcon20 -= pT of the other lepton
  //1 fatjet with pT>250GeV and |eta|<1.2
  //N b-tagged akt4 jets==0
  bool pass_ZlCR = false;
  bool twolep = false;
  bool twoElecs = false;
  bool twoMuons = false;
  bool pass_iso = false;
  bool pass_Zmass = false;
  bool pass_zerobJet = false;
  bool pass_fatjet = po.fatsigJets.size() > 0;

  vector<const xAOD::Jet*> CRbJets;
  TLorentzVector lepvec1;
  TLorentzVector lepvec2;
  TLorentzVector Zvec;

  //b-taged AntiKt4jet 
  for(unsigned int iJet(0); iJet < po.sigJets.size();iJet++){
    const xAOD::Jet* Jet = po.sigJets.at(iJet);
    //if(!(m_superDecorator.get(Jet, JetFloatProps::MV1) > 0.971966))  continue; // 70% operation point 
    CRbJets.push_back(Jet);
  }
  if(CRbJets.size() ==0) pass_zerobJet = true;

  if(po.tightElectrons.size()==2){
    twoElecs = true;
    const xAOD::Electron* elec1 = po.tightElectrons.at(0);
    const xAOD::Electron* elec2 = po.tightElectrons.at(1);  
    lepvec1.SetPtEtaPhiM(elec1->pt(), elec1->eta(), elec1->phi(),elec1->m());
    lepvec2.SetPtEtaPhiM(elec2->pt(), elec2->eta(), elec2->phi(), elec2->m());
    float trackIso1, trackIso2 ;
    trackIso1 = elec1->isolationValue(xAOD::Iso::ptcone20);
    trackIso2 = elec2->isolationValue(xAOD::Iso::ptcone20);
    if(lepvec1.DeltaR(lepvec2) < 0.2 ){
      if((( trackIso1 - elec2->pt())/elec1->pt() < 0.1 ) && (( trackIso2 - elec1->pt())/elec2->pt() < 0.1 ) ) pass_iso = true;  
    }
    else {
      if(( trackIso1 /elec1->pt() < 0.1 ) && ( trackIso2 /elec2->pt() < 0.1 ) ) pass_iso = true;    
    }
  } 

  if(po.tightMuons.size()==2){
    twoMuons = true;  
    const xAOD::Muon* muon1 = po.tightMuons.at(0);
    const xAOD::Muon* muon2 = po.tightMuons.at(1);  
    lepvec1.SetPtEtaPhiM(muon1->pt(), muon1->eta(), muon1->phi(), muon1->m());
    lepvec2.SetPtEtaPhiM(muon2->pt(), muon2->eta(), muon2->phi(), muon2->m());
    float trackIso1, trackIso2 ;
    muon1->isolation(trackIso1,xAOD::Iso::ptcone20);
    muon2->isolation(trackIso2,xAOD::Iso::ptcone20);
    if(lepvec1.DeltaR(lepvec2) < 0.2 ){
      if((( trackIso1 - muon2->pt())/muon1->pt() < 0.1 ) && (( trackIso2 - muon1->pt())/muon2->pt() < 0.1 ) ) pass_iso = true;  
    }
    else {
      if(( trackIso1 /muon1->pt() < 0.1 ) && ( trackIso2 /muon2->pt() < 0.1 ) ) pass_iso = true;    
    }
  }  
  if(twoElecs ^ twoMuons) twolep = true;
  if(twolep){
    Zvec = lepvec1 + lepvec2;
  }
  double Zmass;
  Zmass = Zvec.M()/1000.;
  if( Zmass > 75 && Zmass < 105 ) pass_Zmass = true;
  if(pass_fatjet && pass_zerobJet && twolep && pass_iso && pass_Zmass) pass_ZlCR = true;


  if(pass_ZlCR){
    if(twoElecs){
      m_histMap["ptZe"]->Fill(Zvec.Pt()/1000., m_weight);
      m_histMap["massZe"]->Fill(Zmass, m_weight);
      m_histMap["bjetMultZe"]->Fill(CRbJets.size(),m_weight);
      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassZe"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtZe"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
        m_histMap["fatjetEtaZe"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);
      }  
    }
    if(twoMuons){
      m_histMap["ptZm"]->Fill(Zvec.Pt()/1000., m_weight);
      m_histMap["massZm"]->Fill(Zmass, m_weight);
      m_histMap["bjetMultZm"]->Fill(CRbJets.size(),m_weight);
      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassZm"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtZm"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
        m_histMap["fatjetEtaZm"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);
      }    
    }
  }

  return EL::StatusCode::SUCCESS;
} 

EL::StatusCode AnalysisReader :: fill_zvv(physicsObjects &po)
{
  //Zvv CR
  //no el,mu
  //1 fanjet with pT>250GeV and |eta|<1.2
  //MET>100GeV
  //min dPhi(Met, akt4)>0.4
  bool pass_ZvCR = false;
  bool pass_100met = false;
  bool ZvCRPhi =false;
  bool pass_fatjet = po.fatsigJets.size() > 0;
  bool pass_noelectron = false; 
  bool pass_nomuon = false; 
  double minPhiZv = 10;
  vector<const xAOD::Jet*> CRbJets;

  //b-taged AntiKt4jet 
  for(unsigned int iJet(0); iJet < po.sigJets.size();iJet++){
    const xAOD::Jet* Jet = po.sigJets.at(iJet);
    //if(!(m_superDecorator.get(Jet, JetFloatProps::MV1) > 0.971966))  continue; // 70% operation point 
    CRbJets.push_back(Jet);
  }
  if(po.looseElectrons.size() == 0 ) pass_noelectron = true;
  if(po.looseMuons.size() == 0 ) pass_nomuon = true;

  if(po.metVec.Pt()/1000. > 100.) pass_100met = true;
  for(unsigned int iJet(0);iJet <po.sigJets.size(); iJet++){
    TLorentzVector jetvec;
    jetvec.SetPtEtaPhiM(po.sigJets.at(iJet)->pt(), po.sigJets.at(iJet)->eta(), po.sigJets.at(iJet)->phi(), po.sigJets.at(iJet)->m());
    if(fabs(jetvec.DeltaPhi(po.metVec)) < minPhiZv) minPhiZv = fabs(jetvec.DeltaPhi(po.metVec));  
  }
  if(minPhiZv > 0.4)ZvCRPhi =true;
  if( pass_noelectron && pass_nomuon && pass_fatjet && pass_100met && ZvCRPhi) pass_ZvCR = true;

  //Fill Zvv hists
  if(pass_ZvCR){
    m_histMap["bjetMultZv"]->Fill(CRbJets.size(),m_weight);
    m_histMap["METZv"]->Fill(po.metVec.Pt()/1000., m_weight);
    for(unsigned int i(0);i < po.fatsigJets.size();i++){
      m_histMap["fatjetMassZv"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
      m_histMap["fatjetPtZv"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
      m_histMap["fatjetEtaZv"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);
    }  
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: fill_top(physicsObjects &po, TLorentzVector &fjet)
{
  //TopCR
  //one el or one muon (with isolation cut)
  //1 fanjet with pT>250GeV and |eta|<1.2
  //N b-tagged akt4jet >=1
  //min dR(lep, b-tagged akt4 jet)<1.5
  /*    Please modify the min dR(lep, bakt4)<1.5  selection to:
        - min dR(lep, akt4) <1.5
        - N(b-tagged akt4 jets with dR(fatjet, b-tagged akt4)<1.0) >=1    
   * Show the N-1 b-jet multiplicity distribution.*/

  bool pass_TopCR = false;
  bool pass_nB = false;
  bool pass_minR = false;
  bool oneElec = false;
  bool oneMuon =false;
  bool pass_fatjet = po.fatsigJets.size() > 0;
  bool pass_onelep = po.tightElectrons.size()+po.tightMuons.size() == 1;
  double minPhiZv = 10;

  TLorentzVector WVec;

  if(po.tightElectrons.size() == 1) oneElec = true;
  if(po.tightMuons.size()==1) oneMuon = true;

  vector<const xAOD::Jet*> TopCRbJets;
  vector<const xAOD::Jet*> TopCRJets;
  double minR = 999;

  for(unsigned int iJet(0);iJet <po.sigJets.size(); iJet++){
    TLorentzVector jetvec;
    jetvec.SetPtEtaPhiM(po.sigJets.at(iJet)->pt(), po.sigJets.at(iJet)->eta(), po.sigJets.at(iJet)->phi(), po.sigJets.at(iJet)->m());
    if(fabs(jetvec.DeltaPhi(po.metVec)) < minPhiZv) minPhiZv = fabs(jetvec.DeltaPhi(po.metVec));
  }

  if(pass_onelep){
    TLorentzVector lep; 

    if(oneElec) lep.SetPtEtaPhiM(po.tightElectrons.at(0)->pt(), po.tightElectrons.at(0)->eta(), po.tightElectrons.at(0)->phi(), po.tightElectrons.at(0)->m());
    if(oneMuon) lep.SetPtEtaPhiM(po.tightMuons.at(0)->pt(), po.tightMuons.at(0)->eta(), po.tightMuons.at(0)->phi(), po.tightMuons.at(0)->m());
    for(unsigned int iJet(0);iJet < po.sigJets.size() ;iJet++){
      const xAOD::Jet* Jet = po.sigJets.at(iJet);
      TLorentzVector jetvec;
      jetvec.SetPtEtaPhiM(Jet->pt(), Jet->eta(), Jet->phi(), Jet->m());    
      if( minR > jetvec.DeltaR(lep)) minR = jetvec.DeltaR(lep);  
      TopCRJets.push_back(Jet);
      //if(!(m_superDecorator.get(Jet, JetFloatProps::MV1) > 0.971966))  continue; // 70% operation point 
      if(!(jetvec.DeltaR(fjet) < 1.))continue;
      TopCRbJets.push_back(Jet);    
    }
  }

  if(minR < 1.5) pass_minR = true;
  if(TopCRbJets.size() > 0) pass_nB = true ; 

  if(pass_onelep && pass_fatjet && pass_nB && pass_minR ) pass_TopCR = true;

  if(pass_onelep && pass_fatjet && pass_minR )  {
    if(oneElec)  m_histMap["bjetMultTope"]->Fill(TopCRbJets.size(), m_weight);
    if(oneMuon)  m_histMap["bjetMultTopm"]->Fill(TopCRbJets.size(), m_weight);
  }

  if(pass_TopCR){
    if(oneElec){
      m_histMap["deltaPhiMETJetTope"]->Fill(minPhiZv, m_weight);  //?
      m_histMap["METTope"]->Fill(po.metVec.Pt()/1000., m_weight);

      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassTope"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtTope"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
        m_histMap["fatjetEtaTope"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);    
      }  
    }    
    if(oneMuon){
      m_histMap["deltaPhiMETJetTopm"]->Fill(minPhiZv, m_weight);    //?
      m_histMap["METTopm"]->Fill(po.metVec.Pt()/1000., m_weight);

      for(unsigned int i(0);i < po.fatsigJets.size();i++){
        m_histMap["fatjetMassTopm"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
        m_histMap["fatjetPtTopm"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
        m_histMap["fatjetEtaTopm"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);    
      }  
    }      
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: fill_qcd(physicsObjects &po)
{
  //QCD CR
  bool pass_QCDCR = false;
  bool pass_minPhi = false;
  bool pass_100met = false;
  bool pass_fatjet = po.fatsigJets.size() > 0;
  bool pass_noelectron = false;
  bool pass_nomuon = false;
  double minPhiZv = 10;
  vector<const xAOD::Jet*> CRbJets;

  //b-taged AntiKt4jet 
  for(unsigned int iJet(0); iJet < po.sigJets.size();iJet++){
    const xAOD::Jet* Jet = po.sigJets.at(iJet);
    //if(!(m_superDecorator.get(Jet, JetFloatProps::MV1) > 0.971966))  continue; // 70% operation point 
    CRbJets.push_back(Jet);
  }
  if(po.looseElectrons.size() == 0 ) pass_noelectron = true;
  if(po.looseMuons.size() == 0 ) pass_nomuon = true;
  if(po.metVec.Pt()/1000. > 100.) pass_100met = true;

  for(unsigned int iJet(0);iJet <po.sigJets.size(); iJet++){
    TLorentzVector jetvec;
    jetvec.SetPtEtaPhiM(po.sigJets.at(iJet)->pt(), po.sigJets.at(iJet)->eta(), po.sigJets.at(iJet)->phi(), po.sigJets.at(iJet)->m());
    if(fabs(jetvec.DeltaPhi(po.metVec)) < minPhiZv) minPhiZv = fabs(jetvec.DeltaPhi(po.metVec));
  }

  if(minPhiZv < 0.4) pass_minPhi = true;
  if( pass_noelectron && pass_nomuon && pass_fatjet && pass_100met && pass_minPhi) pass_QCDCR = true;

  //Fill QCD hists
  if(pass_QCDCR){
    m_histMap["deltaPhiMETJetQcd"]->Fill(minPhiZv, m_weight);  
    m_histMap["METQcd"]->Fill(po.metVec.Pt()/1000., m_weight);
    m_histMap["bjetMultQcd"]->Fill(CRbJets.size(),m_weight);
    for(unsigned int i(0);i < po.fatsigJets.size();i++){
      m_histMap["fatjetMassQcd"]->Fill(po.fatsigJets.at(i)->m()/1000., m_weight);
      m_histMap["fatjetPtQcd"]->Fill(po.fatsigJets.at(i)->pt()/1000., m_weight);
      m_histMap["fatjetEtaQcd"]->Fill(fabs(po.fatsigJets.at(i)->eta()), m_weight);    
    }   
  }

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: fill_monoWZ(physicsObjects &po, TLorentzVector &fjet)
{
  //Event selection
  bool pass_250met = false;
  bool pass_fatjet = false;
  bool pass_noelectron = false;
  bool pass_nomuon = false;
  bool pass_jetveto = false;
  bool pass_phimetjet = false;
  bool pass_metcut = false;
  //monoWZ
  bool pass_jetmass = false;

  //SR
  //met > 250 GeV
  if(po.metVec.Pt()/1000. > 250.) pass_250met = true; 
  //a least one fatjet   
  if(po.fatsigJets.size() > 0) pass_fatjet = true; 
  //leptons&photons veto
  if(po.looseElectrons.size() == 0 ) pass_noelectron = true;
  if(po.looseMuons.size() == 0) pass_nomuon = true;
  //at most one additional jet 
  if(po.sigJets.size() < 2 ) pass_jetveto = true;
  //DeltaPhi(met,jets) > 0.4
  double minPhi = 1;
  for(unsigned int j(0);j < po.metJets.size();j++){
    TLorentzVector jetvec;
    jetvec.SetPtEtaPhiM(po.metJets.at(j)->pt(), po.metJets.at(j)->eta(), po.metJets.at(j)->phi(), po.metJets.at(j)->m());
    if(fabs(jetvec.DeltaPhi(po.metVec)) < minPhi) minPhi = fabs(jetvec.DeltaPhi(po.metVec));
  }
  if(minPhi > 0.4) pass_phimetjet = true;
  //MET > 500 GeV
  if(po.metVec.Pt()/1000. > 500.) pass_metcut = true; // ANDY

  if(po.fatsigJets.size() > 0)
  {
    //monoWZ
    // 50 < mJ < 120 GeV 
    if(fjet.M()/1000. < 120. && fjet.M()/1000. > 50.) pass_jetmass = true;
  }


  //fill cutflow hist  
  fillCutflow("pre-selection");

  if(pass_250met)
  {
    fillCutflow("250met");
  }
  if(pass_250met )
  {
    fillCutflow("350met");
  }
  if(pass_250met && pass_fatjet)
  {
    fillCutflow("one fatjet");
    m_histMap["fatjetCutflowMET"]->Fill(po.metVec.Pt()/1000., m_weight);
  }
  if(pass_250met  && pass_fatjet && pass_noelectron)
  {
    fillCutflow("no electron");
  }  
  if(pass_250met  && pass_fatjet && pass_noelectron && pass_nomuon)
  {
    fillCutflow("no muon");
  }
  if(pass_250met  && pass_fatjet && pass_noelectron && pass_nomuon)
  {
    fillCutflow("no photon");
  }
  if(pass_250met  && pass_fatjet &&  pass_noelectron && pass_nomuon && pass_jetveto)
  {
    fillCutflow("addjetveto");
    m_histMap["jetvetoCutflowMET"]->Fill(po.metVec.Pt()/1000., m_weight);
  }
  if(pass_250met  && pass_fatjet &&  pass_noelectron && pass_nomuon && pass_jetveto && pass_phimetjet)
  {  
    fillCutflow("phi metjet");
  }
  if(pass_250met  && pass_fatjet &&  pass_noelectron && pass_nomuon && pass_jetveto && pass_phimetjet && pass_metcut)
  {
    fillCutflow("500met");
    m_histMap["500METCutflowMET"]->Fill(po.metVec.Pt()/1000., m_weight);
    for(unsigned int iJet(0); iJet < po.fatsigJets.size();iJet++){
      const xAOD::Jet* Jet = po.fatsigJets.at(iJet);  
      m_histMap["preSelTau21"]->Fill(Props::Tau21.get(Jet), m_weight);
      m_histMap["preSelD2"]->Fill(Props::D2.get(Jet), m_weight);
    }
  }
  if(pass_250met  && pass_fatjet &&  pass_noelectron && pass_nomuon && pass_jetveto && pass_phimetjet && pass_metcut && pass_jetmass)
  { 
    fillCutflow("mJ_{W/Z}");
    m_histMap["massFatjetCutflowMET"]->Fill(po.metVec.Pt()/1000., m_weight);
    for(unsigned int iJet(0); iJet < po.fatsigJets.size();iJet++){
      const xAOD::Jet* Jet = po.fatsigJets.at(iJet);  
      m_histMap["monoWZSRTau21"]->Fill(Props::Tau21.get(Jet), m_weight);
      m_histMap["monoWZSRD2"]->Fill(Props::D2.get(Jet), m_weight);
    }
  }

  //fill n-1 histogram
  //one fatjet 
  if( pass_noelectron && pass_nomuon && pass_250met && pass_jetveto && pass_metcut && pass_jetmass  && pass_phimetjet)
  {
    m_histMap["fatjetMultN1"]->Fill(po.fatsigJets.size(), m_weight);  
    m_histMap["fatjetPtN1"]->Fill(fjet.Pt()/1000., m_weight);
    m_histMap["fatjetMassN1"]->Fill(fjet.M()/1000., m_weight);
    m_histMap["fatjetEtaN1"]->Fill(fjet.Eta(), m_weight);  
  } 
  //jet veto
  if( pass_noelectron && pass_nomuon && pass_250met && pass_fatjet && pass_metcut && pass_jetmass && pass_phimetjet)
  {
    m_histMap["addjetvetoMultN1"]->Fill(po.sigJets.size(), m_weight);
  } 
  //500met 
  if( pass_noelectron && pass_nomuon && pass_fatjet && pass_jetveto && pass_jetmass && pass_phimetjet)
  {
    m_histMap["METN1"]->Fill(po.metVec.Pt()/1000., m_weight);
  }
  // 500 MET N-1 w/ 250 MET cut
  if( pass_noelectron && pass_nomuon && pass_250met && pass_fatjet && pass_jetveto && pass_jetmass && pass_phimetjet)
  {
    m_histMap["fatjetMultMETN1"]->Fill(po.fatsigJets.size(), m_weight);
    m_histMap["fatjetPtMETN1"]->Fill(fjet.Pt()/1000., m_weight);
    m_histMap["fatjetMassMETN1"]->Fill(fjet.M()/1000., m_weight);
    m_histMap["fatjetEtaMETN1"]->Fill(fjet.Eta(), m_weight);
  } 
  //mJ
  if( pass_noelectron && pass_nomuon && pass_250met  && pass_fatjet && pass_jetveto && pass_metcut && pass_phimetjet)
  {
    m_histMap["massN1"]->Fill(fjet.M()/1000., m_weight);
  } 
  //phimetjet
  if(pass_250met  && pass_fatjet &&  pass_noelectron && pass_nomuon && pass_jetveto  && pass_metcut && pass_jetmass)
  {
    for(unsigned int j(0);j < po.sigJets.size();j++){
      TLorentzVector jetsvec;
      jetsvec.SetPtEtaPhiM(po.sigJets.at(j)->pt(), po.sigJets.at(j)->eta(), po.sigJets.at(j)->phi(), po.sigJets.at(j)->m());
      m_histMap["deltaPhiMETJetN1"]->Fill(fabs(jetsvec.DeltaPhi(po.metVec)), m_weight);  
    }

  }

  return EL::StatusCode::SUCCESS;
} 

EL::StatusCode AnalysisReader :: do_Analysis()
{
  //------------------------
  //Event Selection
  //-------------------------
  // select events
  vector<const xAOD::Jet*> selJets;

  for(unsigned int iJet(0); iJet < m_jets->size();iJet++){
    const xAOD::Jet* Jet = m_jets->at(iJet);
    selJets.push_back(Jet);
  }

  //objects selections
  physicsObjects po;
  TLorentzVector fjet;

  //fat jet selection Pt > 250GeV |eta| < 1.2
  for(unsigned int i(0); i < m_fatJets->size() ;i++){
    if(m_fatJets->at(i)->pt()/1000. < 250.) continue;
    if(fabs(m_fatJets->at(i)->eta()) > 1.2) continue;
    po.fatsigJets.push_back(m_fatJets->at(i));
  }  

  //find leading fatjet
  int leadingfatjet = 0;
  for(unsigned int i(0);i < po.fatsigJets.size();i++){
    if(po.fatsigJets.at(i)->pt() > po.fatsigJets.at(leadingfatjet)->pt() ) leadingfatjet = i; 
  }

  if(po.fatsigJets.size() > 0) fjet.SetPtEtaPhiM(po.fatsigJets.at(leadingfatjet)->pt(), po.fatsigJets.at(leadingfatjet)->eta(), po.fatsigJets.at(leadingfatjet)->phi(), po.fatsigJets.at(leadingfatjet)->m());
  po.metVec.SetPtEtaPhiM(m_met->met(), 0, m_met->phi(), 0);

  //loose quality cut
  //Electrons: isVHLooseElectron, Isolation cut: ptcone20/pt<0.1
  //Muon: isVHLooseMuon,  Isolation cut: ptcone20/pt<0.1
  for(unsigned int i(0); i < m_electrons->size();i++){
    const xAOD::Electron* elec = m_electrons->at(i);
    if(!Props::isVHLooseElectron.get(elec)) continue; 
    if(!(elec->isolationValue(xAOD::Iso::ptcone20) / elec->pt() < 0.1)) continue;
    po.looseElectrons.push_back(elec);
  }
  for(unsigned int i(0); i < m_muons->size();i++){
    const xAOD::Muon* muon = m_muons->at(i);
    if(!Props::isVHLooseMuon.get(muon)) continue;
    float trackIso = -999.;
    muon->isolation(trackIso,xAOD::Iso::ptcone20); 
    if(!((trackIso/ muon->pt()) < 0.1))continue;
    po.looseMuons.push_back(muon);
  }
  
  //tight quality cut
  //Electrons: isVHSignalElectron, Isolation cut: ptcone20/pt<0.1
  //Muon: isVHSignalMuon,  Isolation cut: ptcone20/pt<0.1
  for(unsigned int i(0); i < m_electrons->size();i++){
    const xAOD::Electron* elec = m_electrons->at(i);
    if(!Props::isWHSignalElectron.get(elec)) continue;
    if(!(elec->isolationValue(xAOD::Iso::ptcone20) / elec->pt() < 0.1)) continue;
    po.tightElectrons.push_back(elec);
  }
  for(unsigned int i(0); i < m_muons->size();i++){
    const xAOD::Muon* muon = m_muons->at(i);
    if(!Props::isWHSignalMuon.get(muon)) continue;
    float trackIso = -999.;
    muon->isolation(trackIso,xAOD::Iso::ptcone20); 
    if(!((trackIso/ muon->pt()) < 0.1))continue;
    po.tightMuons.push_back(muon);
  }

  //addjets selection (pT > 40 GeV |eat| < 4.5 deltaR(leading fat,addjet) < 4.5) 
  for(unsigned int i(0);i < selJets.size();i++){
    TLorentzVector addjet;
    addjet.SetPtEtaPhiM(selJets.at(i)->pt(), selJets.at(i)->eta(), selJets.at(i)->phi(), selJets.at(i)->m());
    float DeltaR = 1.;  //set to 1 avoid the condition no signal fat jet 

    if(!((selJets.at(i)->pt()/1000. > 40) && (fabs(selJets.at(i)->eta()) < 4.5)))  continue;
    if(po.fatsigJets.size() > 0) 
    {
      DeltaR = fjet.DeltaR(addjet);
      m_histMap["deltaRFatjetCutflowJet"]->Fill(DeltaR, m_weight);
    }
    if(!(DeltaR > 0.9)) continue;
    //OverlapRemove if DeltaR(akt4, veto electron ) < 0.4
    bool overlapRemove = false;
    for(unsigned int ielec(0);ielec < po.looseElectrons.size();ielec++){
      TLorentzVector elec;
      elec.SetPtEtaPhiM(po.looseElectrons.at(ielec)->pt(), po.looseElectrons.at(ielec)->eta(), po.looseElectrons.at(ielec)->phi(), po.looseElectrons.at(ielec)->m());
      if(elec.DeltaR(addjet) < 0.4) overlapRemove = true;
    }  

    if(overlapRemove) continue;
    po.sigJets.push_back(selJets.at(i));

  }
  //select met jet 
  for(unsigned int iJet(0);iJet < selJets.size();iJet++){
    const xAOD::Jet* Jet = selJets.at(iJet);
    if(!(((Jet->pt()/1000.) > 20 )&& (fabs(Jet->eta()) < 4.5))) continue;
    TLorentzVector jetvec;
    jetvec.SetPtEtaPhiM(Jet->pt(), Jet->eta(), Jet->phi(), Jet->m());
    //OverlapRemove if DeltaR(akt4, veto electron ) < 0.4
    bool overlapRemove = false;
    for(unsigned int ielec(0);ielec < po.looseElectrons.size();ielec++){
      TLorentzVector elec;
      elec.SetPtEtaPhiM(po.looseElectrons.at(ielec)->pt(), po.looseElectrons.at(ielec)->eta(), po.looseElectrons.at(ielec)->phi(), po.looseElectrons.at(ielec)->m());
      if(elec.DeltaR(jetvec) < 0.4) overlapRemove = true;
    }  
    if(overlapRemove) continue;
    po.metJets.push_back(Jet);
  }

  fillCutflow("All");
  // fill pre-selection 
  m_histMap["preSelMET"]->Fill(po.metVec.Pt()/1000., m_weight);
  m_histMap["preSelFatjetMult"]->Fill(m_fatJets->size());
  for(unsigned int i(0); i < m_fatJets->size() ;i++){
    m_histMap["preSelFatjetMass"]->Fill(m_fatJets->at(i)->m()/1000., m_weight); 
    m_histMap["preSelFatjetPt"]->Fill(m_fatJets->at(i)->pt()/1000., m_weight); 
    m_histMap["preSelFatjetEta"]->Fill(m_fatJets->at(i)->eta(), m_weight); 
  }
  for(unsigned int i(0); i < selJets.size() ;i++){
    m_histMap["preSelJetMass"]->Fill(selJets.at(i)->m()/1000., m_weight); 
    m_histMap["preSelJetPt"]->Fill(selJets.at(i)->pt()/1000., m_weight); 
    m_histMap["preSelJetEta"]->Fill(selJets.at(i)->eta(), m_weight); 
  }
 
  // Fill control regions and signal regions
  fill_oneLep(po);
  fill_twoLep(po);
  fill_zvv(po);
  fill_top(po, fjet);
  fill_qcd(po);
  fill_monoWZ(po, fjet);

  return EL::StatusCode::SUCCESS;
}
  

EL::StatusCode AnalysisReader :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: OverlapPassThrough(const xAOD::ElectronContainer* electrons,
                                                    const xAOD::PhotonContainer* photons,
                                                    const xAOD::MuonContainer* muons,
                                                    const xAOD::TauJetContainer* taus,
                                                    const xAOD::JetContainer* jets)  {
  if (electrons) {
    for (const xAOD::Electron* elec : *electrons) {
      Props::passOR.set(elec, true);
    }
  }

  if (photons) {
    for (const xAOD::Photon* photon : *photons) {
      Props::passOR.set(photon, true);
    }
  }

  if (muons) {
    for (const xAOD::Muon* muon : *muons) {
      Props::passOR.set(muon, true);
    }
  }
  if (taus) {
    for (const xAOD::TauJet* tau : *taus) {
      Props::passOR.set(tau, true);
    }
  }

  if (jets) {
    for (const xAOD::Jet* jet : *jets) {
      Props::passOR.set(jet, true);
    }
  }

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AnalysisReader :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.

  Info("finalize()", "Processed %i Events", m_eventCounter );

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AnalysisReader :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.

  return EL::StatusCode::SUCCESS;
}
