#!/usr/bin/env python

# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

import os
import argparse
import string
import re
import mmap
import uuid
import urllib2

class RegExps( object ):
    regExps = ( re.compile( '[\t ]*$' ), re.compile( r"\xEF\xBB\xBF" ) )

class ReplaceData( object ):
    replacements = ( ( '\r\n', '\n', 'EOF1' ), ( '\r', '\n', 'EOF2' ), ( '\t', '    ', 'TAB' ) )

def findFiles( startDir, fileExt, recLevel, currLevel = 0, doFix = False ):
    elems = os.listdir( startDir )

    files = []
    dirs = []

    # build lists of dirs and files
    for ent in elems:
        ent_path = os.path.join( startDir, ent )


        if os.path.isfile( ent_path ) and any( [ True for ex in fileExt if ent.endswith( ex ) ] ):
            files.append( ent )
        elif os.path.isdir( ent_path ) and ent[ 0 ] != '.':
            dirs.append( ent )

    # phase check each file for the issues
    for f in files:
        originalFilePath    = os.path.join( startDir, f )

        content = ""

        with open( originalFilePath, "rb" ) as ofile:
            content = ofile.read()

        #test if we have to convert the file
        exReplacements = [ x for x in ReplaceData.replacements if x[ 0 ] in content ]
        exMatches = [ r for r in RegExps.regExps if r.match( content ) ]

        needRepl = len( exReplacements )
        needRegx = len( exMatches )

        if needRepl:
            for rep in exReplacements:
                print "DETECTED: %s in %s" % ( rep[ 2 ], originalFilePath )
                content = string.replace( content, rep[ 0 ], rep[ 1 ] )

        if needRegx:
            for regexp in exMatches:
                print "DETECTED: %s in %s" % ( regexp.pattern, originalFilePath )
                content = regexp.sub( '', content )

        needFix = needRepl or needRegx

        if len( content ) > 0 and not content[-1] == "\n":
            print "DETECTED: no newline in %s" % originalFilePath
            content = content + "\n"
            needFix = True

        if doFix and needFix:
            with open( originalFilePath, "wb" ) as ofile:
                ofile.write( content )

    if recLevel == 0 or ( recLevel > 0 and currLevel < recLevel ):
        for d in dirs:
            findFiles( os.path.join( startDir, d ), fileExt, recLevel,
                    currLevel + 1, doFix )

if __name__ == '__main__':

    parser = argparse.ArgumentParser( description='Source code cleaner' )
    parser.add_argument( '-p', dest='startDir', type=str, default='./src/libxively',
                        help='starting directory path default local' )
    parser.add_argument( '-e', dest='extension', nargs="+", type=str, default='.h .c',
                        help='file extension(s) list default .h .c' )
    parser.add_argument( '-r', dest='recursive', type=int, default=0,
                       help='recursive mode, default 0, 0 enables unlimited recursion')
    parser.add_argument( '-f', const=True, dest='doFix', action='store_const', default=False,
                       help='force the tool to do the fix' )

    args        = parser.parse_args()

    startDir    = args.startDir
    recursive   = args.recursive
    fileExt     = args.extension
    doFix       = args.doFix

    if doFix:
        print "\n<Tool is now fixing all issues in %s>\n" % startDir
    else:
        print "\n<Tool is now only looking for issues in %s \n in order to fix them use -f parameter>\n" % startDir

    print "This is your fortune...\n"
    print urllib2.urlopen("http://anduin.eldar.org/cgi-bin/fortune.pl?fortune_db=pratchett&text_format=yes").read()

    findFiles( startDir, fileExt, recursive, 0, doFix )
