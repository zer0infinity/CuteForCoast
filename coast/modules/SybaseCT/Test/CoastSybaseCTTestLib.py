import os
import StanfordUtils
from stat import *

packagename = StanfordUtils.getPackageName(__name__)

buildSettings = {
    packagename : {
        'targetType'       : 'ProgramTest',
        'linkDependencies' : ['CoastSybaseCT', 'testfwWDBase'],
        'sourceFiles'      : StanfordUtils.listFiles(['*.cpp']),
        'copyFiles'        : [(StanfordUtils.findFiles(['.'],['.any','.txt','.tst','.sql']), S_IRUSR|S_IRGRP|S_IROTH)],
    },
}

StanfordUtils.createTargets(packagename, buildSettings)
