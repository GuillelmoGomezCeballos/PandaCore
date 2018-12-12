import ROOT as root

# kill interactive options
root.PyConfig.IgnoreCommandLineOptions = True 
root.gROOT.SetBatch(1)

# set plotting defaults 
root.gStyle.SetOptStat(0)
root.gStyle.SetNumberContours(99)
root.gStyle.SetPalette(root.kBird)
