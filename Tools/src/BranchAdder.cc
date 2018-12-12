#include "../interface/BranchAdder.h"


void BranchAdder::doAddBranch(TTree *t)
{
  unsigned nE = t->GetEntriesFast();
  unsigned iE = 0; 

  turnOnBranches(t, cut);

  ProgressReporter pr("BranchAdder", &iE, &nE, reportFreq);

  float newBranchVal{0};

  TBranch *b = t->Branch(newBranchName.Data(), 
                         &newBranchVal,
                         Form("%s/F",newBranchName.Data()));

  TTreeFormula tcut("cut", cut.Data(), t);
  tcut.SetQuickLoad(true);

  for (iE = 0; iE != nE; ++iE) {
    if (verbose)
      pr.Report();

    t->GetEntry(iE);
    if (tcut.EvalInstance()) {
      newBranchVal = getValue();
    } else {
      newBranchVal = defaultValue;
    }
    b->Fill();
  }
  pr.Done();

  t->SetBranchStatus("*", 1);
}

void FormulaBranchAdder::addBranch(TTree *t)
{
  t->SetBranchStatus("*",0);
  turnOnBranches(t, formula);

  tf = new TTreeFormula(newBranchName.Data(), formula.Data(), t);
  tf->SetQuickLoad(true);

  doAddBranch(t);

  delete tf; tf = nullptr;
}

float FormulaBranchAdder::getValue()
{
  return tf->EvalInstance();
}


template <typename H>
void HBranchAdder<H>::addBranch(TTree *t)
{
  t->SetBranchStatus("*",0);
  turnOnBranches(t, formulaX);
  turnOnBranches(t, formulaY);

  tfX = new TTreeFormula(newBranchName.Data(), formulaX.Data(), t);
  tfX->SetQuickLoad(true);
  tfY = new TTreeFormula(newBranchName.Data(), formulaY.Data(), t);
  tfY->SetQuickLoad(true);

  doAddBranch(t);

  delete tfX; tfX = nullptr;
  delete tfY; tfY = nullptr;
}

template <>
void HBranchAdder<TH1D>::setH(const TH1* h_)
{
  h_->Copy(h);
  xlo = h.GetXaxis()->GetBinCenter(1);
  xhi = h.GetXaxis()->GetBinCenter(h.GetNbinsX());
}

template <>
float HBranchAdder<TH1D>::getValue()
{
  float x = tfX->EvalInstance();
  x = bound(x, xlo, xhi);

  return h.GetBinContent(h.FindBin(x));
} 

template <>
void HBranchAdder<TH2D>::setH(const TH1* h_)
{
  auto* h2_ = dynamic_cast<const TH2*>(h_);
  if (h2_ == nullptr) {
    logger.error("H2BranchAdder::setH", "Histogram provided was not a TH2!");
    exit(1);
  }

  h2_->Copy(h);
  xlo = h.GetXaxis()->GetBinCenter(1);
  xhi = h.GetXaxis()->GetBinCenter(h.GetNbinsX());
  ylo = h.GetYaxis()->GetBinCenter(1);
  yhi = h.GetYaxis()->GetBinCenter(h.GetNbinsY());
}

template <>
float HBranchAdder<TH2D>::getValue()
{
  float x = tfX->EvalInstance();
  x = bound(x, xlo, xhi);
  float y = tfY->EvalInstance();
  y = bound(y, ylo, yhi);

  return h.GetBinContent(h.FindBin(x,y));
} 

