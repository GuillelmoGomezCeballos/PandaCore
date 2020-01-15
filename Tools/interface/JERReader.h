#ifndef PANDACORE_TOOLS_JERReader
#define PANDACORE_TOOLS_JERReader

#include "PandaCore/Tools/interface/Common.h"
#include "TString.h"
#include "TF1.h"
#include "TRandom3.h"
#include "vector"

/**
 * \brief Reads a SF and PtResolution file and provides smearing factors 
 *
 */
class JERReader
{
public:
	enum SmearShift { 
									 kNominal,
									 kUp,
									 kDown
									};
	JERReader(TString sfPath, TString resPath, int year);
	~JERReader() { delete bins_sfEta; delete bins_resEta; delete bins_resRho; }

	void getStochasticSmear(double pt, double eta, double rho, double &smear, double &smearUp, double &smearDown, bool usegen, double genpt, int year);

	// TODO implement	gen-based smearing
private:
	int n_sfEta, n_sfPt;
	Binner *bins_sfEta=0;
	Binner2D *bins_sfEtaPt=0;
	int n_resEta, n_resRho;
	Binner *bins_resEta=0, *bins_resRho=0;

	std::vector<float> sf_central, sf_up, sf_down;
	std::vector<float> res_ptLo, res_ptHi; 
	std::vector<float> res_param0, res_param1, res_param2, res_param3;

	TF1 fres = TF1("fres","sqrt([0]*abs([0])/(x*x)+[1]*[1]*pow(x,[3])+[2]*[2])",0,13000);
	TRandom3 rng = TRandom3();
	
};
#endif
