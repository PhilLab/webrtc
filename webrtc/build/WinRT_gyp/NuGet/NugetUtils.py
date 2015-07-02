# Utils for creating nuget packages

import  os
import  sys
import argparse

def createDirIfNotExist(path):
    if not os.path.exists(os.path.dirname(path)):
        os.makedirs(os.path.dirname(path))

def log(text):
    print >> sys.stdout, text

def getVersionFromArguments():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-V', default='1.0.0', help='Version for nuget package',
                        action='store', dest='version')
    args = parser.parse_args()
    return args.version
