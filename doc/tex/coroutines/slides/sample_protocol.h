typedef enum 
{
   M_UNKNOWN = 0, M_PUBLISH, M_PUBACK
} M_message_types;

typedef struct
{
   M_message_types type;
} M_header_t;
