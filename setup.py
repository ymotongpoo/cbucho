# -*- coding: utf-8 -*-

try:
  from setuptools.setuptools import setup, Extension
except:
  from distutils.core import setup, Extension
  
import sys
import os
from subprocess import Popen, PIPE


def read(name):
  return open(os.path.join(os.path.dirname(__file__), name)).read()


def curl_option(option):
  """\
  get options for libcurl
  """
  cmd = "curl-config " + option
  p = Popen(cmd, shell=True, stdin=PIPE, stdout=PIPE)
  res = p.stdout.read()
  p.wait();
  
  opts = []
  for opt in res.split():
    opt = opt.strip()
    if opt.startswith("-l") and option == '--libs':
      opts.append(opt[2:])
    elif opt.startswith("-L") and option == '--static-libs':
      opts.append(opt[2:])
    elif opt.startswith("-I") and option == '--cflags':
      opts.append(opt[2:])
    elif option == '--prefix':
      opts.append(opt)

  return opts
      

if sys.version_info >= (3, 0):
    if not getattr(setuptools, '_distribute', False):
        raise RuntimeError(
                'You must installed `distribute` to setup bucho with Python3')
    extra.update(
        use_2to3=True
    )

version = '0.0.1'
name = 'cbucho'
short_description = '`cbucho` is a package for C/API exercises.'

classifiers = [
    'Development Status :: 2 - Pre-Alpha',
    'License :: OSI Approved :: Python Software Foundation License',
    'Programming Language :: Python',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 2.6',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: 3.2',
    'Topic :: Utilities',
    ]

extra = {}

libraries = curl_option('--libs')
library_dirs = [opt + "/lib" for opt in curl_option('--prefix')]
include_dirs = [opt + "/include" for opt in curl_option('--prefix')]
print libraries
define_macros = []

cbucho_module = Extension('cbucho',
                          sources = ['cbuchomodule.c'],
                          libraries = libraries,
                          library_dirs = library_dirs,
                          include_dirs = include_dirs,
                          define_macros = define_macros)

setup(name = name,
      version = version,
      description = short_description,
      long_description = read('README.rst'),
      classifiers = classifiers,
      keywords = ['practice',],
      author = 'Yoshifumi YAMAGUCHI (ymotongpoo)',
      author_email = 'ymotongpoo@gmail.com',
      url='http://github.com/ymotongpoo/cbucho/',
      license='PSL',
      ext_modules = [cbucho_module],
      **extra
      )
