
/**
 * @brief This function receives a pointer to a pre-allocated buffer to be filled in.
 *        We recommend that string is allocated XI_TOPIC_MAX_LEN bytes
 */
static int8_t build_xively_topic(
    char* topic_name, char* account_id, char* device_id, char* dst, uint32_t dst_len )
{
    int retval = 0;

    assert( NULL != topic_name );
    assert( NULL != account_id );
    assert( NULL != device_id );
    assert( NULL != dst );

    retval = snprintf( dst, dst_len, "xi/blue/v1/%.64s/d/%.64s/%.64s", account_id,
                       device_id, topic_name );

    if ( ( retval >= dst_len ) || ( retval < 0 ) )
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  Build the MQTT topic addresses with the given account_id and device_id
 * @param
 * @retval -1: Error
 * @retval 0: OK
 */
static int8_t update_all_mqtt_topics( char* xi_account_id, char* xi_device_id )
{
    assert( strlen( xi_account_id ) > 0 );
    assert( strlen( xi_device_id ) > 0 );

    if ( 0 > build_xively_topic( ACCELEROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.accelerometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( GYROSCOPE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.gyroscope, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( MAGNETOMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.magnetometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BAROMETER_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.barometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( HUMIDITY_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.hygrometer, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( TEMPERATURE_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.temperature, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( BUTTON_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.button, XI_TOPIC_MAX_LEN ) )
        return -1;

    if ( 0 > build_xively_topic( LED_CHANNEL_NAME, xi_account_id, xi_device_id,
                                 mqtt_topics.led, XI_TOPIC_MAX_LEN ) )
        return -1;

    return 0;
}

xi_state_t pub_accelerometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_accelero( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }

    return xi_publish( gXivelyContextHandle, mqtt_topics.accelerometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_magnetometer( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_magneto( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.magnetometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_gyroscope( void )
{
    char out_msg[IO_AXES_JSON_BUFFER_MAX_SIZE];
    SensorAxes_t sensor_input;

    if ( 0 > io_read_gyro( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_axes_to_json( sensor_input, out_msg, IO_AXES_JSON_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.gyroscope, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_barometer( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_pressure( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.barometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_humidity( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_humidity( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.hygrometer, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_temperature( void )
{
    char out_msg[IO_FLOAT_BUFFER_MAX_SIZE];
    float sensor_input = 0;

    if ( 0 > io_read_temperature( &sensor_input ) )
    {
        return -1;
    }
    if ( 0 > io_float_to_string( sensor_input, out_msg, IO_FLOAT_BUFFER_MAX_SIZE ) )
    {
        return -1;
    }
    return xi_publish( gXivelyContextHandle, mqtt_topics.temperature, out_msg,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

xi_state_t pub_virtual_switch( void )
{
    const char* payload = ( virtual_switch_state == 1 ) ? "1" : "0";

    return xi_publish( gXivelyContextHandle, mqtt_topics.button, payload,
                       XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void pub_button_interrupt( void )
{
    const char* payload = ( virtual_switch_state == 1 ) ? "1" : "0";

    if ( ( int )XI_CONNECTION_STATE_OPENED != ( int )xi_connection_state )
    {
        printf( "\r\n\tInterrupt publish [ABORT] Device not connected to the broker" );
        return;
    }

    xi_publish( gXivelyContextHandle, mqtt_topics.button, payload,
                XI_MQTT_QOS_AT_MOST_ONCE, XI_MQTT_RETAIN_FALSE, NULL, NULL );
}

void publish_mqtt_topics()
{
    if ( !is_burst_mode )
    {
        pub_gyroscope();
        pub_accelerometer();
    }

    pub_virtual_switch();
    pub_magnetometer();
    pub_barometer();
    pub_humidity();
    pub_temperature();
}

void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    /* sanity check */
    assert( NULL != data );

    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;
    xi_connection_state             = conn_data->connection_state;

    printf( "\r\n>> MQTT connection state: %i", conn_data->connection_state );
    switch ( conn_data->connection_state )
    {
        /* connection attempt failed */
        case XI_CONNECTION_STATE_OPEN_FAILED:
            printf( "\r\n>> [%d] Xively Connection [ERROR] Broker: %s:%d, Error: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );
            printf( "\r\n\tReconnecting to the MQTT broker" );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been closed */
        case XI_CONNECTION_STATE_CLOSED:
            /* unregister task handle */
            xi_cancel_timed_task( gXivelyTimedTaskHandle );
            gXivelyTimedTaskHandle = XI_INVALID_TIMED_TASK_HANDLE;

            printf( "\r\n>> [%d] Xively Connection Closed. Broker: %s:%d, Reason: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port, state );
            printf( "\r\n\tReconnecting to the MQTT broker" );

            xi_connect( in_context_handle, conn_data->username, conn_data->password,
                        conn_data->connection_timeout, conn_data->keepalive_timeout,
                        conn_data->session_type, &on_connected );
            break;
        /* connection has been established */
        case XI_CONNECTION_STATE_OPENED:
            printf( "\r\n>> [%d] Xively Connection [ESTABLISHED] Broker: %s:%d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(), conn_data->host,
                    conn_data->port );

            {
                /* here put the activation code */
                init_xively_topics( in_context_handle );
            }

            break;
        /* something really bad happened */
        default:
            printf( "\r\n>> [%d] Xively Internal [ERROR] Invalid connection status: %d",
                    ( int )xi_bsp_time_getcurrenttime_seconds(),
                    ( int )conn_data->connection_state );
    };
}

/* handler for xively mqtt topic subscription */
void on_led_msg( xi_context_handle_t in_context_handle,
                 xi_sub_call_type_t call_type,
                 const xi_sub_call_params_t* const params,
                 xi_state_t state,
                 void* user_data )
{
    switch ( call_type )
    {
        case XI_SUB_CALL_SUBACK:
            if ( XI_MQTT_SUBACK_FAILED == params->suback.suback_status )
            {
                printf( "\r\n>> Subscription to LED topic [FAIL]" );
            }
            else
            {
                printf( "\r\n>> Subscription to LED topic [OK]" );
            }
            return;
        case XI_SUB_CALL_MESSAGE:
            printf( "\r\n>> received message on LED topic" );
            printf( "\r\n\t Message size: %d",
                    params->message.temporary_payload_data_length );
            printf( "\r\n\t Message payload: " );
            for ( size_t i = 0; i < params->message.temporary_payload_data_length; i++ )
            {
                printf( "0x%02x ", params->message.temporary_payload_data[i] );
            }

            if ( params->message.temporary_payload_data_length != 1 )
            {
                printf( "\r\n\tUnrecognized LED message [IGNORED] Expected 1 or 0" );
                return;
            }

            if ( params->message.temporary_payload_data[0] == '0' )
            {
                io_led_off();
            }
            else
            {
                io_led_on();
            }
            break;
        default:
            return;
    }
}

/*
 * Initializes Xively's demo application topics
 */
static xi_state_t init_xively_topics( xi_context_handle_t in_context_handle )
{
    xi_state_t ret = XI_STATE_OK;

    if ( 0 >
         update_all_mqtt_topics( user_config.xi_account_id, user_config.xi_device_id ) )
    {
        printf( "\r\n>> Topic composition [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    printf( "\r\n>> Subscribing to MQTT topic [%s]", mqtt_topics.led );
    ret = xi_subscribe( in_context_handle, mqtt_topics.led, XI_MQTT_QOS_AT_MOST_ONCE,
                        &on_led_msg, NULL );

    if ( XI_STATE_OK != ret )
    {
        printf( "\r\n>> Xively subscription [ERROR]" );
        return XI_MQTT_SUBSCRIPTION_FAILED;
    }

    /* registration of the publish function */
    gXivelyTimedTaskHandle = xi_schedule_timed_task(
        in_context_handle, &publish_mqtt_topics, XI_PUBLISH_INTERVAL_SEC, 1, NULL );

    if ( XI_INVALID_TIMED_TASK_HANDLE == gXivelyTimedTaskHandle )
    {
        printf( "\r\n>> Libxively timed task registration [ERROR]" );
        return XI_FAILED_INITIALIZATION;
    }

    return ret;
}


