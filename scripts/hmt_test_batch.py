#!/usr/bin/env python
#import numpy as np
#import sys, shutil, socket, argparse
import sys, shutil, socket, os
#sys.path.append('/path/to/glia/code/gadget/python')
sys.path.append('/Users/ayla/devel/glia/code/gadget/python')
from script_util import *

#dim = '2d'

t_resume = False
#t_resume = (len(sys.argv) > 1 and sys.argv[1] == '-r')

def main(argv):
  #print(argv)
  if len(argv) < 9:
    raise StandardError('{} requires at least 9 arguments'.format(argv[0]))

  im_gray_training = argv[1]
  im_gray_test = argv[2]
  im_gray_all = argv[3]
  im_cropped_chm = argv[4]
  im_cropped_blur = argv[5]
  truth_cropped_region = argv[6]
  glia_bin = argv[7]
  glia_results = argv[8]

  # paths
  p_d = {
    'bin': glia_bin,
    'res': glia_results
  }
  p_dbin = p_d['bin'] + '/{b}'

  gray_training_files = [os.path.join(im_gray_training, f) for f in os.listdir(im_gray_training) if os.path.isfile(os.path.join(im_gray_training, f)) and f.lower().endswith('.mha')]
  if len(gray_training_files) < 1:
    raise StandardError("Missing grayscale data files")

  gray_test_files = [os.path.join(im_gray_test, f) for f in os.listdir(im_gray_test) if os.path.isfile(os.path.join(im_gray_test, f)) and f.lower().endswith('.mha')]
  if len(gray_test_files) < 1:
    raise StandardError("Missing grayscale test data files")

  boundary_files = [os.path.join(im_cropped_chm, f) for f in os.listdir(im_cropped_chm) if os.path.isfile(os.path.join(im_cropped_chm, f)) and f.lower().endswith('.mha')]
  if len(boundary_files) < 1:
    raise StandardError("Missing boundary files")

  blur_boundary_files = [os.path.join(im_cropped_blur, f) for f in os.listdir(im_cropped_blur) if os.path.isfile(os.path.join(im_cropped_blur, f)) and f.lower().endswith('.mha')]
  if len(blur_boundary_files) < 1:
   raise StandardError("Missing boundary files")

  if len(boundary_files) != len(blur_boundary_files):
    raise StandardError("Require same number of boundary and blurred boundary files")

  truth_files = [os.path.join(truth_cropped_region, f) for f in os.listdir(truth_cropped_region) if os.path.isfile(os.path.join(truth_cropped_region, f)) and f.lower().endswith('.png')]
  if len(truth_files) < 1:
   raise StandardError("Missing ground truth files")

  file_names = []
  for f in gray_training_files:
    basename = os.path.basename(f)
    name, ext = os.path.splitext(basename)
    file_names.append(name)

  test_names = []
  for f in gray_test_files:
    basename = os.path.basename(f)
    name, ext = os.path.splitext(basename)
    test_names.append(name)

  p_fdat = {
    # ground truth segmentations
    'truth': truth_cropped_region + '/{i}.png',
    # all raw images
    'gray_all': im_gray_all + '/{i}.mha',
    # training raw images
    'gray_training': im_gray_training + '/{i}.mha',
    # boundary probability maps
    'pb': im_cropped_chm + '/{i}.mha',
    # blurred boundary probability maps
    'pbb': im_cropped_blur + '/{i}.mha'
  }

  p_dres = {
    'segii': p_d['res'] + '/segii',  # watershed superpixels
    'segiit': p_d['res'] + '/segiitest',  # watershed superpixels
    'segi': p_d['res'] + '/segi',  # pre-merged superpixels
    'segit': p_d['res'] + '/segitest',  # pre-merged superpixels
    'order': p_d['res'] + '/order',  # merge trees
    'sal': p_d['res'] + '/sal',  # merge saliencies
    'bcf': p_d['res'] + '/bcf',  # boundary features
    'bcl': p_d['res'] + '/bcl',  # boundary labels
    'bcm': p_d['res'] + '/bcm',  # boundary classifier model
    'bcp': (p_d['res'] + '/bcp'),  # boundary predictions
    'seg': (p_d['res'] + '/seg'),  # final segmentations
    'segt': (p_d['res'] + '/segtest'),  # final segmentations
  }

  p_fres = {}
  for x in ['segiit', 'segit', 'segt', 'segii', 'segi', 'seg']: p_fres[x] = p_dres[x] + '/{i}.png'
  for x in ['order', 'sal', 'bcf', 'bcl', 'bcp']:
    p_fres[x] = p_dres[x] + '/{i}.ssv'
  p_fres['bcm'] = p_dres['bcm'] + '/bcm.bin'

  # parameters
  t_nproc = 70  # number of max parallel processes
  t_nthrds= 1   # number of max parallel threads
  t_wt = '0.008'  # initial water level
  t_th = '0.001'  # watershed threshold
  t_st = ['100', '500', '3.0']  # pre-merging params

  t_iids = {
   'tr' : file_names, 'te': test_names
  }

  t_iids['all'] = t_iids['tr'] + t_iids['te']
  print(t_iids)

  # initial superpixels
  _jobs = list()
  make_dir(p_dres['segii'].format(s=''))
  make_dir(p_dres['segiit'].format(s=''))
  for i in t_iids['all']:
    _f = p_fres['segii'].format(s='', i=i)
    _g = p_fres['segiit'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='watershed'),
        '-i', p_fdat['pbb'].format(s='', i=i),
        '-l', t_wt, '-t', t_th, '-u', 'true', '-o', _f, '--toi', _g]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=t_nthrds, name='segii')

  # pre-merging
  _jobs = list()
  make_dir(p_dres['segi'].format(s=''))
  make_dir(p_dres['segit'].format(s=''))
  for i in t_iids['all']:
    _f = p_fres['segi'].format(s='', i=i)
    _g = p_fres['segit'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='pre_merge'),
        '-s', p_fres['segii'].format(s='', i=i),
        '-p', p_fdat['pb'].format(s='', i=i),
        '-t', t_st[0], t_st[1], '-b', t_st[2],
        '-r', 'true', '-u', 'true', '-o', _f, '--toi', _g]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=t_nthrds, name='segi')
