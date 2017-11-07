/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_TYPES_H__
#define __XIVELY_TYPES_H__
#include <stdint.h>
#include <stddef.h>

#include <xively_mqtt.h>
#include <xively_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XI_INVALID_CONTEXT_HANDLE -1
#define XI_INVALID_TIMED_TASK_HANDLE -1

/**
 * @name    xi_context_handle_t
 * @brief   Internal context handle, no user action required on these type variables
 *          apart from passing them around properly through API calls.
 */
typedef int32_t xi_context_handle_t;

/**
 * @name    xi_timed_task_handle_t
 * @brief   Timed task handle to facilitate timed tasks identification.
 */
typedef int32_t xi_timed_task_handle_t;

/**
 * @name    xi_user_task_callback_t
 * @brief   User custom action which can be passed to the xi_schedule_timed_task
 *          to have it executed in the given time intervals.
 *
 * @param [in]  in_context_handle the context handle provided to xi_schedule_timed_task
 * @param [in]  timed_task_handle is the handle that identifies timed task and it allows
 *              to manipulate the timed task from within the callback
 *              e.g. to cancel the task if it's no longer needed
 * @param [in]  user_data is the data provided to xi_schedule_timed_task
 */
typedef void( xi_user_task_callback_t )( const xi_context_handle_t context_handle,
                                         const xi_timed_task_handle_t timed_task_handle,
                                         void* user_data );

/**
 * @name    xi_user_callback_t
 * @brief   Some API functions notify the application about the result through
 *          callbacks, these callbacks can be passed as xi_user_callback_t to the API.
 *
 * @param [in]  in_context_handle the context handle which was provided to the
 *              original API call
 * @param [in]  data contains API specific information. Before usage it should
 *              be cast to specific type depending on the original API call.
 *              E.g. xi_connect's callback receives xi_connection_data_t*
 *              in the data attribute. Thus its usage is as follows:
 *              xi_connection_data_t* conn_data = (xi_connection_data_t*)data;
 *              See the documentation of the specific API and/or the Examples
 *              what type the data pointer hides.
 * @param [in]  state is the result of the API call. XI_STATE_OK in case of
 *              success. For other error codes please see the Xively User Guide
 *              or Examples.
 */
typedef void( xi_user_callback_t )( xi_context_handle_t in_context_handle,
                                    void* data,
                                    xi_state_t state );

/**
 * @enum xi_sub_call_type_t
 * @brief determines the subscription callback type and the data passed to the user
 * callback through xi_subscription_data_t
 *
 * XI_SUB_UNKNOWN - whenever the type of the call is not known ( user should check the
 * state value )
 * XI_SUBSCRIPTION_DATA_SUBACK - callback is a SUBACK notification thus suback part
 * should be used from the params
 * XI_SUBSCRIPTION_DATA_MESSAGE - callback is a MESSAGE notification thus message part
 * should be used from the params
 */
typedef enum xi_subscription_data_type_e {
    XI_SUB_CALL_UNKNOWN = 0,
    XI_SUB_CALL_SUBACK,
    XI_SUB_CALL_MESSAGE
} xi_sub_call_type_t;

/**
 * @union xi_sub_call_params_u
 * @brief it's a union that describes the type of data that is passed through the user
 * subscription callback
 *
 * suback - contains information important from perspective of processing mqtt suback
 * by user
 *
 * message - contains information important from perspective of processing mqtt
 * message on subscribed topic
 */
typedef union xi_sub_call_params_u {
    struct
    {
        const char* topic;
        xi_mqtt_suback_status_t suback_status;
    } suback;

    struct
    {
        const char* topic;
        const uint8_t* temporary_payload_data; /* automatically free'd when the callback
                                                  returned */
        size_t temporary_payload_data_length;
        xi_mqtt_retain_t retain;
        xi_mqtt_qos_t qos;
        xi_mqtt_dup_t dup_flag;
    } message;
} xi_sub_call_params_t;

#define XI_EMPTY_SUB_CALL_PARAMS                                                         \
    ( xi_sub_call_params_t )                                                             \
    {                                                                                    \
        .message = {                                                                     \
            NULL,                                                                        \
            NULL,                                                                        \
            0,                                                                           \
            XI_MQTT_RETAIN_FALSE,                                                        \
            XI_MQTT_QOS_AT_MOST_ONCE,                                                    \
            XI_MQTT_DUP_FALSE                                                            \
        }                                                                                \
    }

/**
 * @brief subscription callback used to notify user about changes in topic subscription
 * and about messages that has been received on subscribed topics
 *
 * @param in_context_handle - context on which callback has been invoked
 * @param call_type - gives user information about which information type is passed in
 * data parameter
 * @param params - pointer to a structure that holds the details
 * @param topic - name of the topic
 * @param user_data - pointer previously registered via user during the subscription
 * callback registration, may be used for identification or carrying some helper data
 */
typedef void( xi_user_subscription_callback_t )( xi_context_handle_t in_context_handle,
                                                 xi_sub_call_type_t call_type,
                                                 const xi_sub_call_params_t* const params,
                                                 xi_state_t state,
                                                 void* user_data );


typedef void( xi_sft_on_file_downloaded_callback_t )(
    void* data, const char* filename, uint8_t flag_download_finished_successfully );

typedef uint8_t( xi_sft_url_handler_callback_t )(
    const char* url,
    const char* filename,
    xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded,
    void* callback_data );

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_TYPES_H__ */
