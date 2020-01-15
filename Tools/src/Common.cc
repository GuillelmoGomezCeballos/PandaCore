#include "../interface/Common.h"

template <typename T>
void concat(std::vector<T> &v1, const std::vector<T>& v2) {
  v1.insert(v1.end(),v2.begin(),v2.end());
}

void activateBranch(TTree *t, const char *bname, void *address) {
  t->SetBranchStatus(bname,1);
  t->SetBranchAddress(bname,address);
}

double getVal(TH1*h,double val) {
  return h->GetBinContent(h->FindBin(val));
}

double getVal(TH2D*h,double val1, double val2) {
  return h->GetBinContent(h->FindBin(val1,val2));
}

double getError(TH1*h,double val) {
  return h->GetBinError(h->FindBin(val));
}

double getError(TH2D*h,double val1, double val2) {
  return h->GetBinError(h->FindBin(val1,val2));
}

std::vector<TString> getDependencies(TString cut) {
  std::vector<TString> deps;
  int nChars = cut.Length();
  TString tmpString="";
  for (int iC=0; iC!=nChars; ++iC) {
    const char c = cut[iC];
    if ( c==' ' || c=='&' || c=='|' || c=='(' || c==')'
        || c=='*' || c=='+' || c=='-' || c=='/' || c=='!'
        || c=='<' || c=='>' || c=='=' || c=='.' || c==','
        || c=='[' || c==']' || c==':') {
      if (tmpString != "" && !tmpString.IsDigit() &&
          // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
          !(tmpString.Contains("TMath")) && !(tmpString=="fabs")) {
        deps.push_back(tmpString);
      }
      tmpString = "";
    } else {
        tmpString.Append(c);
    }
  }
  if (tmpString != "" && !tmpString.IsDigit() &&
      // tmpString!="Pt" && tmpString!="Eta" && tmpString!="Phi" &&
      !tmpString.Contains("TMath")) {
    deps.push_back(tmpString);
  }
  return deps;
}

ProgressReporter::ProgressReporter(const char *n, 
                                   const unsigned int *iE, 
                                   const unsigned int *nE, 
                                   unsigned int nR) : 
  idx(iE),
  N(nE),
  frequency(nR),
  name(n),
  newline(isatty(fileno(stdout)) ? "\r" : "\n")
{
  name = n; name+="::Progress";
}

void ProgressReporter::Report() 
{
  if (*idx == 0) 
    globalStart = static_cast<long>(gSystem->Now());
  float progress = 1.*(*idx)/(*N);
  if (progress >= threshold) {
    float timeLeft = progress > 0 ?
                      (static_cast<long>(gSystem->Now()) - globalStart) * (1 - progress) / (1000 * progress) : 
                      -1; 
    logger.info(name.Data(),
          TString::Format("%-40s",
              TString::Format("%5.2f%%, %8.2fs left (%u/%u) ",progress*100,timeLeft,*idx,*N).Data()
            ).Data(),
          newline);
    threshold += 1./frequency;
  }
}

void ProgressReporter::Done()
{
  logger.info(name.Data(),
      TString::Format("%-50s",
        TString::Format("%u steps completed in %.2fs", 
                        *N, (static_cast<long>(gSystem->Now()) - globalStart) * 0.001).Data()
        ).Data(),
      "\n");
}

TimeReporter::TimeReporter(TString n, int on_)
{
  on=on_;
  name = n; name += "::Time";
  sw = new TStopwatch();
  subsw = new TStopwatch();
}

TimeReporter::~TimeReporter() { delete sw; delete subsw; }

void
TimeReporter::Start()
{
  if (on) {
    sw->Start(true);
    subsw->Start(true);
  }
  currentEvent=0;
  currentSubEvent=1;
  globalStart = static_cast<long>(gSystem->Now()); 
}

void
TimeReporter::TriggerEvent(TString s,bool reset)
{
  if (!on)
    return;
  currentSubEvent=1;
  double interval = sw->RealTime()*1000;
  if (on > 1)
    logger.debug(name,TString::Format("%2i   : %.3f (%s)",currentEvent,interval,s.Data()).Data());
  if (s.Contains("GetEntry"))
    s = "GetEntry";
  if (totalTime.find(s) == totalTime.end()) {
    totalTime[s] = 0;
    nCalls[s] = 0;
    callOrders.push_back(s);
  } else { // always ignore the first call, frequently initialization is slow
    nCalls[s] += 1;
    totalTime[s] += interval;
  }
  sw->Start();
  subsw->Start();
  if (reset)
    currentEvent+=1;
}

void
TimeReporter::TriggerSubEvent(TString s)
{
  if (!on)
    return;
  double interval = subsw->RealTime()*1000;
  if (on > 2)
    logger.debug(name,
        TString::Format("%2i.%-2i: %.3f (%s)",currentEvent,currentSubEvent,interval,s.Data()).Data());
  if (totalTime.find(s) == totalTime.end()) {
    totalTime[s] = 0;
    nCalls[s] = 0;
    callOrders.push_back(s);
  } else {
    nCalls[s] += 1;
    totalTime[s] += interval;
  }
  currentSubEvent+=1;
  subsw->Start();
}

void
TimeReporter::Summary()
{
  if (!on)
    return;
  for (TString s : callOrders) {
    logger.debug(name,
        TString::Format("Task %20s called %5u times, average time = %6.3f us, total time = %.3f ms",
          s.Data(), nCalls[s], totalTime[s]/nCalls[s]*1000, totalTime[s]));
  }
  logger.debug(name,
      TString::Format("TOTAL TIME = %.3f s", (static_cast<long>(gSystem->Now()) - globalStart) * 0.001));
}
