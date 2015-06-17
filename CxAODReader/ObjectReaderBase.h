// Dear emacs, this is -*-c++-*-
#ifndef CxAODReader_ObjectReaderBase_H
#define CxAODReader_ObjectReaderBase_H

#include <string>
#include "TTree.h"

class ObjectReaderBase {

public:

  /**
   * @brief Destructor.
   */
  virtual ~ObjectReaderBase() {;}

  /**
   * @brief Retrieve physics objects from event.
   * @return Status code.
   */
  virtual void discoverVariations(TTree* collectionTree) = 0;

  virtual std::vector<std::string> getVariations() = 0;
  virtual  std::string getContainerName() = 0;

  virtual void clearEvent() = 0;

};

#endif
