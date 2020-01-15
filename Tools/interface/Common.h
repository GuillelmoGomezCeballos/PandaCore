#include "TH1F.h"
#include "TTree.h"
#include "TH2F.h"
#include "TString.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include <vector>
#include <unistd.h>
#include "Functions.h"
#include "PandaCore/Utils/interface/Logger.h"

#ifndef COMMONUTILITIES_H
#define COMMONUTILITIES_H

#define PI 3.141592654

using namespace std; 

/** \file PandaCore/Tools/interface/Common.h
 * \brief Common function header file
 * \author S. Narayanan
 *
 * Just some stuff that are used all over the place
 */


//////////////////////////////////////////////////////////////////////////////////

extern Logger logger;

/**
 * \param v1  reference to vector that is lengthened
 * \param v2  vector to add to v1
 * \brief Function to concatenate two vectors
 */
template <typename T>
void concat(std::vector<T> &v1, const std::vector<T> &v2);

// /**
//  * \param t  input tree
//  * \param bname  name of branch
//  * \param address  address of branch
//  * \brief Turns on a branch in a TTree and sets the address to a specified location
//  */
// void activateBranch(TTree *t, const char *bname, void *address);
//
// /**
//  * \param module  name of caller
//  * \param module  msg message to be printed
//  * \param newline  newline to use at end of message
//  * \brief Prints to info stdout
//  */
// void logger.info(const char *module, const char *msg, const char *newline="\n");
//
// /**
//  * \param module  name of caller
//  * \param module  msg message to be printed
//  * \param newline  newline to use at end of message
//  * \brief Prints debug to  stderr
//  */
// void logger.debug(const char *module, const char *msg, const char *newline="\n");
//
// /**
//  * \param module  name of caller
//  * \param module  msg message to be printed
//  * \param newline  newline to use at end of message
//  * \brief Prints warning to  stderr
//  */
// void PWarning(const char *module, const char *msg, const char *newline="\n");
//
// /**
//  * \param module  name of caller
//  * \param module  msg message to be printed
//  * \param newline  newline to use at end of message
//  * \brief Prints error to  stderr
//  */
// void PError(const char *module, const char *msg, const char *newline="\n");

/**
 * \param h  histogram to read
 * \param val  x-value
 * \brief Returns the value/error of a 1D histogram given an x-val
 */
double getVal(TH1*h,double val);

/**
 * \param h  histogram to read
 * \param val  x-value
 * \brief Returns the value/error of a 1D histogram given an x-val
 */
double getError(TH1*h,double val);

/**
 * \param h  histogram to read
 * \param val1  x-value
 * \param val2  y-value
 * \brief Returns the value/error of a 2D histogram given an x-val,y-val
 */
double getVal(TH2D*h,double val1, double val2);
double getError(TH2D*h,double val1, double val2);

/**
 * \param cut  string to parse
 * \brief Finds all substrings in a string that might be branch dependencies
 */
std::vector<TString> getDependencies(TString cut);

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Reports progress of some process
 *
 * Given a total length of a task and an integer to track the progress,
 * this class prints the progress at regular intervals
 */
