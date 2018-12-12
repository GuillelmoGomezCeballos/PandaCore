#!/usr/bin/env python

import ROOT as root
import numpy as np
from array import array
from PandaCore.Tools.Misc import *
from PandaCore.Utils.load import Load
from PandaCore.Tools.root_interface import read_files, draw_hist
from os import getenv, system, path
from pprint import pprint
from multiprocessing.pool import ThreadPool

Load('HistogramDrawer')

tree_name = 'events'

class Process(object):
    def __init__(self, name, pt, custom_color=root.nProcesses):
        self.name = name
        self.process_type = pt
        if (custom_color==root.nProcesses):
            self.color = pt
        else:
            self.color = custom_color
        # public config - defaults
        self.dashed = False
        self.dotted = False
        self.use_common_weight = True 
        self.use_common_cut = True
        self.additional_cut = '1==1'
        self.additional_weight = '1'
        self.tree_name = tree_name
        self.ratio = False
        # private stuff
        self.files = []
        return
    def add_file(self, fpath):
        self.files.append(fpath)
    def read(self, variables, weights, cut, files=None):
        if files is None:
            files = self.files
        return read_files(filenames = files, 
                          branches = variables+weights, 
                          cut = cut, 
                          treename = self.tree_name)

def convert_name(n):
    rn = str(n)
    rn = rn.replace("/", "Over");    
    rn = rn.replace("*", "Times");   
    rn = rn.replace("+", "Plus");    
    rn = rn.replace("-", "Minus");   
    rn = rn.replace("TMath::", "");  
    rn = rn.replace(")", "");        
    rn = rn.replace("(", "");        
    rn = rn.replace(".", "_");       
    rn = rn.replace('[', '_')
    rn = rn.replace(']', '_')
    return rn


def divide_bin_width(h):
    nb = h.GetNbinsX()
    for ib in xrange(1, nb+1):
        val = h.GetBinContent(ib)
        err = h.GetBinError(ib)
        width = h.GetBinWidth(ib)
        h.SetBinContent(ib, val/width)
        h.SetBinError(ib, err/width)


def fix_underflow(h):
    h.SetBinContent(1, h.GetBinContent(0)+h.GetBinContent(1))

def fix_overflow(h):
    nbins = h.GetNbinsX()
    h.SetBinContent(nbins, h.GetBinContent(nbins+1)+h.GetBinContent(nbins))


class Distribution(object):
    def __init__(self, formula, xlabel, ylabel, filename=None, ybounds=None):
        self.formula = formula
        self.xlabel = xlabel
        self.ylabel = ylabel
        self.filename = convert_name(filename if filename else formula)
        self.ybounds = ybounds
        self.histograms = {} # so we can access all hists of this distribution
        self.systs = {}
        self.calc_chi2 = False
    def generate_hist(self, label):
        self.histograms[label] = self.hbase.Clone('h_%s_%s'%(self.filename, convert_name(label)))
        self.histograms[label].Sumw2()
        return self.histograms[label]
    def generate_syst(self, label):
        hup = self.hbase.Clone('h_%s_%s_Up'%(self.filename, convert_name(label)))
        hdown = self.hbase.Clone('h_%s_%s_Down'%(self.filename, convert_name(label)))
        hup.Sumw2(); hdown.Sumw2()
        self.systs[label] = (hup, hdown)
        return (hup, hdown)

class FDistribution(Distribution):
    def __init__(self, formula, lo, hi, nbins, xlabel, ylabel, filename=None, ybounds=None):
        super(FDistribution, self).__init__(formula, xlabel=xlabel, ylabel=ylabel, 
                                            filename=filename,
                                            ybounds=ybounds)
        self.nbins = nbins 
        self.hbase = root.TH1D('hbase_%s'%(self.filename), 
                               '', self.nbins, lo, hi)

class VDistribution(Distribution):
    def __init__(self, formula, bins, xlabel, ylabel, filename=None, ybounds=None):
        super(VDistribution, self).__init__(formula, xlabel=xlabel, ylabel=ylabel, 
                                            filename=filename,
                                            ybounds=ybounds)

        self.nbins = len(bins)-1
        self.hbase = root.TH1D('hbase_%s'%(self.filename), 
                               '', self.nbins, bins)

