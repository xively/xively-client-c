requested_state_t process( handler_state_t state )
{
   BEGIN_CORO( state->co );

   ALLOC_AT( message_t, state->data );
   ALLOC_BUFFER_AT( message_t, state->data->data );

   do 
   {
      res = write( state->data + state->offset, 32 );
      if( res <= 0 ) { // do the error handling and maybe YIELD( state->co ); }
      YIELD_UNTIL( state->co, state->sent < data->lenght, WANT_WRITE )
   } while( state->sent < data->length );

   FREE( state->data->data ); FREE( state->data );

   state->timeout = REGISTER_TIMER( make_handle( process, TIMEOUT_STATE ), TIMEOUT );
   YIELD( state->co, WANT_READ ); 

   ALLOC_AT( buffer_t, state->buffer ); ALLOC_BUFFER_AT( state->buffer->data, 32 );
   ALLOC_AT( message_t, state->recvd_msg );

   if( state == TIMEOUT ) { //timeout }
   UNREGISTER_TIMER( state->timeout );

   do 
   {
      res = read( state->buffer->data, 32 );      
      if( res <= 0 ) { // do the error handling and maybe YIELD( state->co ); }
      state->buffer->length = res;
      state->parser_done = parse( state->buffer, state->recvd_msg );
      YIELD_UNTIL( state->co, state->parser_done, WANT_READ );
   } while( state->parser_done );

   FREE( state->buffer->data ); FREE( state->buffer );

   if( recvd_msg->header.type != M_PUBACK ) { // wrong message type }
   // take the payload
   // send it or read it
   FREE( recvd_msg );
   END_CORO();
}