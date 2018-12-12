/**
 * \file CanvasDrawer.h
 * \brief Defines the CanvasDrawer class
 * \author S. Narayanan
 */

#ifndef PANDACORE_TOOLS_THINGDRAWER
#define PANDACORE_TOOLS_THINGDRAWER

#include "TTree.h"
#include "TChain.h"
#include "TString.h"
#include "vector"
#include "TROOT.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLatex.h"
#include "TLegend.h"
#include  <vector>
#include "TStyle.h"
#include "TColor.h"
#include "algorithm"
#include "PandaCore/Tools/interface/Common.h"


/**
 * \brief Enumerator of processes
 */
enum ProcessType {
  kData=0, /**< data */
  kSignal, /**< start of signals */
  kSignal1,
  kSignal2,
  kSignal3,
  kQCD,  /**< start of backgrounds */
  kTTbar,
  kWjets,
  kZjets,
  kST,
  kDiboson,
  kGjets,
  kExtra1,  /**< start of extra processes */
  kExtra2,   
  kExtra3,   
  kExtra4,   
  kExtra5,   
  kExtra6,   
  kExtra7,   
  kExtra8,   
  nProcesses /**< should never ever be used to label a process */
};

/**
 * \brief Wrapper class for TCanvas
 *
 * Base class for all other PandaCore::Drawer objects.
 * Essentially used as a wrapper for TCanvas, defining various
 * methods for formatting the owned canvas. An externally-constructed
 * canvas can also be provided.
 */
class CanvasDrawer
{
public:
  /**
   * \brief Constructor - creates new TCanvas
   * \param x x-width
   * \param y y-width
   */
  CanvasDrawer(double x=-1, double y=-1);
  CanvasDrawer(TCanvas *c0); //!< Constructor - given existing TCanvas
  virtual ~CanvasDrawer(); //!< Destructor

  void SetCanvas(TCanvas *c0); //!< Set externally-defined TCanvas
  void Reset(); //!< Reset canvas
  TCanvas *GetCanvas() { return c; } //!< Return canvas
  TPad *GetPad1() { return pad1; } //!< Return top plot pad (or NULL)
  TPad *GetPad2() { return pad2; } //!< Return bottom ratio pad (or NULL)

  void Stack(bool b)  {doStack = b;} //!< Set stack option for histograms
  void DrawEmpty(bool b)  {drawEmpty = b;} //!< Option to not fill histograms when drawing
  void StackSignal(bool b)  {doStackSignal = b; if (b) doStack=true;} //!< Option to stack signal with BGs
  void Logy(bool b=true)	{ doLogy = b; maxScale = (b) ? 100 : 2.; } //!< Set y log scale
  void SetNormFactor(bool b)  { doSetNormFactor = b;  } //!< Set histograms normalized to 1
  void SetMaxScale(double d) { maxScale = d; } //!< Set maximum scale
  void Ratio(bool b=true) { doRatio = b; } //!< Turn on ratio pad calculation (only implemented in certain inherited classes)
  void SplitCanvas(TCanvas *c_=0); //!< Split canvas into two pads
  void FixRatio(double m=2) { fixRatio=true; ratioMax=m; } //!< Fix range of ratio pad

  bool IsStack() { return doStack; }

  void SetLumi(float f) { lumi = f; } //!< Set luminosity label
  /**
   * \brief Draw a label on top of the plot
   * \param s the label
   * \param x the x-position
   * \param y the y-position
   * \param drawImmediately if true, then draw it now, else draw right before the canvas is printed to file
   * \param font font choice (default is Helvetica)
   * \param textSize text size, if not provided, default is used
   * \param textAlign alignment of text, if not provided, left-align is used
   */
  void AddPlotLabel(const char *s, double x, double y, bool drawImmediately=true, int font=42, float textSize=-1, int textAlign=11); 
  void SetSignalScale(double d) { signalScale = d; } //!< Scale the signal by a factor
  void InitLegend(double x0=0.6, double y0=0.55, double x1=0.88, double y1=0.9, int ncolumns=1); //!< Initialize a TLegend

  /**
   * \brief Draw the TCanvas and save it
   * \param outDir location of output files
   * \param baseName baseName.[pdf,png,C] are created
   */
  virtual void Draw(TString outDir,TString baseName); 
  void SetDrawOption(const char *s) { drawOption=s; } //!< Set draw option
  void DrawMCErrors(bool b) { doDrawMCErrors=b; } //!< Set MC errors
  void SetEvtNum(const char *s) { eventnumber=s; } //!< Set name for event number
  void SetEvtMod(int i) { eventmod=i; } //!< Set number of events to mod by

  void SetTDRStyle(TString opt="default"); //!< Set some plotting options to be CMS-friendly
  void SetAutoRange(bool b) { doAutoRange=b; } //!< Automatically calculate ranges as plotting
  void SetRatioStyle(); //!< Some predefined options nice for drawing some ratio plots
  void AddCMSLabel(double x=0.18, double y=0.85, TString subhead=""); //!< Add a CMS label
  void AddLumiLabel(bool fb=true, double customLumi=-1); //!< Add a luminosity label
  void AddSqrtSLabel(); //!< Only add 13 TeV label
  void ClearCanvas() { c->Clear(); } //!< Clear
  void cd() { c->cd(); } //!< cd
  void SetGrid() { gStyle->SetGridColor(16); c->SetGrid(); } //!< Set a grid
  void SetRatioLabel(TString s) { ratioLabel = s; }
  void SetLineWidth(int i) { emptyLineWidth = i; }

