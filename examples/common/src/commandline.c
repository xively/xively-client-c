/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * This module implements a command line argument parser
 */

#include "xively.h"
#include "commandline.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef XI_CROSS_TARGET
#include <getopt.h>
#define XI_EXAMPLE_DEFAULT_QOS XI_MQTT_QOS_AT_LEAST_ONCE
#else
#define XI_EXAMPLE_DEFAULT_QOS XI_MQTT_QOS_AT_MOST_ONCE
#endif /* XI_CROSS_TARGET */

void xi_usage( const char* options, unsigned options_length );

/* Globals shared with the example application
 * (At least for now. These should be in a structure someday) */
/* Flags set by commandline arguments. */
int xi_quiet_flag               = 0;
xi_mqtt_retain_t xi_will_retain = XI_MQTT_RETAIN_FALSE;
int xi_abnormalexit_flag        = 0;
int xi_help_flag                = 0;

xi_mqtt_qos_t xi_example_qos = XI_EXAMPLE_DEFAULT_QOS;

const char* xi_account_id = NULL;
/* user name and password */
const char* xi_username = NULL;
const char* xi_password = NULL;
/* topics */
const char* xi_publishtopic   = NULL;
const char* xi_subscribetopic = NULL;
/* Last will items */
const char* xi_will_topic   = NULL;
const char* xi_will_message = NULL;
/* memory limit */
int xi_memorylimit = 0;
/* number of times through the publish loop */
unsigned int xi_numberofpublishes = 1;

#ifndef XI_CROSS_TARGET
int xi_parse( int argc, char** argv, char* valid_options, unsigned options_length )
{
    int c;

    while ( 1 )
    {
        static struct option long_options[] = {
            {"account-id", required_argument, 0, 'a'},
            {"username", required_argument, 0, 'u'},
            {"password", required_argument, 0, 'P'},
            {"subscribetopic", required_argument, 0, 't'},
            {"publishtopic", required_argument, 0, 'p'},
            {"publishmessage", required_argument, 0, 'm'},
            {"publishrate", required_argument, 0, 'r'},
            {"quiet", no_argument, 0, 'q'},
            {"credentials", required_argument, 0, 'c'},
            {"will-topic", required_argument, 0, 'T'},
            {"will-payload", required_argument, 0, 'M'},
            {"will-qos", required_argument, 0, 'Q'},
            {"will-retain", no_argument, 0, 'R'},
            {"exitwithoutclosing", no_argument, 0, 'E'},
            {"memorylimit", required_argument, 0, 'l'},
            {"numberofpublishes", required_argument, 0, 'n'},
            {"messagesize", required_argument, 0, 'S'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}};

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long( argc, argv, valid_options, long_options, &option_index );

        /* Detect the end of the options. */
        if ( -1 == c )
        {
            break;
        }

        switch ( c )
        {
            case 'q':
                xi_quiet_flag = 1;
                break;
            case 'R':
                xi_will_retain = XI_MQTT_RETAIN_TRUE;
                break;
            case 'E':
                xi_abnormalexit_flag = 1;
                break;
            case 'h':
                xi_help_flag = 1;
                break;
            case 'a':
                xi_account_id = optarg;
                break;
            case 'u':
                xi_username = optarg;
                break;
            case 'P':
                xi_password = optarg;
                break;
            case 't':
                xi_subscribetopic = optarg;
                break;
            case 'p':
                xi_publishtopic = optarg;
                break;
            case 'm':
                printf( "-m --publishmessage\n\tThe message to publish.\n" );
                break;
            case 'r':
                printf( "-r --publishrate\n\tThe rate at which to publish messages in "
                        "seconds.\n" );
                break;
            case 'c':
                printf( "-c --credentials\n\tThe name of a file containing MQTT username "
                        "and password.\n" );
                break;
            case 'T':
                xi_will_topic = optarg;
                break;
            case 'M':
                xi_will_message = optarg;
                break;
            case 'Q':
                /* will_qos = atoi(optarg); */
                break;
            case 'l':
                xi_memorylimit = atoi( optarg );
                break;
            case 'n':
                xi_numberofpublishes = atoi( optarg );
                break;
            case 'S':
                printf(
                    "-S --messagesize\n\tLength of the message to publish in bytes.\n" );
                break;
            default:
                abort();
        }
    }

    /* Print any unrecognized command line arguments. */
    if ( optind < argc )
    {
        printf(
            "The application could not recognize the following non-option arguments: " );
        while ( optind < argc )
        {
            printf( "%s ", argv[optind++] );
        }
        putchar( '\n' );
    }
    putchar( '\n' );

    if ( 1 == xi_help_flag ) /* Print the usage statement */
    {
        xi_usage( valid_options, options_length );
        return ( -1 ); /* Don't run the application if -h --help was on the commandline */
    }

    return ( 0 );
}
#else
int xi_embedded_parse( xi_embedded_args_t* xi_embedded_args,
                       char* valid_options,
                       const unsigned options_length )
{
    int i = 0;
    for ( ; i < options_length && valid_options[i] != 0; ++i )
    {
        switch ( valid_options[i] )
        {
            case 'q':
                xi_quiet_flag = xi_embedded_args->xi_quiet_flag;
                break;
            case 'R':
                xi_will_retain = xi_embedded_args->xi_will_retain;
                break;
            case 'E':
                xi_abnormalexit_flag = xi_embedded_args->xi_abnormalexit_flag;
                break;
            case 'h':
                xi_help_flag = xi_embedded_args->xi_help_flag;
                break;
            case 'a':
                xi_account_id = xi_embedded_args->xi_account_id;
                break;
            case 'u':
                xi_username = xi_embedded_args->xi_username;
                break;
            case 'P':
                xi_password = xi_embedded_args->xi_password;
                break;
            case 't':
                xi_subscribetopic = xi_embedded_args->xi_subscribetopic;
                break;
            case 'p':
                xi_publishtopic = xi_embedded_args->xi_publishtopic;
                break;
            case 'm':
                printf( "-m --publishmessage\n\tThe message to publish.\n" );
                break;
            case 'r':
                printf( "-r --publishrate\n\tThe rate at which to publish messages in "
                        "seconds.\n" );
                break;
            case 'c':
                printf( "-c --credentials\n\tThe name of a file containing MQTT username "
                        "and password.\n" );
                break;
            case 'T':
                xi_will_topic = xi_embedded_args->xi_will_topic;
                break;
            case 'M':
                xi_will_message = xi_embedded_args->xi_will_message;
                break;
            case 'Q':
                /* will_qos = atoi(optarg); */
                break;
            case 'l':
                xi_memorylimit = xi_embedded_args->xi_memorylimit;
                break;
            case 'n':
                xi_numberofpublishes = xi_embedded_args->xi_numberofpublishes;
                break;
            case 'S':
                printf(
                    "-S --messagesize\n\tLength of the message to publish in bytes.\n" );
                break;
            case ':':
                break;
            default:
                abort();
        }
    }
    /* Don't run the application if the help flag has been set */
    if ( xi_help_flag )
        return -1;
    else
        return 0;
}
#endif /* XI_CROSS_TARGET */

