# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

import random

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

"""////////////////////////////////////"""
xi_backoff_class_t = enum(
      'XI_BACKOFF_CLASS_NONE'
    , 'XI_BACKOFF_CLASS_RECOVERABLE'
    , 'XI_BACKOFF_CLASS_TERMINAL'
)

class xi_backoff_status_t( object ):
    def __init__( self, backoff_class ):
        self._backoff_class = backoff_class
        self._last_update = 0
        self._backoff_lut_i = 0

    @property
    def connection_timeout( self ):
        """Get the current connection time ( penalty time )"""
        prev_i = max( self.backoff_lut_i - 1, 0 )

        curr_val = xi_updatable_storage.xi_backoff_lut[ self.backoff_lut_i ]
        prev_val = xi_updatable_storage.xi_backoff_lut[ prev_i ]

        half_prev_val = prev_val / 2

        return curr_val + random.randint( -half_prev_val, half_prev_val )

    @property
    def backoff_class( self ):
        """Get the current backoff class"""
        return self._backoff_class

    @backoff_class.setter
    def backoff_class( self, value ):
        """Get the current backoff class"""
        self._backoff_class = value

    @property
    def last_update( self ):
        return self._last_update

    @property
    def backoff_lut_i( self ):
        return self._backoff_lut_i

    @backoff_lut_i.setter
    def backoff_lut_i( self, value ):
        self._backoff_lut_i = value

    @last_update.setter
    def last_update( self, seconds ):
        self._last_update = seconds

def clamp_value( vmin, vmax, value ):
    return max( min( vmax, value ), vmin )

class xi_updatable_storage( object ):
    """
    this suppose to simulate by server updatable storage
    server can send us different values through operational channel
    so all members of this class may be changed over time and in runtime
    """
    xi_backoff_lut = [ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 ]
    xi_decay_lut = [ 4, 4, 8, 16, 30, 30, 30, 30, 30, 30, 30 ]

"""////////////////////////////////////"""

def xi_backoff_inc_penalty():
    xi_globals.backoff_status.backoff_lut_i \
        = min( \
            ( xi_globals.backoff_status.backoff_lut_i + 1 ) \
            , len( xi_updatable_storage.xi_backoff_lut ) - 1 )

    print "@inc - new backoff_lut_i = %d" % ( xi_globals.backoff_status.backoff_lut_i, )

    xi_globals.backoff_status.last_update = xi_globals.time

def xi_backoff_dec_penalty():
    xi_globals.backoff_status.backoff_lut_i \
        = max( xi_globals.backoff_status.backoff_lut_i - 1, 0 );

    print "@dec - new backoff_lut_i = %d" % ( xi_globals.backoff_status.backoff_lut_i, )

def xi_backoff_terminal():
    xi_globals.backoff_status.backoff_class = xi_backoff_class_t.XI_BACKOFF_CLASS_TERMINAL

def xi_backoff_update():
    if xi_globals.backoff_status.backoff_lut_i > 0: # There is no point in checking if lut_i == 0
        if xi_globals.time - xi_globals.backoff_status.last_update > xi_updatable_storage.xi_decay_lut[ xi_globals.backoff_status.backoff_lut_i ]:
            if xi_globals.backoff_status.backoff_class == xi_backoff_class_t.XI_BACKOFF_CLASS_NONE:
                xi_backoff_dec_penalty()
            xi_globals.backoff_status.last_update = xi_globals.time

def xi_backoff_reset():
    xi_globals.backoff_status.backoff_lut_i = 0
    xi_globals.backoff_status.last_update = xi_globals.time

def xi_nop():
    pass

"""////////////////////////////////////"""

xi_status = enum(
      'XI_STATUS_OK', 'XI_BACKOFF_TERMINAL', 'XI_BACKOFF_CONNECTION', 'XI_SOCKET_ERROR' \
    , 'XI_SOCKET_CONNECTION_ERROR', 'XI_CONNECTION_RESET_BY_PEER_ERROR' \
    , 'XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE', 'XI_INTERNAL_ERROR' )

class xi_globals( object ):
    backoff_status = xi_backoff_status_t( xi_backoff_class_t.XI_BACKOFF_CLASS_NONE )
    time = 0

def xi_backoff_classify_state( state ):
    options = {
          xi_status.XI_STATUS_OK : xi_nop
        , xi_status.XI_BACKOFF_TERMINAL : xi_backoff_terminal
        , xi_status.XI_SOCKET_ERROR : xi_backoff_inc_penalty
        , xi_status.XI_SOCKET_CONNECTION_ERROR : xi_backoff_inc_penalty
        , xi_status.XI_CONNECTION_RESET_BY_PEER_ERROR : xi_backoff_terminal
        , xi_status.XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE : xi_backoff_inc_penalty
        , xi_status.XI_INTERNAL_ERROR : xi_backoff_inc_penalty
    }
    options[ state ]()

def step_over_sec( seconds, state ):
    """ it fast forwards time ( seconds ) which is an aproximation of number of calls of errored connection tries """

    xi_globals.time += seconds

    for i in range( 0, seconds ):
        print "Connection timeout = %d" % ( xi_globals.backoff_status.connection_timeout, )
        xi_backoff_classify_state( state )

    xi_backoff_update()

