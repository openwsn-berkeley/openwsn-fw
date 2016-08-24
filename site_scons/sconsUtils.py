import os
import fnmatch
from SCons.Script import *

'''
Includes common SCons utilities, usable by SConstruct and any SConscript.
'''

def printEnv(env):
    '''Prints provided SCons environment'''
    envDict = env.Dictionary()
    envKeys = envDict.keys()
    envKeys.sort()
    
    print 'Environment Contents'
    print '===================='
    for key in envKeys:
        print '* {0}\n  {1}'.format(key, envDict[key])


def findPattern(pattern, path):
    '''
    Finds the files matching the provided pattern in the directory tree rooted
    at the provided path.

    :returns: List of the files found
    '''
    returnVal = []
    for (dirpath,dirnames,filenames) in os.walk(path):
        for filename in filenames:
            if fnmatch.fnmatch(filename,pattern):
                returnVal += [os.path.join(dirpath,filename)]
    return returnVal
