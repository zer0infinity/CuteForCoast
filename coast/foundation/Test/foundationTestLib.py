import os, pdb
import StanfordUtils

packagename = StanfordUtils.getPackageName(__name__)
target = None

buildSettings = {
                 'includeSubdir'    : '',
                 'libDepends'       : ['CoastFoundation', 'testfwFoundation'],
#                 'appendUnique'     : { 'CPPDEFINES' : 'fooX' }
                 }

# create environment for target
targetEnv = StanfordUtils.CloneBaseEnv()

# update environment by adding dependencies to used modules
StanfordUtils.setModuleDependencies(targetEnv, buildSettings.get('libDepends', []))

# win32 specific define to export all symbols when creating a DLL
##newVars = buildSettings.get('appendUnique',{}) + StanfordUtils.EnvVarDict(CPPDEFINES=[packagename.upper() + '_IMPL'])
##targetEnv.AppendUnique(**newVars)

# maybe we need to add this libraries local include path when building it (if different from .)
StanfordUtils.setIncludePath(targetEnv, packagename, buildSettings.get('includeSubdir', ''))

# specify this modules target
target = targetEnv.Program(packagename, StanfordUtils.listFiles(['*.cpp']))

# either create a new environment to register this package with or use targetEnv when no real target gets created
registerEnv = targetEnv
registerEnv.Tool('registerObjects',
             package=packagename,
             testApps=[target],
             config=StanfordUtils.listFiles(['config/*.any', 'config/*.txt', 'config/*.tst', 'config/testscript.sh', 'config/prjconfig.sh', 'tmp/*.any', '*.any'])
             )