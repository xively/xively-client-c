requested_state_t process( handler_state_t state )
{
   BEGIN_CORO( state->co );

   ALLOC_AT( message_t, state->data );
   ALLOC_BUFFER_AT( message_t, state->data->data );

   do 
   {
      res = write( state->data + state->offset, 32 );
      if( res <= 0 ) { // do the error handling 
                       // and maybe YIELD( state->co ); }
      YIELD_UNTIL( state->co
        , state->sent < data->lenght
        , WANT_WRITE )
   } while( state->sent < data->length );

