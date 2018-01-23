#!/usr/bin/env python

# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

import argparse
import os.path
import struct
from pprint import pprint

PATTERN     = "0x%02x"
NEWLINE     = "\n"
INDENT      = 4 * " "
PER_LINE    = 16

def tabs_2_spaces( s ):
   return s.replace( "\t", "    " )

h_pro       = tabs_2_spaces( "// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.\n// This is part of Xively C library." )
h_guard_beg = tabs_2_spaces( "#ifndef __%(name)s_H__\n#define __%(name)s_H__" )
h_guard_end = tabs_2_spaces( "#endif // __%s_H__" )
cpp_beg     = tabs_2_spaces( "#ifdef __cplusplus\nextern \"C\" {\n#endif" )
cpp_end     = tabs_2_spaces( "#ifdef __cplusplus\n}\n#endif" )

def load_file( file_name ):
    if not os.path.isfile( file_name ):
        raise RuntimeError( "There is no such file: [%s]!" % file_name );

    data = []

    with open( file_name, "rb" ) as f:
        byte = f.read( 1 )

        while byte:
            data.append( struct.unpack( 'B', byte )[ 0 ] )
            byte = f.read( 1 )

    return data[ 0 : -1 ]

def convert_to_c_array( data, array_name ):
    out     = "const unsigned char %s[ %d ] = {" % ( array_name, len( data ) )
    out    += NEWLINE
    out    += INDENT

    while data:
        head = data[ 0 : PER_LINE ]
        data = data[ PER_LINE :  ]

        for byte in head[ 0 : -1 ]:
            out += PATTERN % byte
            out += ", "

        out += PATTERN % head[ -1 ]

        if data:
            out += ","
            out += NEWLINE
            out += INDENT

    out += " };"

    return out

def create_c_file( data, array_name ):
    out  = h_pro + "\n\n"
    out += cpp_beg + "\n\n"

    out += convert_to_c_array( data, array_name )
    out += "\n\n"
    out += cpp_end + "\n"

    return out

def create_h_file( data, array_name ):
    out  = h_pro + "\n\n"
    out += ( h_guard_beg % { "name" : array_name.upper() } ) + "\n\n"
    out += cpp_beg + "\n\n"
    out += "extern const unsigned char %s[ %d ];\n\n" % ( array_name, len( data ) )
    out += cpp_end + "\n\n"
    out += ( h_guard_end % ( array_name.upper() ) ) + "\n"

    return out

def write_to_file( file_name, s ):

    print "writing to: %s" % file_name

    with open( file_name, "wb" ) as f:
        f.write( s )

if __name__ == '__main__':
    parser = argparse.ArgumentParser( description = "libxively buffer creator" )
    parser.add_argument( '--file_name', dest = 'file_name', type = str, required = 'True', help = 'file name that is going to be converted' )
    parser.add_argument( '--out_path', dest = 'out_path', type = str, help = 'output path' )
    parser.add_argument( '--array_name', dest = 'array_name', type = str, required = 'True', help = 'array name that is going to be used as an output' )
    parser.add_argument( '--no-pretend', dest = 'no_pret', action = 'store_const', const = True, default = False, help = 'disable printing to the console and enables writing to files' )

    args = parser.parse_args()

    file_name   = args.file_name
    array_name  = args.array_name
    out_path    = args.out_path

    if out_path:
        path = out_path
    else:
        path = os.path.join( '.', 'src', 'libxively' )

    h_file_name = os.path.join( path, array_name + ".h" )
    c_file_name = os.path.join( path, array_name + ".c" )

    data = load_file( file_name )

    if args.no_pret:
        write_to_file( h_file_name, create_h_file( data, array_name ) )
        write_to_file( c_file_name, create_c_file( data, array_name ) )
    else:
        print( create_h_file( data, array_name ) )
        print( create_c_file( data, array_name ) )
