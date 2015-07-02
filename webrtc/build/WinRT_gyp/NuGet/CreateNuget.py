# Recreate and publish the nuget package

import os
import sys
import shutil
import subprocess
import xml.etree.ElementTree as ElemTree
from NugetUtils import createDirIfNotExist, log, getVersionFromArguments

win_dir = '..\\..\\..\\..\\build_win\\Release\\'
win_dir_dst = 'lib\\netcore45'
win_dir_files = ['webrtc_winrt_api.dll', 'webrtc_winrt_api.winmd']

win_phone_dir = '..\\..\\..\\..\\build_win_phone\\Release\\'
win_phone_dir_dst = 'lib\\portable-wpa81'
win_phone_dir_files = ['webrtc_winrt_api_win_phone.dll' , 'webrtc_winrt_api.winmd']
win_phone_renames = {'webrtc_winrt_api_win_phone.dll' : 'webrtc_winrt_api.dll'}

nuspecfile = 'Webrtc_winrt_api.base.nuspec'

nuget_api_key = ''

def main():

    for fl in win_dir_files:
        file_path_src = os.path.join(os.getcwd(), win_dir, fl)
        createDirIfNotExist(file_path_src)
        file_path_dst = os.path.join(os.getcwd(), win_dir_dst, fl)
        createDirIfNotExist(file_path_dst)
        shutil.copy(file_path_src, file_path_dst)

    for fl in win_phone_dir_files:
        file_path_src = os.path.join(os.getcwd(), win_phone_dir, fl)
        createDirIfNotExist(file_path_src)
        file_path_dst = os.path.join(os.getcwd(), win_phone_dir_dst, fl)
        createDirIfNotExist(file_path_dst)
        shutil.copy(file_path_src, file_path_dst)

    for k, v in win_phone_renames.iteritems():
        file_path_initial = os.path.join(os.getcwd(), win_phone_dir_dst, k)
        file_path_final = os.path.join(os.getcwd(), win_phone_dir_dst, v)
        if os.path.isfile(file_path_final):
            os.remove(file_path_final)
        os.rename(file_path_initial, file_path_final)

    version = getVersionFromArguments()
    p = subprocess.Popen(['nuget', 'pack', nuspecfile,
                         '-Version', version],
                         shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    log(p.stdout.read())
    log(p.stderr.read())

    if nuget_api_key == '':
        log('Publishing aborted because an API key is not provided. Please set it in CreateNuget.py:nuget_api_key')
        sys.exit(1)

    nupkg_file = nuspecfile.replace('base', version).replace('nuspec', 'nupkg')
    log('Pushing ' + nupkg_file + ' package to nuget.org')

    p = subprocess.Popen(['nuget', 'push', nupkg_file,
                          nuget_api_key],
                         shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    log(p.stdout.read())
    log(p.stderr.read())

if __name__ == "__main__":
    main()