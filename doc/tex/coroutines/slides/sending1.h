void handle_events( buffer_t* data, event_t e, uint32_t time_diff )
{
   static state_t state = STATE_SEND_DATA;
   static uint16_t sent = 0;
   static uint32_t timer_value = 0;
   static bool parser_not_done = false;

   timer_value += time_diff;

   switch( state )
   {
      case STATE_INIT:
      {
         state             = STATE_SEND_DATA;
         sent              = 0;
         parser_not_done   = false;
      }
      break;