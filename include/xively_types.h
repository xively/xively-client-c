/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
 * @name  xi_user_subscription_callback_t
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

/**
 * @name  xi_sft_on_file_downloaded_callback_t
 * @brief At a the end of a Secure File Transfer (SFT) HTTP file download the application
 * should report back the result to the Xively C Client through such typed function.
 *
 * The application calls this function reporting the result of the SFT file HTTP download.
 * This function is the trigger for continuing the update package download inside the
 * Xively C Client. The application receives this function pointer during a request for
 * HTTP download, in the application callback (`xi_sft_url_handler_callback_t` see below).
 *
 * @param [in] callback_data the data pointer received as argument `callback_data`
 *                           previously in the call of HTTP downloader function
 *                           `xi_sft_url_handler_callback_t`. The application should not
 *                           do anything with this pointer *just* pass it back.
 * @param [in] filename the name of the file that was downloaded and stored on the
 *                      non-volatile storage of the device
 * @param flag_download_finished_successfully the result of the HTTP download:
 *                                            1 - if file download finished successfully,
 *                                            0 - if file download failed. In latter case
 *                                                the Xively C Client will try to
 *                                                fallback to MQTT file download.
 */
typedef void( xi_sft_on_file_downloaded_callback_t )(
    void* callback_data,
    const char* filename,
    uint8_t flag_download_finished_successfully );

/**
 * @name  xi_sft_url_handler_callback_t
 * @brief application entry point type for HTTP download of update package files
 *
 * The application must define and pass a pointer to a function with this signature only
 * if Secure File Update (SFT) package contains files too large to download over MQTT.
 * This function is called with each and every file in the update package (*even for MQTT
 * provided ones!*) and responsible to start a file download and then report the result
 * back to Xively C Client.
 * The function may reject the HTTP download, in such case the Xively C Client
 * tries to fallback to MQTT download. This is independently true for each and every
 * file in the update package. The HTTP or MQTT file download way is indifferent
 * from the update process point of view.
 * Please see the POSIX example for SFT with HTTP download in
 * `examples/firmware_update/src/firmware_update.c`
 *
 * This function should return immediately and NOT block. The download process should
 * take place on a separate thread spawned by the application. But the callback
 * (`fn_on_file_downloaded_callback`) to the Xively C Client should be called on the
 * Client's thread and NOT on the download thread.
 * A task might be scheduled to poll download status with API function:
 * `xi_schedule_timed_task`.
 *
 * Or on low resource devices *advanced developers* can solve the download
 * on the Xively C Client's thread downloading the file in chunks with non-blocking
 * socket and with the help of Xively C Client's scheduled task through the
 * `xi_schedule_timed_task` *API function. In this case the download task should
 * comply to the cooperative multitasking principle: return in a very short
 * period of time.
 *
 * @param [in] url the file can be downloaded from this URL
 * @param [in] filename the the downloaded file must be saved with this filename onto the
 *                      device's non-volatile storage
 * @param [in] checksum the checksum of the file set in CPM. The application
 *                      is responsible to validate the checksum. To do this one may use
 *                      the functions in `xi_bsp_fwu.h:xi_bsp_fwu_checksum_*`.
 * @param [in] checksum_len the number of bytes the checksum consists of
 * @param [in] flag_mqtt_download_available 1 - if internal MQTT download fallback is
 *                                              available and will happen if this
 *                                              function fails the download
 *                                          0 - if MQTT download isn't available at all
 * @param [in] fn_on_file_downloaded_callback pointer to a Xively C Client callback. The
 *                                            application has to call this function after
 *                                            download finishes.
 * @param [in] callback_data a pointer required to be passed back to the
 *                           `fn_on_file_downloaded_callback`. The application
 *                           shouldn't do anything with this data pointer, just pass
 *                           back to the callback.
 *
 * @retval 1 - if HTTP download is started by the application
 * @retval 0 - if HTTP download is rejectedby the application. If
 *             `flag_mqtt_download_available` is `1` the Xively C Client will
 *             fall back to MQTT download through the SFT service.
 */
typedef uint8_t( xi_sft_url_handler_callback_t )(
    const char* url,
    const char* filename,
    uint8_t* checksum,
    uint16_t checksum_len,
    uint8_t flag_mqtt_download_available,
    xi_sft_on_file_downloaded_callback_t* fn_on_file_downloaded_callback,
    void* callback_data );

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_TYPES_H__ */
