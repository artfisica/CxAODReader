#include "xAODRootAccess/Init.h"
#include "SampleHandler/SampleHandler.h"
#include "SampleHandler/ToolsDiscovery.h"
#include "SampleHandler/DiskListLocal.h"
#include "SampleHandler/DiskListEOS.h"
#include "SampleHandler/Sample.h"

#include "EventLoop/Job.h"
#include "EventLoop/LSFDriver.h"
#include "EventLoop/DirectDriver.h"
#include "EventLoop/ProofDriver.h"
#include "EventLoop/Driver.h"

#include <stdlib.h> 
#include <vector> 

#include <TSystem.h> 
#include <TFile.h> 

#include "CxAODTools/ConfigStore.h"

#include "CxAODReader/AnalysisReader.h"

void tag(SH::SampleHandler& sh, const std::string& tag);

int main(int argc, char* argv[]) {

  // Take the submit directory from the input if provided:
  std::string submitDir = "submitDir";
  std::string configPath = "data/FrameworkExe/framework-read.cfg";
  if (argc > 1) submitDir = argv[1];
  //if (argc > 2) configPath = argv[2];
  int run(0);
  if (argc > 2) run = atoi(argv[2]);
  // read run config
  static ConfigStore* config = ConfigStore::createStore(configPath);

  // Set up the job for xAOD access:
  xAOD::Init().ignore();

  // Construct the samples to run on:
  SH::SampleHandler sampleHandler;

  // choose D4 samples
  std::vector<std::string> sample_names;
  // list all samples here - if they don't exist it will skip
  if(run==0)sample_names.push_back("test");
  if(run==1)sample_names.push_back("Zvv");
  if(run==2)sample_names.push_back("Wev");
  if(run==3)sample_names.push_back("Wmuv");
  if(run==4)sample_names.push_back("Wtauv");
  if(run==5)sample_names.push_back("ttbarallhad");
  if(run==6)sample_names.push_back("ttbarnonall");
  if(run==8)sample_names.push_back("dijet");  
  if(run==10)sample_names.push_back("singletop");
  if(run==11){
    sample_names.push_back("monoWjjIsrDesD1m50");
    sample_names.push_back("monoWjjIsrDesD1m1300");
    sample_names.push_back("monoWjjIsrDesD5m50");
    sample_names.push_back("monoWjjIsrDesD5m1300");
    sample_names.push_back("monoWjjIsrDesD9m50");
    sample_names.push_back("monoWjjIsrDesD9m1300");
    sample_names.push_back("monoWjjIsrConD5m50");
    sample_names.push_back("monoWjjIsrConD5m1300");
    sample_names.push_back("monoWjjSimDesSsdMed100m50");
    sample_names.push_back("monoWjjSimDesSsdMed1500m50");
    sample_names.push_back("monoWjjSimDesSvdMed100m50");
    sample_names.push_back("monoWjjSimDesSvdMed1500m50");
    sample_names.push_back("monoWjjSimDesSvdMed1500m1300");
    sample_names.push_back("monoWjjSimConSvdMed100m50");
    sample_names.push_back("monoWjjSimConSvdMed1500m50");
    sample_names.push_back("monoWjjSimConSvdMed1500m1300");
    sample_names.push_back("monoWjjWwxxm50");
    sample_names.push_back("monoWjjWwxxm1300");
  
    sample_names.push_back("monoZjjIsrD1m50");
    sample_names.push_back("monoZjjIsrD1m1300");
    sample_names.push_back("monoZjjIsrD5m50");
    sample_names.push_back("monoZjjIsrD5m1300");
    sample_names.push_back("monoZjjIsrD9m50");
    sample_names.push_back("monoZjjIsrD9m1300");
    sample_names.push_back("monoZjjSimSsdMed100m50");
    sample_names.push_back("monoZjjSimSsdMed1500m50");
    sample_names.push_back("monoZjjSimSsdMed1500m1300");
    sample_names.push_back("monoZjjSimSvdMed100m50");
    sample_names.push_back("monoZjjSimSvdMed1500m50");
    sample_names.push_back("monoZjjSimSvdMed1500m1300");
    sample_names.push_back("monoZjjZzxxm50");
    sample_names.push_back("monoZjjZzxxm1300");
  }
  if(run==12)sample_names.push_back("gamma");
  if(run==19)sample_names.push_back("Zee");
  if(run==20)sample_names.push_back("Zmumu");
  if(run==21)sample_names.push_back("Ztautau");

  // where to read data from
  bool eos(false);

  // datasets copied to afs
  //std::string afsdir("/data8/data/anelson/CxAOD.v01/");
  //std::string afsdir("/gdata/atlas/anelson/CxAOD.v02/");
  std::string afsdir("/gdata/atlas/anelson/CxAOD.SubStructure/");
  // datasets copied to eos
  TString CxAODver("CxAOD_00-00-05");
  TString xAODver("_p1784");
  TString eosdir("/eos/atlas/atlasgroupdisk/phys-higgs/HSG5/Run2/VH/");

  if (eos)
    std::cout << " Looking for directory " << eosdir << std::endl;

  std::string dataset_dir(eosdir);
  if (!eos) dataset_dir=afsdir;

  // Query - I had to put each background in a separate directory
  // for samplehandler to have sensible sample names etc.
  // Is it possible to define a search string for directories and assign all those to a sample name?
  //  SH::scanDir (sampleHandler, list); // apparently it will come

  // tuples are downloaded to same directory so specify CxAOD root file pattern
  for (unsigned int isamp(0) ; isamp<sample_names.size() ; isamp++) {
    std::string sample_name(sample_names.at(isamp));
    std::string sample_dir(dataset_dir+sample_name);

    if (!eos) {
      bool direxists=gSystem->OpenDirectory (sample_dir.c_str());
      if (!direxists) {
	std::cout << " No sample exists: " << sample_name << " , skipping: "  << sample_dir << std::endl;
	continue;
      }
    }

    // eos, local disk or afs
    if (eos) {
      SH::DiskListEOS list(sample_dir,"root://eosatlas/"+sample_dir );
      //SH::DiskListEOS list("eosatlas.cern.ch", sample_dir);
      SH::scanSingleDir (sampleHandler, sample_name, list, "*outputLabel*") ;
    } else {
      SH::DiskListLocal list(sample_dir);
      SH::scanSingleDir (sampleHandler, sample_name, list, "*outputLabel*") ;
      //SH::scanSingleDir (sampleHandler, sample_name, list, "*data_CxAOD*") ;
    }

    //
    SH::Sample* sample=sampleHandler.get(sample_name);
    int nsampleFiles=sample->numFiles();

    std::cout << "Sample name " << sample_name << " with nfiles : " <<  nsampleFiles  << std::endl;
  }


  // Set the name of the input TTree. It's always "CollectionTree"
  // for xAOD files.
  sampleHandler.setMetaString("nc_tree", "CollectionTree");

  // Print what we found:
  sampleHandler.print();

  // Create an EventLoop job:
  EL::Job job;
  job.sampleHandler(sampleHandler);

  // remove submit dir before running the job
  //job.options()->setDouble(EL::Job::optRemoveSubmitDir, 1);

  // create algorithm, set job options, maniplate members and add our analysis to the job:
  AnalysisReader* algorithm = new AnalysisReader();
  algorithm->setConfig(configPath); 

  //limit number of events to maxEvents - set in config
  job.options()->setDouble (EL::Job::optMaxEvents, config->get<int>("maxEvents"));

  // try different access
  // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EventLoop#Access_the_Data_Through_xAOD_EDM
  //job.options()->setString (EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_class);
  //job.options()->setString (EL::Job::optXaodAccessMode, EL::Job::optXaodAccessMode_class);

  // set the lepton analysis type and COM
  //algorithm->SetAnalysisType(analysisType);
  //algorithm->SetCOMEnergy(comEnergy);

  // uncomment me
  job.algsAdd(algorithm);

  //  return 0;

  // Run the job using the local/direct driver:
  EL::DirectDriver driver;
  driver.submit(job, submitDir);

  return 0;
}
