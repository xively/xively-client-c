  FREE( state->buffer->data ); 
  FREE( state->buffer );

   if( recvd_msg->header.type != M_PUBACK ) { // wrong message type }
   // take the payload
   // send it or read it
   FREE( recvd_msg );
   END_CORO();
}