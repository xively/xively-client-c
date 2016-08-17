      case STATE_RECV_DATA:
      {
         if( recvd_msg == NULL ) { ALLOC_AT( M_protocol_t, recvd_msg ); }

         if( e == CAN_READ )
         {
            if( timer_value > TIMEOUT ) { // timeout!!! }

            while( parser_not_done == true )
            {
               ALLOC( buffer_t, recv_data ); 
               ALLOC_AT( recv_data->data, 32 );
               
               int res = read( recv_data->data, 32 );
               if( res <= 0 ) { // do the error handling }

               recv_data->length             = res;
               parser_result_t parser_result = call_parser( recv_data
                  , recvd_msg
                  , &parser_not_done );

               if( parser_result == PARSER_ERROR ) { // do the error handling }
               FREE( recv_data->data );
               FREE( recv_data );
            }         
         }

         state = ANALYSE;
      }
      break;
