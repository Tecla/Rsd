#!/usr/bin/env python

import os

# waf options module
from waflib import Options, Utils, Configure, Context


#
# Source description variables
#

VERSION = '1.0'
APPNAME = 'Rsd'


#
# RenderSpud variables
#

try:
    (usysname, unodename, urelease, uversion, umachine) = os.uname()
    osarch = os.getenv('RS_OSARCH', usysname + '.' + umachine)
except:
    osarch = os.getenv('RS_OSARCH', 'Windows.x86')

# The subdirectories with wscripts in them
buildSubdirectories = [ 'src', 'test' ]


#
# Mandatory variables (for waf)
#

top = os.getenv('CWD', '.')
out = os.getenv('WAF_BUILD_DIR', os.path.join('build', osarch))


#
# Required functions for setting up and running the build
#

# Adds to the waf commandline, especially for 'waf configure'
def options(opt):
    # Add in our tools
    #import Options
    #Options.tooldir.append('waf_tools')

    # Add tool options to the commandline
    opt.load('compiler_c compiler_cxx boost')

    # Add in custom options
    # ...

    # Add in all the wscripts for options
    for buildSubdir in buildSubdirectories:
        opt.recurse(buildSubdir)


# Sets up build configuration; this is persistent after 'waf configure'
def configure(conf):
    # Make sure we've got our tools available we need;
    # the order of all of the following is important
    conf.load('compiler_c compiler_cxx boost')

    # Set up pthread and real-time clock availability (for boost_thread)
    if osarch.startswith('Windows'):
        pass
    else:
        conf.env['LIB_PTHREAD'] = [ 'pthread', 'rt' ]

    # Check for boost libraries we use
    conf.check_boost(lib = 'thread system atomic')

    # Common libraries used (that have no associated tool)
    # ...

    conf.env.append_unique('DEFINES', [ 'WAF=1' ])

    if osarch.startswith('Windows'):
        conf.env.append_unique('CCFLAGS', [ '/MD', '/Wall' ])
        conf.env.append_unique('CXXFLAGS', [ '/MD', '/Wall' ])
        conf.env.append_unique('LINKFLAGS', [ '/MD' ])
    else:
        conf.env.append_unique('CCFLAGS', [ '-fPIC', '-rdynamic', '-Wall', '-Wno-unused-parameter' ])
        conf.env.append_unique('CXXFLAGS', [ '-std=gnu++11', '-fPIC', '-rdynamic', '-Wall', '-Wno-unused-parameter' ])
        conf.env.append_unique('LINKFLAGS', [ '-fPIC', '-rdynamic' ])

    # Add in all the wscripts for configuration
    for buildSubdir in buildSubdirectories:
        conf.recurse(buildSubdir)

    # Save off the env so far to copy it for debug, opt, and profile variants
    defaultEnv = conf.env

    # Debug variant of the build
    conf.setenv('debug', defaultEnv)
    conf.env.append_unique('DEFINES', [ 'DEBUG=1', '_DEBUG=1' ])
    if osarch.startswith('Windows'):
        conf.env.append_unique('CCFLAGS', [ '/Od', '/Zi' ])
        conf.env.append_unique('CXXFLAGS', [ '/Od', '/Zi' ])
        conf.env.append_unique('LINKFLAGS', [ '/Od', '/Zi' ])
    else:
        conf.env.append_unique('CCFLAGS', [ '-O0', '-g3' ])
        conf.env.append_unique('CXXFLAGS', [ '-O0', '-g3' ])
        conf.env.append_unique('LINKFLAGS', [ '-O0', '-g3' ])

    # Opt variant; add some optimization flags (shared between opt and profile)
    conf.setenv('opt', defaultEnv)
    optEnv = conf.env
    conf.env.append_unique('DEFINES', [ 'NDEBUG=1' ])
    if osarch.startswith('Windows'):
        conf.env.append_unique('CCFLAGS', [ '/Ox' ])
        conf.env.append_unique('CXXFLAGS', [ '/Ox' ])
    else:
        conf.env.append_unique('CCFLAGS', [ '-O3' ])
        conf.env.append_unique('CXXFLAGS', [ '-O3' ])

    # Profile variant of the build
    conf.setenv('profile', optEnv)
    if osarch.startswith('Windows'):
        conf.env.append_unique('CCFLAGS', [ '/Zo' ])
        conf.env.append_unique('CXXFLAGS', [ '/Zo' ])
        conf.env.append_unique('LINKFLAGS', [ '/Zo' ])
    else:
        conf.env.append_unique('CCFLAGS', [ '-g3' ])
        conf.env.append_unique('CXXFLAGS', [ '-g3' ])
        conf.env.append_unique('LINKFLAGS', [ '-g3' ])


# Set up build targets (build_opt, build_debug, build_profile, clean_opt, ...)
from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
for x in 'debug opt profile'.split():
    for y in (BuildContext, CleanContext, InstallContext, UninstallContext):
        name = y.__name__.replace('Context','').lower()
        class tmp(y):
            cmd = name + '_' + x
            variant = x


# Build the codebase; this is where build tasks are set up
def build(bld):
    # Sanity check that they chose a specific variant to build
    if not bld.variant and (bld.cmd != 'list'):
        bld.fatal('call "waf build_debug|build_opt|build_profile", and try "waf --help"')

    # Add in all the subdirectory wscripts for building
    for buildSubdir in buildSubdirectories:
        bld.recurse(buildSubdir)

