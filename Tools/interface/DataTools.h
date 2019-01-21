
#ifndef DATAUTILITIES_H
#define DATAUTILITIES_H

#include "PandaCore/Tools/interface/Common.h"
#include "TString.h"
#include "string.h"

/** \file DataTools.h
 * \brief Tools used to deal with CMS detector metadata
 * \author S. Narayanan
 *
 */


//////////////////////////////////////////////////////////////////////////////////

bool matchTriggerName(std::string pattern, std::string name);

class EraHandler {
  public:
    /**
     * \param year year of data-taking
     * \brief constructor
     */
    EraHandler(int year=2016);
    ~EraHandler() { delete bins; }
    TString getEra(int runNumber);
  private:
    std::vector<double> runBounds;
    std::vector<TString> eraNames;
    Binner *bins=0;
    bool hadError=false;
};

#endif
