#define BEGIN_CORO( state )\
    switch( state )\
    { \
        default:

#define YIELD( state, ret )\
    state = __LINE__; return ret; case __LINE__:

#define YIELD_ON( state, expression, ret )\
{ \
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
    } \
};

#define YIELD_UNTIL( state, expression, ret )\
{ \
    if ( (expression) ) \
    { \
      state = __LINE__; return ret; case __LINE__:; \
      continue; \
    } \
};

#define CORO_END()\
    };