void xi_usage( const char* options, unsigned options_length )
{
    assert( NULL != options );

    /* For debugging printf( "options = %s %d\n", options, options_length ); */

    printf( "Usage:\n" );
    while ( 0 < options_length )
    {
        /* printf( "parsing option %c\n", *options ); */
        switch ( *options )
        {
            case 'a':
                printf( "-a --account-id\n\tProvide an account id registered in "
                        "BluePrint.\n" );
                break;
            case 'u':
                printf( "-u --username\n\tProvide an MQTT username to be used for "
                        "authenticating with the broker.\n" );
                break;
            case 'P':
                printf( "-P --password\n\tProvide an MQTT password to be used for "
                        "authenticating with the broker.\n" );
                break;
            case 't':
                printf( "-t --subscribetopic\n\tThe topic on which to subscribe.\n" );
                break;
            case 'p':
                printf( "-p --publishtopic\n\tThe topic on which to publish.\n" );
                break;
            case 'm':
                printf( "-m --publishmessage\n\tThe message to publish. A shell quoted "
                        "string of characters.\n" );
                break;
            case 'r':
                printf( "-r --publishrate\n\tThe rate at which to publish messages in "
                        "seconds.\n" );
                break;
            case 'q':
                printf( "-q --quiet\n\tDon't print subscribed message contents.\n" );
                break;
            case 'c':
                printf( "-c --credentials\n\tThe name of a file containing MQTT username "
                        "and password.\n" );
                break;
            case 'T':
                printf(
                    "-T --will-topic\n\tThe topic on which to send a last will, in the "
                    "event that the client disconnects unexpectedly.\n" );
                break;
            case 'M':
                printf(
                    "-M --will-message\n\tSpecify a message that will be stored by the "
                    "broker and sent out if this client disconnects unexpectedly. A "
                    "shell quoted string of characters."
                    "\n\tThis must be used in conjunction with --will-topic.\n" );
                break;
            case 'Q':
                printf( "-Q --will-qos\n\tThe QoS to use for the last will.\n"
                        "\tAT_MOST_ONCE  = 0\n"
                        "\tAT_LEAST_ONCE = 1\n"
                        "\tEXACTLY_ONCE  = 2\n"
                        "\tDefaults to 0."
                        "\n\tThis must be used in conjunction with --will-topic.\n" );
                break;
            case 'R':
                printf(
                    "-R --will-retain\n\tIf given, if the client disconnects "
                    "unexpectedly the message sent out will be treated as a retained "
                    "message.\n\tThis must be used in conjunction with --will-topic.\n" );
                break;
            case 'E':
                printf( "-E --exitwithoutclosing\n\tExit the program abnormally.\n" );
                break;
            case 'l':
                printf( "-l --memorylimit\n\tSet the memory limit in bytes.\n" );
                break;
            case 'n':
                printf( "-n --numberofpublishes\n\tNumber of messages to publish before "
                        "closing and exiting.\n" );
                break;
            case 'S':
                printf(
                    "-S --messagesize\n\tLength of the message to publish in bytes.\n" );
                break;

            case 'h': /* Don't print anything for the help option since we're printing
                         usage */
                break;
            case ':': /* We'll skip the ':' character since it's not an option. */
                break;
            case '\0':
                break;
            default:
                printf( "WARNING: Option %c not recognized by usage()\n", *options );
        }
        ++options;
        --options_length;
    }
}
