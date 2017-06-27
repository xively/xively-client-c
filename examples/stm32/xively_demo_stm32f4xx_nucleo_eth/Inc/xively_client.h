/******************************************************************************
 *                                                                            *
 *  xively_client.h                                                           *
 *                                                                            *
 ******************************************************************************/

#ifndef  XIVELY_CLIENT_H
#define  XIVELY_CLIENT_H

void xively_client_start();
void pub_button_interrupt( void );

extern int  xc_control;

#endif  /* XIVELY_CLIENT_H */
