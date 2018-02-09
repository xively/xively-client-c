/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_control_topic_name.h"
#include "xi_types.h"
#include "xi_helpers.h"
#include "xi_err.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XI_CONTROL_TOPIC_ENABLED
const char* const XI_TOPIC_DOMAIN = "xi";

static const char* const control_topic_service_id = "ctrl";
static const char* const control_topic_version_id = "v1";

static const char* const channel_name_publish   = "svc";
static const char* const channel_name_subscribe = "cln";

static const char* const control_topic_name_pat = "%s/%s/%s/%s/%s";
static const char* const device_id_is_missing   = "device-id-is-missing";

static xi_state_t xi_control_topic_allocate_and_snprintf( char** topic_name,
                                                          const char* const device_id,
                                                          const char* const channel_name )
{
    xi_state_t local_state = XI_STATE_OK;
    int result_snprintf    = 0;

    int len = 4; /* the length is the sum of the component names and four '/'
                  * separator characters. See control_topic_name_pat */

    len += strlen( XI_TOPIC_DOMAIN );
    len += strlen( control_topic_service_id );
    len += strlen( control_topic_version_id );
    len += strlen( device_id );
    len += strlen( channel_name );
    len += 1; /* trailing 0 */

    XI_ALLOC_BUFFER_AT( char, *topic_name, len, local_state );

    result_snprintf = snprintf( *topic_name, len, control_topic_name_pat, XI_TOPIC_DOMAIN,
                                control_topic_service_id, control_topic_version_id,
                                device_id, channel_name );

    if ( result_snprintf < 0 || result_snprintf >= len )
    {
        local_state = XI_INTERNAL_ERROR;
    }

    return local_state;

err_handling:
    if ( *topic_name != NULL )
    {
        XI_SAFE_FREE( *topic_name );
    }
    return local_state;
}

xi_state_t xi_control_topic_create_topic_name( const char* device_id,
                                               char** subscribe_topic_name,
                                               char** publish_topic_name )
{
    if ( subscribe_topic_name == NULL || *subscribe_topic_name != NULL ||
         publish_topic_name == NULL || *publish_topic_name != NULL )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t local_state = XI_STATE_OK;

    local_state = xi_control_topic_allocate_and_snprintf(
        subscribe_topic_name, ( device_id == NULL ) ? device_id_is_missing : device_id,
        channel_name_subscribe );

    XI_CHECK_STATE( local_state );

    local_state = xi_control_topic_allocate_and_snprintf(
        publish_topic_name, ( device_id == NULL ) ? device_id_is_missing : device_id,
        channel_name_publish );

    XI_CHECK_STATE( local_state );

    return local_state;

err_handling:
    local_state = XI_INTERNAL_ERROR;

    if ( *subscribe_topic_name != NULL )
    {
        XI_SAFE_FREE( *subscribe_topic_name );
    }

    if ( *publish_topic_name != NULL )
    {
        XI_SAFE_FREE( *publish_topic_name );
    }

    return local_state;
}

#endif

#ifdef __cplusplus
}
#endif
