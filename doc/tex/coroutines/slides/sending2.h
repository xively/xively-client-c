      case STATE_SEND_DATA:
      {
         if( e == CAN_WRITE )
         {
            while( sent < data->length )
            {
               int res = write( data->data + sent, data->length - sent );

               if( res <= 0 )
               {
                  // do the error handling and maybe leave
               }

               sent += res;
            }

            state       = STATE_RECV_DATA;
            timer_value = 0;
         }
      }
      break;
