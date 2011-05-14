# -*- coding: utf-8 -*-

try:
  from setuptools.setuptools import setup, Extension
except:
  from distutils.core import setup, Extension
  
import sys
import os
from subprocess import Popen, PIPE

##### utility functions
def read(name):
  return open(os.path.join(os.path.dirname(__file__), name)).read()

def conf_option(config_cmd, option):
  """\
  get options for libcurl
  """
  cmd = ' '.join([config_cmd, option])
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


      
##### version check
if sys.version_info >= (3, 0):
    if not getattr(setuptools, '_distribute', False):
        raise RuntimeError(
                'You must installed `distribute` to setup bucho with Python3')
    extra.update(
        use_2to3=True
    )

##### properties
version = '0.0.3'
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

# libcurl
libraries = conf_option('curl-config', '--libs')
library_dirs = [opt + "/lib" for opt in conf_option('curl-config', '--prefix')]
include_dirs = [opt + "/include" for opt in conf_option('curl-config', '--prefix')]

# libxml2
libraries.extend(conf_option('xml2-config', '--libs'))
library_dirs.extend([opt + "/lib" for opt in conf_option('xml2-config', '--prefix')])
include_dirs.extend([opt + "/include/libxml2" 
                     for opt in conf_option('xml2-config', '--prefix')])


define_macros = []

cbucho_module = Extension('cbucho',
                          sources = ["cbucho/cbuchomodule.c"],
                          libraries = libraries,
                          library_dirs = library_dirs,
                          include_dirs = include_dirs,
                          define_macros = define_macros)

# setup
setup(name = name,
      version = version,
      description = short_description,
      long_description = read("README.rst"),
      classifiers = classifiers,
      keywords = ["practice", "bucho"],
      author = "Yoshifumi YAMAGUCHI (ymotongpoo)",
      author_email = "ymotongpoo@gmail.com",
      url="http://github.com/ymotongpoo/cbucho/",
      platforms="Linux, Darwin",
      license="PSL",
      ext_modules = [cbucho_module],
      **extra
      )
