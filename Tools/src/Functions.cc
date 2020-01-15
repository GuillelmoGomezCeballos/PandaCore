#include "../interface/Functions.h"
#include "TMath.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"

TMatrixDSym compMomentumTensor(double r, std::vector<TLorentzVector> & inputVectors_)
{
  TMatrixDSym momentumTensor(3);
  momentumTensor.Zero();

  if ( inputVectors_.size() < 2 ){
    return momentumTensor;
  }

  // fill momentumTensor from inputVectors
  double norm = 1.;
  for ( int i = 0; i < (int)inputVectors_.size(); ++i ){
    double p2 = inputVectors_[i].Dot(inputVectors_[i]);
    double pR = ( r == 2. ) ? p2 : TMath::Power(p2, 0.5*r);
    norm += pR;
    double pRminus2 = ( r == 2. ) ? 1. : TMath::Power(p2, 0.5*r - 1.);
    momentumTensor(0,0) += pRminus2*inputVectors_[i].X()*inputVectors_[i].X();
    momentumTensor(0,1) += pRminus2*inputVectors_[i].X()*inputVectors_[i].Y();
    momentumTensor(0,2) += pRminus2*inputVectors_[i].X()*inputVectors_[i].Z();
    momentumTensor(1,0) += pRminus2*inputVectors_[i].Y()*inputVectors_[i].X();
    momentumTensor(1,1) += pRminus2*inputVectors_[i].Y()*inputVectors_[i].Y();
    momentumTensor(1,2) += pRminus2*inputVectors_[i].Y()*inputVectors_[i].Z();
    momentumTensor(2,0) += pRminus2*inputVectors_[i].Z()*inputVectors_[i].X();
    momentumTensor(2,1) += pRminus2*inputVectors_[i].Z()*inputVectors_[i].Y();
    momentumTensor(2,2) += pRminus2*inputVectors_[i].Z()*inputVectors_[i].Z();
  }


  // return momentumTensor normalized to determinant 1
  return (1./norm)*momentumTensor;
}

/// helper function to fill the 3 dimensional vector of eigen-values;
/// the largest (smallest) eigen-value is stored at index position 0 (2)
TVectorD compEigenValues(double r, std::vector<TLorentzVector> & inputVectors_)
{
  TVectorD eigenValues(3);
  TMatrixDSym myTensor = compMomentumTensor(r, inputVectors_);
  if( myTensor.IsSymmetric() ){
    if( myTensor.NonZeros() != 0 ) myTensor.EigenVectors(eigenValues);
  }

  // CV: TMatrixDSym::EigenVectors returns eigen-values and eigen-vectors
  //     ordered by descending eigen-values, so no need to do any sorting here...
  //std::cout << "eigenValues(0) = " << eigenValues(0) << ","
  //      << " eigenValues(1) = " << eigenValues(1) << ","
  //      << " eigenValues(2) = " << eigenValues(2) << std::endl;

  return eigenValues;
}

/// 1.5*(q1+q2) where 0<=q1<=q2<=q3 are the eigenvalues of the momentum tensor sum{p_j[a]*p_j[b]}/sum{p_j**2} 
/// normalized to 1. Return values are 1 for spherical, 3/4 for plane and 0 for linear events


float sphericity(double r,  std::vector<TLorentzVector> & inputVectors_)
{
  TVectorD eigenValues = compEigenValues(r, inputVectors_);
  double tmp1 = eigenValues(1);
  double tmp2 = eigenValues(2);
  return float(1.5*(eigenValues(1) + eigenValues(2)));
}


double clean(double x, double d) {
  return TMath::Finite(x) ? x : d;
}

double bound(double val, double low, double high) {
  return TMath::Max(low,TMath::Min(high,val));
}

int dsign(double x) {
    return sign(x);
}

double Mxx(double pt1, double eta1, double phi1, double m1, double pt2, double eta2, double phi2, double m2) {
    TLorentzVector v1,v2;
    v1.SetPtEtaPhiM(pt1,eta1,phi1,m1);
    v2.SetPtEtaPhiM(pt2,eta2,phi2,m2);
    return (v1+v2).M();
}

double MT(double pt1, double phi1, double pt2, double phi2)
{
    TLorentzVector v1,v2;
    v1.SetPtEtaPhiM(pt1,0,phi1,0);
    v2.SetPtEtaPhiM(pt2,0,phi2,0);
    return (v1+v2).M();
}

double SignedDeltaPhi(double phi1, double phi2) {
    double dPhi = phi1-phi2;
    if (dPhi<-PI)
        dPhi = 2*PI+dPhi;
    else if (dPhi>PI)
        dPhi = -2*PI+dPhi;
    return dPhi;
}

double DeltaR2(double eta1, double phi1, double eta2, double phi2) {
    float dEta2 = (eta1-eta2); dEta2 *= dEta2;
    float dPhi = SignedDeltaPhi(phi1,phi2);
    return dEta2 + dPhi*dPhi;
}

double ExpErf(double x, double a, double b, double c) {
    double exp_ = TMath::Exp(c*x);
    double erf_ = TMath::Erf((x-a)/b);
    return exp_*(1+erf_)/2;
}


