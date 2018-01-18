#!/usr/bin/env python

# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

import os
import argparse
import uuid
import subprocess
import shlex

CLANG_FORMAT = "clang-format -style=file"

def clangFormatFile( filename ):
    #oldFilePath         = filename + '.old'
    #switchFiles         = False

    args = shlex.split( CLANG_FORMAT )
    args += [ "-i", filename ]

    print( args )

    p = subprocess.Popen( args )
    #p.wait()

    # switch files f1->tmp, f2->f1, tmp->f2
    #if switchFiles:
    #    tmpFileName = os.path.join( startDir, str( uuid.uuid1() ) )
    #    os.rename( originalFilePath, tmpFileName )
    #    os.rename( oldFilePath, originalFilePath )
    #    os.rename( tmpFileName, oldFilePath )


def findFiles( startDir, fileExt, recLevel, currLevel = 0 ):
    contents    = os.listdir( startDir )

    with open(".clang-format-ignore") as f:
        files_to_ignore = [x.strip('\n') for x in f.readlines()]

    files   = [ x for x in contents if os.path.isfile( os.path.join( startDir, x ) ) and x.endswith( fileExt ) and x not in files_to_ignore ]
    dirs    = [ x for x in contents if os.path.isdir( os.path.join( startDir, x ) ) and x[ 0 ] != '.' and x not in files_to_ignore ]

    for f in files:
        filename = os.path.join( startDir, f )
        clangFormatFile( filename )

    if recLevel == 0 or ( recLevel > 0 and currLevel < recLevel ):
        for d in dirs:
            findFiles( os.path.join( startDir, d ), fileExt, recLevel,
                    currLevel + 1 )

if __name__ == '__main__':

    parser = argparse.ArgumentParser( description='Source code formatter' )
    parser.add_argument( '-r', dest='recursive', type=int, default=100,
                       help='recursive mode, default 1, set 0 if you want to enable unlimited recursion')

    args        = parser.parse_args()
    recursive   = args.recursive

    directories = [ 'src/', 'include/', 'include_senml/', 'examples/' ];

    for dir in directories:
        findFiles( dir, ".h", recursive )
        findFiles( dir, ".c", recursive )