#  execute(_jobs, nproc=1, nt=1, name='segi')

  # trees and merging saliencies
  _jobs = list()
  make_dir(p_dres['order'].format(s=''))
  make_dir(p_dres['sal'].format(s=''))
  for i in t_iids['all']:
    _f_o = p_fres['order'].format(s='', i=i)
    _f_s = p_fres['sal'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f_o) and is_file_valid(_f_s)):
      _job = [
        p_dbin.format(b='merge_order_pb'),
        '-s', p_fres['segi'].format(s='', i=i),
        '-p', p_fdat['pb'].format(s='', i=i),
        '-t', '1', '-o', _f_o, '-y', _f_s]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=t_nthrds, name='order/sal')

  # boundary features
  _jobs = list()
  make_dir(p_dres['bcf'].format(s=''))
  for i in t_iids['all']:
    _f = p_fres['bcf'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='bc_feat'),
        '-s', p_fres['segi'].format(s='', i=i),
        '-o', p_fres['order'].format(s='', i=i),
        '-y', p_fres['sal'].format(s='', i=i),
        # since we're going over all data, use all raw grayscale files
        '--rbi', p_fdat['gray_all'].format(s='', i=i),
        # 16 or 32 appears to work best here
        '--rbb', '16', '--rbl', '120.0', '--rbu', '255.0',
        '--rbi', p_fdat['pb'].format(s='', i=i),
        '--rbb', '16', '--rbl', '120.0', '--rbu', '255.0',
        '--pb', p_fdat['pb'].format(s='', i=i),
        '--s0', '1.0', '--sb', '1.0', '--bt', '70', '190', '255',
        '-n', 'false', '-l', 'false', '--simpf', 'false', '-b', _f]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=t_nthrds, name='bcf')
#  execute(_jobs, nproc=1, nt=1, name='bcf')

  # boundary labels
  _jobs = list()
  make_dir(p_dres['bcl'].format(s=''))
  #for i in t_iids['all']:
  # no segmentations available for test data
  for i in t_iids['tr']:
    _f = p_fres['bcl'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='bc_label_ri'),
        '-s', p_fres['segi'].format(s='', i=i),
        '-o', p_fres['order'].format(s='', i=i),
        '-t', p_fdat['truth'].format(s='', i=i),
        '--f1', 'true', '-d', '0.5', '-g', '0', '-p', 'false', '-w', 'false', '-l', _f]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=t_nthrds, name='bcl')

  # boundary classifier training
  _jobs = list()
  make_dir(p_dres['bcm']).format(s='')
  _f = p_fres['bcm'].format(s='')
  if not (t_resume and is_file_valid(_f)):
    _job = [
      p_dbin.format(b='train_rf'),
      '--nt', '255', '--mt', '0', '--sr', '0.1', '--ns', '10', '--bal', 'true',
      '--m', _f]
    for i in t_iids['tr']:
      _job.extend([
        '--f', p_fres['bcf'].format(s='', i=i),
        '--l', p_fres['bcl'].format(s='', i=i)])
    #print(_job)
    _jobs.append(_job)
  execute(_jobs, nproc=1, nt=1, name='bcm')

  # boundary predictions
  _jobs = list()
  make_dir(p_dres['bcp']).format(s='')
  for i in t_iids['all']:
    _f = p_fres['bcp'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='pred_rf'),
        '--m', p_fres['bcm'].format(s=''), '--l', '-1',
        '--f', p_fres['bcf'].format(s='', i=i),
        '--p', p_fres['bcp'].format(s='', i=i)]
      #print(_job)
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=1, name='bcp')

  # final segmentation
  _jobs = list()
  make_dir(p_dres['seg'].format(s=''))
  make_dir(p_dres['segt'].format(s=''))
  for i in t_iids['all']:
    _f = p_fres['seg'].format(s='', i=i)
    _g = p_fres['segt'].format(s='', i=i)
    if not (t_resume and is_file_valid(_f)):
      _job = [
        p_dbin.format(b='segment_greedy'),
        '-s', p_fres['segi'].format(s='', i=i),
        '-o', p_fres['order'].format(s='', i=i),
        '-p', p_fres['bcp'].format(s='', i=i),
        '-t', '0.5',
        '-r', 'true', '-u', 'true', '-f', _f, '--toi', _g]
      _jobs.append(_job)
  execute(_jobs, nproc=t_nproc, nt=1, name='seg')
#  execute(_jobs, nproc=1, nt=1, name='seg')


if __name__ == "__main__":
  main(sys.argv)
