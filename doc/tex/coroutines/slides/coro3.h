   ALLOC_AT( buffer_t, state->buffer ); 
   ALLOC_BUFFER_AT( state->buffer->data, 32 );
   ALLOC_AT( message_t, state->recvd_msg );

   if( state == TIMEOUT ) { //timeout }
   UNREGISTER_TIMER( state->timeout );

   do 
   {
      res = read( state->buffer->data, 32 );      
      if( res <= 0 ) { // do the error handling 
                       //and maybe YIELD( state->co ); }
      state->buffer->length = res;
      state->parser_done = parse( state->buffer, state->recvd_msg );
      YIELD_UNTIL( state->co, state->parser_done, WANT_READ );
   } while( state->parser_done );

 