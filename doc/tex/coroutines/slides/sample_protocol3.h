typedef struct 
{
   M_header_t header;
   union
   {
      M_publish_t publish;
      M_puback_t  puback;
   } types;
} M_protocol_t;