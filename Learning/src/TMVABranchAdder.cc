#include "../interface/TMVABranchAdder.h"
#include "TMVA/Config.h"


void TMVABranchAdder::AddVariable(TString vname, TString formula) {
  xvar *x = new xvar(vname,formula);
  variables.push_back(x);
  reader->AddVariable(x->name,x->val);
}

void TMVABranchAdder::AddFormula(TString vname, TString formula) {
  xvar *x = new xvar(vname,formula);
  formulae.push_back(x);
  reader->AddVariable(x->name,x->val);
}

void TMVABranchAdder::AddSpectator(TString vname) {
  reader->AddSpectator(vname,&spectator);
}

void TMVABranchAdder::BookMVA(TString branchname, TString fpath) {
  reader->BookMVA(branchname,fpath);
  bnames.push_back(branchname);
}

void TMVABranchAdder::RunFile(TString fpath) {
  TFile *fin = TFile::Open(fpath,"UPDATE");
  TTree *tin = (TTree*)fin->FindObjectAny(treename);

  TBranch **newbranches = new TBranch*[bnames.size()];
  float *responses = new float[bnames.size()];
  unsigned int nBranches = bnames.size();
  for (unsigned int iB=0; iB!=nBranches; ++iB) {
    responses[iB]=0;
    newbranches[iB] = tin->Branch(bnames[iB],&(responses[iB]),bnames[iB]+"/F");
  }

  for (auto *x : variables) {
    tin->SetBranchAddress(x->formula,x->val);
  }

  std::vector<TTreeFormula*> treeformulae;
  for (auto *x : formulae) {
    TTreeFormula *tf = new TTreeFormula(x->name.Data(),x->formula.Data(),tin);
    tf->SetQuickLoad(true);
    treeformulae.push_back(tf);
  }

  unsigned int nFormulae = formulae.size();
  unsigned int nEntries = tin->GetEntries();
  unsigned int nMVA = bnames.size();

  unsigned int iE=0;
  ProgressReporter pr("TMVABranchAdder::RunFile",&iE,&nEntries,100); 

  for (iE=0; iE!=nEntries; ++iE) {
    pr.Report();
    tin->GetEntry(iE);
    for (unsigned int iF=0; iF!=nFormulae; ++iF) {
      *(formulae[iF]->val) = treeformulae[iF]->EvalInstance();
    }

    for (unsigned int iM=0; iM!=nMVA; ++iM) {
      TString bname = bnames[iM];
      responses[iM] = reader->EvaluateMVA(bname);
      newbranches[iM]->Fill();
    }
  }

  fin->WriteTObject(tin,treename,"Overwrite");
  fin->Close();

  for (auto tf : treeformulae)
    delete tf;
  delete newbranches;
  delete responses;
}
