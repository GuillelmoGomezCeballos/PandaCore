#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "TLorentzVector.h" 
#include "TMath.h"

#define PI 3.141592654

/** \file Functions.h
 * \brief Some numerical functions
 * \author S. Narayanan
 */

//////////////////////////////////////////////////////////////////////////////////

/**
 * \param x	value to be cleaned
 * \param d	default value
 * \brief Returns a default value if input is NaN
 */
double clean(double x, double d=-1);

/**
 * \param val	input value
 * \param low	low value
 * \param high	high value
 * \brief Bounds a value within a range
 */
double bound(double val, double low, double high);

template <typename T>
int sign(T x) {
    return (x<0) ? -1 : 1;
}

// need a non-templated instance to be able to 
// load in interactive ROOT => use in TTreeFormula
int dsign(double x);

/**
 * \brief Mass of a pair of particles
 */
double Mxx(double pt1, double eta1, double phi1, double m1, double pt2, double eta2, double phi2, double m2);

double MT(double pt1, double phi1, double pt2, double phi2);

/**
 * \brief Signed delta-phi
 */
double SignedDeltaPhi(double phi1, double phi2);

/**
 * \brief Calculates the delta-R-squared metric
 */
double DeltaR2(double eta1, double phi1, double eta2, double phi2);

/**
 * \brief Sphericity
 */
float sphericity(double r,  std::vector<TLorentzVector> & inputVectors_);

/**
 * \brief Exponential times erf, aka CMSShape
 */
double ExpErf(double x, double a, double b, double c);

/**
 * \brief in-place insertion sort
 */
template <typename T>
void insertion_sort(std::vector<T>& v)
{
  int n = v.size();
  for (int i = 1; i != n; ++i) {
    T pivot(v[i]); // current element is the pivot, make a copy
    int j = i - 1;
    while (j != -1 && pivot < v[j]) { // starting at pivot and going backwards
      v[j + 1] = v[j]; // if v[j] is less than pivot, move it up one index
      --j;
    }
    v[j + 1] = pivot; // end up here either because j = -1 (pivot is smallest element) or
                      // because we found a j such that v[j]<pivot, in which case v[j+1] should be pivot
  }
}

#endif