class ProgressReporter {
  public:
    /**
     * \param n  name of this task
     * \param iE  pointer to progress tracker
     * \param nE  pointer to total length of task
     * \param nR  number of reports desired
     * \brief Constructor
     */
    ProgressReporter(const char *n, const unsigned int *iE, const unsigned int *nE, unsigned int nR=100);
    /**
     * \brief Destrucctor
     */
    ~ProgressReporter() { }
    /**
     * \brief Reports if threshold has been reached
     */
    void Report();
    void Done();
  private:
    const unsigned int *idx, *N; /**< addresses of progress trackers */
    unsigned int frequency; /**< how often to print */
    TString name; /**< name of this task */
    float threshold=0; /**< internal tracker of next threshold */
    long globalStart; 
    TString newline;
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Keeps track of events and real time
 *
 * Reports the occurence of a user-specified event
 * and the amount of time that has passed between
 * consecutive events. Sub-events can also be tracked,
 * which do not reset the event stopwatch
 */
class TimeReporter {
  public:
    /**
     * \param n  name of the calling object
     * \param on_  whether this reporter is on
     * \brief Constructor
     */
    TimeReporter(TString n, int on_) ;
    /**
     * \brief Destructor
     */
    ~TimeReporter();
    /**
     * \brief Starts all stopwatches and resets counters
     */
    void Start();
    /**
     * \param s  name of event
     * \param reset  whether to reset the stopwatch for the next event
     * \brief Triggers a top-level event
     */
    void TriggerEvent(TString s,bool reset=true);
    /**
     * \param s  name of sub-event
     * \brief Triggers a sub-level event
     */
    void TriggerSubEvent(TString s);
    /**
     * \brief Prints a summary of timing to screen
     */
    void Summary();
  private:
    TString name; /**< name of this task */
    int on; /**< whether this reporter is active */
    TStopwatch *sw, *subsw; /**< internal stopwatches */
    std::map<TString, double> totalTime; /**< total time spent on each task */
    std::map<TString, unsigned> nCalls; /**< number of calls to each task */
    std::vector<TString> callOrders; /**< order in which tasks are called */
    int currentEvent=0; /**< internal tracker of the index of the current event */
    int currentSubEvent=1; /**< internal tracker of the index of the sub-event */
    long globalStart=0;
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Bins a continuous variable
 *
 */
class Binner {
  public:
    /**
     * \param bounds_  boundaries of the variable
     * \brief Constructor
     */
    Binner(std::vector<double> bounds_) {
      bounds = bounds_;
      nB = bounds.size();
    }
    /**
     * \brief Destructor
     */
    ~Binner() {}
    /**
     * \param x  value to bin
     * \brief Returns a bin
     */
    unsigned int bin(double x) {
      if (x<bounds.at(0))
        return 0;

      for (unsigned int iB=1; iB!=nB; ++iB) {
        if (x<bounds.at(iB))
          return iB-1;
      }

      return nB-2; // return top bin if out of bounds
    }
    /**
     * \brief Returns max boundary
     */
    double getMax() { return bounds.at(nB-1); }
    /**
     * \brief Returns min boundary
     */
    double getMin() { return bounds.at(0);    }
  private:
    std::vector<double> bounds; /**< internal boundary structure */
    unsigned int nB=0; /**< number of bins */
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Bins a continuous variable
 *
 */
class Binner2D {
  public:
    /**
     * \param bounds_  boundaries of the variable
     * \brief Constructor
     */
    Binner2D(std::vector<double> boundsXLow_,std::vector<double> boundsXHigh_,std::vector<double> boundsYLow_,std::vector<double> boundsYHigh_) {
      boundsXLow = boundsXLow_;
      boundsXHigh = boundsXHigh_;
      boundsYLow = boundsYLow_;
      boundsYHigh = boundsYHigh_;
      nBX = boundsXLow.size();
      nBY = boundsYLow.size();
    }
    /**
     * \brief Destructor
     */
    ~Binner2D() {}
    /**
     * \param x  value to bin
     * \brief Returns a bin
     */
    unsigned int bin(double x, double y) {
      // x and y outside of boundaries?
      if (x<*min_element(boundsXLow.begin(),boundsXLow.end())){
	x = *min_element(boundsXLow.begin(),boundsXLow.end())+1e-5;
      }
      if (x>*max_element(boundsXHigh.begin(),boundsXHigh.end())){
	x = *max_element(boundsXHigh.begin(),boundsXHigh.end())-1e-5;
      }
      if (y<*min_element(boundsYLow.begin(),boundsYLow.end())){
	y = *min_element(boundsYLow.begin(),boundsYLow.end())+1e-5;
      }
      if (y>*max_element(boundsYHigh.begin(),boundsYHigh.end())){
	y = *max_element(boundsYHigh.begin(),boundsYHigh.end())-1e-5;
      }

      for (unsigned int iBX=0; iBX<nBX; iBX++){
	if (x<boundsXHigh.at(iBX) && x>boundsXLow.at(iBX) && y<boundsYHigh.at(iBX) && y>boundsYLow.at(iBX))
	  return iBX;
      }
      return 0;
    }
    /**
     * \brief Returns max boundary
     */
    double getMax() { return boundsX.at(nBX-1); }
    /**
     * \brief Returns min boundary
     */
    double getMin() { return boundsX.at(0);    }
  private:
    std::vector<double> boundsX,boundsY,boundsXLow,boundsYLow,boundsXHigh,boundsYHigh; /**< internal boundary structure */
    unsigned int nBX=0,nBY=0; /**< number of bins */
};

//////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Wrapper for run,lumi,event
 *
 * Simple struct containing run,lumi,event.
 * Hashing and == are implemented for use in
 * ordered and unordered containers.
 */
struct EventObj {
  int run, lumi;
  ULong64_t evt;
  bool operator==(const EventObj &other) const {
    return ( run==other.run && lumi==other.lumi && evt==other.evt);
  }
};

namespace std{
  template<> struct hash<EventObj> {
    std::size_t operator()(const EventObj &e) const {
      using std::size_t;
      using std::hash;
      return  ( ( ( hash<int>()(e.run) ^ (hash<int>()(e.lumi) << 1) ) >> 1 ) ^ (hash<ULong64_t>()(e.evt) << 1) );
    }
  };
}

//////////////////////////////////////////////////////////////////////////////////

#endif