  bool HasLegend() { return legend!=0; }
  void ClearLegend() { if (legend) legend->Clear(); }
  TLegend* GetLegend() { return legend; }
  /**
   * \brief Adds a drawable TObject to be drawn after all histograms are drawn 
   * \param o a TObject to draw
   * \param opt option to use when drawing
   * \param aname if provided and legend is created, this label is used in the legend
   */
  void AddAdditional(TObject *o, TString opt="", TString aname="");

protected:
  /**
   * \brief Internal class for managing TObjects and their properties
   */
  struct ObjWrapper {
      TObject *o;
      TString label;
      TString opt;
  };
  /**
   * \brief Internal class used for managing labels 
   */
  class Label {
    public:
      /**
       * \param n label text
       * \param x0 x-coordinate
       * \param y0 y-coordinate
       * \param f0 ROOT font index
       * \param s0 text size
       * \param a0 ROOT text align index
       */
      Label(const char *n, double x0, double y0, int f0=42, float s0=-1, int a0=11):
        name(n),
        x(x0),
        y(y0),
        font(f0),
        align(a0),
        size(s0)
      {  }
      ~Label() { };
    TString name;
    double x;
    double y;
    int font,align;
    float size;
  };

  /**
   * \brief Colors of histograms
   */
  int PlotColors[20] = {1,
                        kBlack,
                        kBlue,
                        kRed,
                        kGreen+3,
                        kGray,
                        kOrange-3,
                        kGreen-6,
                        kAzure+5,
                        kRed-5,
                        kViolet-8,
                        kCyan-2,
                        kMagenta-3,
                        kGreen-2,
                        kCyan+3,
                        kOrange+8,
                        kBlue+2,
                        kViolet,
                        kAzure,
                        kSpring+8};

  int VBFColors[20] = {1,
                      kBlack, // signal
                      kBlue, // signal1
                      kRed, // signal2
                      kGreen+3, // signal3
                      TColor::GetColor("#F1F1F2"),  // qcd
                      TColor::GetColor("#CF3721"), //ttbar
                      TColor::GetColor("#FAAF08"), // w
                      TColor::GetColor("#4D975D"), // z(vv)
                      kRed-5, // st
                      TColor::GetColor("#4897D8"), // dibos
                      kCyan-2, // gjets
                      TColor::GetColor("#9A9EAB"), // zll
                      kGreen-2, // wewk
                      kCyan+3, // zewk
                      kOrange+8,
                      kBlue+2,
                      kViolet,
                      kAzure,
                      kSpring+8};
  /**
   * \brief Colors of graphs
   */
  int GraphColors[18] = {
                        1, //black
                        2,  //red
                        4,  //purple
                        kAzure+3,  
                        kGreen+2,
                        kBlue-3,
                        6,  //magenta
                        kRed-7,
                        kMagenta+1,
                        kViolet,
                        kCyan+1,
                        kOrange+8,
                        kTeal+10,
                        kOrange+10,
                        kAzure,
                        kRed+3,
                        kMagenta+3,
                        kCyan+3
  };

  TCanvas *c=0;                         //!< pointer to canvas
  TPad *pad1=0, *pad2=0;                //!< pointers to TPads
  bool canvasIsOwned=false;             //!< is canvas owned?
  bool doStack=false;                   //!< option to stack everything but data
  bool doStackSignal=false;             //!< option to stack signal as well
  bool doLogy=false;                    //!< do y-axes log
  bool doDrawMCErrors=false;            //!< option to draw MC errors
  bool doSetNormFactor=false;           //!< option to scale mc to match data (if doStack) else SetNormFactor() everything 
  bool doAutoRange=true;                //!< automatically calculate ranges
  bool doRatio=false;                   //!< draw a ratio pad
  bool fixRatio=false;                  //!< fix the ratio pad size to ratioMax
  double ratioMax=2;                    //!< maximum size of ratio pad
  TCut mcWeight;                        //!< mc weight
  float lumi=-1;                        //!< luminosity for label
  TLatex *label=0;                      //!< label
  TLegend *legend=0;                    //!< legend
  double signalScale=1;                 //!< factor by which to scale the signal
  TString drawOption="hist";            //!< default draw option
  std::vector<Label> plotLabels;        //!< labels to be plotted right before saving the canvas
  TString eventnumber="eventNumber";    //!< name of event branch
  int eventmod=0;                       //!< event mod value
  int whichstyle=0;                     //!< for different styles of plotting
  double maxScale=1.5;                  //!< factor by which to scale the y-axis relative to the largest histogram
  bool drawEmpty=false;                 //!< for stacks, do not fill in the histograms
  int emptyLineWidth=3;                 //!< line width if not stacked

  int *Colors = NULL; // !< colors used for plotting
  std::vector<ObjWrapper> internalAdds; //!< collection of TObjects to plot
  bool addsDrawn = false;
  TString ratioLabel="Data/Pred."; //!< y-axis label for ratio pad
};
#endif