#def FDistribution(name, lo, hi, nbins, xlabel, ylabel, filename=None, ybounds=None):
#    return Distribution(name=name, bin_list=(lo, hi, nbins), is_variable=False, 
#                        xlabel=xlabel, ylabel=ylabel, filename=filename, ybounds=ybounds)
#
#def VDistribution(name, bins, xlabel, ylabel, filename=None, ybounds=None):
#    return Distribution(name=name, bin_list=bins, is_variable=True, 
#                        xlabel=xlabel, ylabel=ylabel, filename=filename, ybounds=ybounds)

class Systematic(object):
    def __init__(self, name, weight_up, weight_down, color=1):
        self.name = name
        self.weight_up = weight_up
        self.weight_down = weight_down
        self.color = color
        self.hists = None
    def generate_weight(self, central, is_up):
        replacement = self.weight_up if is_up else self.weight_down
        if type(replacement) == str:
            return tTIMES(central, replacement)
        else:
            shift = str(central)
            for k, v in replacement.iteritems():
                shift = shift.replace(k, v)
            return shift

class PlotUtility(object):
    def __init__(self):
        self.canvas = root.HistogramDrawer()
        members = [x for x in dir(self.canvas) if ('__' not in x)]
        for m in members:
            if callable(getattr(self.canvas, m)):
                self._getPlotMethod(m)
            else:
                self._getPlotMember(m)
        self._processes = [] 
        self._distributions = []
        self._systematics = []
        # public configuration - defaults
        self.plot_label = '#it{CMS Preliminary}'
        self.do_underflow = True
        self.do_overflow = True
        self.signal_scale = 1
        self.cut = None
        self.mc_weight = None
        self.eventmod =  0
        self.eventnumber = 'eventNumber'
        self.n_threads = 8
    def _getPlotMethod(self, x):
        method = getattr(self.canvas, x)
        setattr(self, x,  lambda *args : method(*args))
    def _getPlotMember(self, x):
        member = getattr(self.canvas, x)
        setattr(self, x,  member)
    def add_process(self, p):
        self._processes.append(p)
    def add_distribution(self, d):
        self._distributions.append(d)
    def add_systematic(self, name, up, down, color=1):
        self._systematics.append(Systematic(name, up, down, color))
    def reset(self):
        self._distributions = []
        self._systematics = []
    def Draw(self):
        # override the thing that's bound in __init__
        logger.error('plot_utility.PlotUtility.Draw', 'Not implemented!')
    #def _read(self, proc, f):
    def _read(self, variables, proc, f):
        logger.info('PlotUtility.draw_all', 'Starting to read '+f.split('/')[-1])
        # figure out the nominal weight and cut strings
        final_weight = '1'
        final_cut = self.cut
        if (proc.process_type!=root.kData and proc.use_common_weight):
            final_weight = self.mc_weight
        if (proc.process_type<=root.kSignal3 and proc.process_type!=root.kData):
            final_weight = tTIMES(final_weight, str(self.signal_scale))
        if self.eventmod:
            if (proc.process_type==root.kData):
                final_cut = tAND(final_cut, '(%s%%%i)==0'%(self.eventnumber, self.eventmod))
            else:
                final_weight = tTIMES(final_weight, str(1./self.eventmod))
        final_cut = tAND(final_cut, proc.additional_cut)
        final_weight = tTIMES(final_weight, proc.additional_weight)

        weight_map = {'nominal' : final_weight}
        weights = [final_weight]
        if proc.process_type!=root.kData:
            for syst in self._systematics:
                if proc.use_common_weight:
                    up_weight = syst.generate_weight(final_weight, True)
                    down_weight = syst.generate_weight(final_weight, False)
                else:
                    continue
                weight_map['%s_Up'%(syst.name)] = up_weight
                weight_map['%s_Down'%(syst.name)] = down_weight
                weights.append(up_weight)
                weights.append(down_weight)

        xarr = proc.read(variables, weights, final_cut, files=[f])
        logger.info('PlotUtility.draw_all', 'Finished reading '+f.split('/')[-1])
        return {'xarr':xarr, 'weight_map':weight_map, 'f':f, 'proc':proc}

    def __draw(self, proc, weight_map, xarr):
        for dist in self._distributions:
            weights_nominal = xarr[weight_map['nominal']]
            draw_hist(hist = dist.histograms[proc.name],
                      xarr = xarr,
                      fields = (dist.formula, ),
                      weight = weight_map['nominal'])
            if proc.process_type!=root.kData:
                for syst in self._systematics:
                    if proc.use_common_weight:
                        weights_up = weight_map['%s_Up'%syst.name]
                        weights_down = weight_map['%s_Down'%syst.name]
                    else:
                        weights_up = weight_map['nominal']
                        weights_down = weight_map['nominal']
                    draw_hist(hist = dist.systs[syst.name][0],
                              xarr = xarr,
                              fields = (dist.formula, ),
                              weight = weights_up)
                    draw_hist(hist = dist.systs[syst.formula][1],
                              xarr = xarr,
                              fields = (dist.formula, ),
                              weight = weights_down)

    def draw_all(self, outdir):
        if not self.canvas.HasLegend():
            self.canvas.InitLegend()

        if not path.isdir(outdir):
            system('mkdir -p %s'%outdir)
        f_out = root.TFile(outdir+'hists.root', 'UPDATE')
        if f_out.IsZombie() or (f_out.GetListOfKeys().GetEntries() == 0):
            f_out.Close()
            f_out = root.TFile(outdir+'hists.root', 'RECREATE')
        f_buffer_path = '/tmp/%s/buffer_%i.root'%(getenv('USER'), root.gSystem.GetPid())
        f_buffer = root.TFile(f_buffer_path, 'RECREATE')
        f_buffer.cd()

        variables = []
        for dist in self._distributions:
            for syst in self._systematics:
                syst.hists = dist.generate_syst(syst.name)

            for proc in self._processes:
                dist.generate_hist(proc.name)

            if isinstance(dist.formula, basestring):
                variables.append(dist.formula)

        # loop through each process
        read_args = []
        for proc in self._processes:
            for f in proc.files:
                read_args.append((variables, proc, f))
        if self.n_threads == 1 or len(read_args) == 1:
            read = []
            for r in read_args:
                read.append(self._read(*r))
        else:
            pool = ThreadPool(self.n_threads)
            results = [pool.apply_async(self._read, r) for r in read_args]
            read = [r.get() for r  in results]

        # merge the results and draw
        for proc in self._processes:
            proc_read = []
            for r in read:
                if r['proc'] == proc:
                    proc_read.append(r)
            if len(proc_read) == 0:
                continue
            read[:] = [x for x in read if x not in proc_read] # remove the ones we've read
            xarr = {key:[] for key in proc_read[0]['xarr'].dtype.names}
            for r in proc_read:
                for key in xarr:
                    xarr[key].append(r['xarr'][key])
            for key in xarr:
                xarr[key] = np.concatenate(xarr[key], axis=0)
            self.__draw(proc_read[0]['proc'], proc_read[0]['weight_map'], xarr)

        # everything is filled,  now draw the histograms!
        for dist in self._distributions:
            if dist.formula=="1":
                totals = {'bg':0,  'sig':0,  'data':0}
                errs = {'bg':0,  'sig':0,  'data':0}
                table_format = '%-25s | %15f +/- %15f'

                table = ['%-25s | %15s +/- %15s'%('Process', 'Yield', 'Stat. unc.')]
                table.append("=================================================================")

                for proc in self._processes:
                    h = dist.histograms[proc.name]
                    integral = h.GetBinContent(1)
                    error = h.GetBinError(1)
                    table.append(table_format%(proc.name, integral, error))
                    if proc.process_type==root.kData:
                        proc_label = 'data'
                    elif proc.process_type<=root.kSignal3:
                        proc_label = 'sig'
                    else:
                        proc_label = 'bg'
                    totals[proc_label] += integral
                    errs[proc_label] += pow(error, 2)

                for k, v in errs.iteritems():
                    errs[k] = np.sqrt(v)

                table.append("=================================================================")
                table.append(table_format%('MC(bkg)', totals['bg'], errs['bg']))
                table.append(table_format%('MC(sig)', totals['sig'], errs['sig']))
                table.append(table_format%('Data', totals['data'], errs['data']))

                table.append("=================================================================")
                if totals['bg']:
                    table.append('S/B=%.3f, S/sqrtB=%.3f'%(totals['sig']/totals['bg'], 
                                                           totals['sig']/np.sqrt(totals['bg'])))

                with open(outdir+'yields.txt', 'w') as fyields:
                    fyields.write('\n'.join(table))

                for t in table:
                    logger.info('plot_utility.PlotUtility.Dump', t)

            h_unscaled = {'data':None, 'mc':None} # used for chi2 calc
            for proc in self._processes:
                h = dist.histograms[proc.name]
                if self.do_overflow:
                    fix_overflow(h)
                if self.do_underflow:
                    fix_underflow(h)
                if proc.process_type==root.kData:
                    h_unscaled['data'] = h.Clone()
                elif proc.process_type>root.kSignal3:
                    if not h_unscaled['mc']:
                        h_unscaled['mc'] = h.Clone()
                    else:
                        h_unscaled['mc'].Add(h)
                if isinstance(dist, VDistribution):
                    divide_bin_width(h)
                if dist.ybounds:
                    h.SetMinimum(ybounds[0])
                    h.SetMaximum(ybounds[1])
                h.GetXaxis().SetTitle(dist.xlabel)
                h.GetYaxis().SetTitle(dist.ylabel)
                h.SetTitle('')
                if self.canvas.IsStack():
                    h.SetLineWidth(2)
                if proc.dashed:
                    h.SetLineStyle(2)
                elif proc.dotted:
                    h.SetLineStyle(3)

                add_hist_args = [h]
                if (proc.process_type<=root.kSignal3 
                    and proc.process_type!=root.kData 
                    and self.signal_scale!=1):
                    add_hist_args.append( '%.1f#times%s'%(self.signal_scale, proc.name) )
                else:
                    add_hist_args.append( proc.name )
                add_hist_args.append( proc.process_type )
                if proc.color != proc.process_type:
                    add_hist_args.append( proc.color )
                else:
                    add_hist_args.append( -1 )
                add_hist_args += ['', proc.ratio]
                self.canvas.AddHistogram(*add_hist_args)

                f_out.WriteTObject(h, h.GetName(), "overwrite")
            for syst in self._systematics:
                hup, hdown = dist.systs[syst.name]
                for h in [hup, hdown]:
                    if self.do_overflow:
                        fix_overflow(h)
                    if self.do_underflow:
                        fix_underflow(h)
                    if isinstance(dist, VDistribution):
                        divide_bin_width(h)
                    h.SetLineWidth(3)
                    h.SetLineColor(syst.color)
                self.canvas.AddSystematic(hup, 'hist', syst.name)
                self.canvas.AddSystematic(hdown, 'hist')
                f_out.WriteTObject(hup, hup.GetName(), "overwrite")
                f_out.WriteTObject(hdown, hdown.GetName(), "overwrite")

            # output the canvas
            if dist.calc_chi2:
                p = h_unscaled['data'].Chi2Test(h_unscaled['mc'],'UW')
                self.canvas.AddPlotLabel('P(#chi^{2}|NDoF)=%.3g'%(p),0.6,0.5,False,42,.04)
            self.canvas.Logy(False)
            if not outdir.endswith('/'):
                outdir_ = '/'.join(outdir.split('/')[:-1])
                base_ = outdir.split('/')[-1] 
            else:
                outdir_ = outdir 
                base_ = ''
            self.canvas.Draw(outdir_, base_+dist.filename)
            self.canvas.ClearLegend()
            if dist.calc_chi2:
                p = h_unscaled['data'].Chi2Test(h_unscaled['mc'],'UW')
                self.canvas.AddPlotLabel('P(#chi^{2}|NDoF)=%.3g'%(p),0.6,0.5,False,42,.04)
            self.canvas.Logy(True)
            self.canvas.Draw(outdir_, base_+dist.filename+'_logy')

            self.canvas.Reset(False)

        f_out.Close()
        f_buffer.Close()
        system('rm %s'%f_buffer_path)
