from re import sub
from sys import stdout,stderr
from os import getenv
from PandaCore.Utils.logging import logger
from collections import namedtuple

ModelParams = namedtuple('ModelParams',
                         ['m_V','m_DM','gV_DM','gA_DM',
                          'gV_q','gA_q','sigma','delta'])

def read_nr_model(mV,mDM,couplings=None,path='non-resonant'):
    tmpl = getenv('PANDA_XSECS')+'/'+path+'/%i_%i_xsec_gencut.dat'
    try:
        fdat = open(tmpl%(mV,mDM))
    except IOError:
        logger.error('PandaCore.Tools.models.read_nr_model','Could not open %s'%(tmpl%(mV,mDM)))
        return None
    for line in fdat:
        if 'med dm' in line:
            continue
        p = ModelParams(*[float(x) for x in line.strip().split()])
        if couplings:
            if couplings==(p.gV_DM,p.gA_DM,p.gV_q,p.gA_q):
                fdat.close()
                return p
            else:
                continue
        else:
            # if not specified, take the first valid model (nominal)
            fdat.close()
            return p

def read_r_model(mV,mDM=100,couplings='nominal'):
    tmpl = getenv('PANDA_XSECS')+'/resonant/%i_%i.dat'
    try:
        fdat = open(tmpl%(mV,mDM))
    except IOError:
        logger.error('PandaCore.Tools.models.read_nr_model','Could not open %s'%(tmpl%(mV,mDM)))
        return None
    for line in fdat:
        line_coupling,sigma = line.split(':')
        if not(line_coupling==couplings):
            continue
        sigma = float(sigma)
        p = ModelParams(mV,mDM,1,1,0.25,0.25,sigma,0)
        fdat.close()
        return p

