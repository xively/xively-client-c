do
{
    layer_data->local_state = mqtt_parser_execute(
	  &layer_data->parser
	, layer_data->msg
	, data_descriptor );

    YIELD_UNTIL( layer_data->on_data_ready_cs
	, ( layer_data->local_state == LAYER_STATE_WANT_READ )
	, CALL_ON_PREV_ON_DATA_READY( context
	    , data_descriptor
	    , layer_data->local_state ) );
} while( in_out_state != LAYER_STATE_ERROR
      && layer_data->local_state == LAYER_STATE_WANT_READ );