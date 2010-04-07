import os, pdb
import SConsider

packagename = SConsider.getPackageName(__name__)

buildSettings = {
    packagename : {
        'sourceFiles'      : SConsider.listFiles(['*.cpp']),
        'targetType'       : 'LibraryShared',
        'lazylinking'      : True,
        'public' : {
            'includes'     : SConsider.listFiles(['*.h']),
        }
    }
}

SConsider.createTargets(packagename, buildSettings)
