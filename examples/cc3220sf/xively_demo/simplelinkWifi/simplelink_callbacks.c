/*
 * simplelinke_callbacks.c
 *
 *  Created on: Jan 6, 2017
 *      Author: Owner
 */

#include <mqueue.h>

#include <xively_example.h>

/* message queue for control messages */
mqd_t controlMQueue;


//*****************************************************************************
//
//! The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    switch(pWlanEvent->Id)
    {
        case SL_WLAN_EVENT_CONNECT:
        {
            SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Connection);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6gAcquired);

            /*
             Information about the connected AP (like name, MAC etc) will be
             available in 'slWlanConnectAsyncResponse_t'-Applications
             can use it if required:

              slWlanConnectAsyncResponse_t *pEventData = NULL;
              pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            */

            /* Copy new connection SSID and BSSID to global parameters */
            memcpy(gApplicationControlBlock.connectionSSID,
                    pWlanEvent->Data.Connect.SsidName,
                    pWlanEvent->Data.Connect.SsidLen);

            gApplicationControlBlock.ssidLen = pWlanEvent->Data.Connect.SsidLen;

            memcpy(gApplicationControlBlock.connectionBSSID,
                    pWlanEvent->Data.Connect.Bssid,
                    SL_WLAN_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s ,"
                        "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                            gApplicationControlBlock.connectionSSID,
                            gApplicationControlBlock.connectionBSSID[0], gApplicationControlBlock.connectionBSSID[1],
                            gApplicationControlBlock.connectionBSSID[2], gApplicationControlBlock.connectionBSSID[3],
                            gApplicationControlBlock.connectionBSSID[4], gApplicationControlBlock.connectionBSSID[5]);

            sem_post(&Provisioning_ControlBlock.connectionAsyncEvent);
        }
        break;

        case SL_WLAN_EVENT_DISCONNECT:
        {
            SlWlanEventDisconnect_t*    pEventData = NULL;

            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Connection);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired);
            CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6gAcquired);

            pEventData = &pWlanEvent->Data.Disconnect;

            /* 	If the user has initiated 'Disconnect' request,
             	'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED.
             */
            if(SL_WLAN_DISCONNECT_USER_INITIATED == pEventData->ReasonCode)
            {
                UART_PRINT("[WLAN EVENT]Device disconnected from the "
                            "AP: %s, BSSID: %x:%x:%x:%x:%x:%x "
                            "on application's request \n\r",
                            gApplicationControlBlock.connectionSSID,
                            gApplicationControlBlock.connectionBSSID[0], gApplicationControlBlock.connectionBSSID[1],
                            gApplicationControlBlock.connectionBSSID[2], gApplicationControlBlock.connectionBSSID[3],
                            gApplicationControlBlock.connectionBSSID[4], gApplicationControlBlock.connectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                            "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                            gApplicationControlBlock.connectionSSID,
                            gApplicationControlBlock.connectionBSSID[0], gApplicationControlBlock.connectionBSSID[1],
                            gApplicationControlBlock.connectionBSSID[2], gApplicationControlBlock.connectionBSSID[3],
                            gApplicationControlBlock.connectionBSSID[4], gApplicationControlBlock.connectionBSSID[5]);
            }
            memset(gApplicationControlBlock.connectionSSID, 0, sizeof(gApplicationControlBlock.connectionSSID));
            memset(gApplicationControlBlock.connectionBSSID, 0, sizeof(gApplicationControlBlock.connectionBSSID));
        }
        break;

	 case SL_WLAN_EVENT_STA_ADDED:
        {
        	UART_PRINT("[WLAN EVENT] External Station connected to SimpleLink AP\r\n");

        	UART_PRINT("[WLAN EVENT] STA BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        						   pWlanEvent->Data.STAAdded.Mac[0],
        						   pWlanEvent->Data.STAAdded.Mac[1],
        						   pWlanEvent->Data.STAAdded.Mac[2],
        						   pWlanEvent->Data.STAAdded.Mac[3],
        						   pWlanEvent->Data.STAAdded.Mac[4],
        						   pWlanEvent->Data.STAAdded.Mac[5]);

        }
        break;

        case SL_WLAN_EVENT_STA_REMOVED:
        {
        	UART_PRINT("[WLAN EVENT] External Station disconnected from SimpleLink AP\r\n");
        }
        break;

        case SL_WLAN_EVENT_PROVISIONING_PROFILE_ADDED:
        {
            UART_PRINT("[WLAN EVENT] Profile Added\r\n");
        }
        break;

		case SL_WLAN_EVENT_PROVISIONING_STATUS:
		{
            _u16 status = pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus;
            switch(status)
			{
				case SL_WLAN_PROVISIONING_GENERAL_ERROR:
				case SL_WLAN_PROVISIONING_ERROR_ABORT:
				{
					UART_PRINT("[WLAN EVENT] Provisioning Error status=%d\r\n",status);
					SignalProvisioningEvent(PrvnEvent_Error);
				}
				break;
				case SL_WLAN_PROVISIONING_ERROR_ABORT_INVALID_PARAM:
				case SL_WLAN_PROVISIONING_ERROR_ABORT_HTTP_SERVER_DISABLED:
				case SL_WLAN_PROVISIONING_ERROR_ABORT_PROFILE_LIST_FULL:
				{
					UART_PRINT("[WLAN EVENT] Provisioning Error status=%d\r\n",status);
					SignalProvisioningEvent(PrvnEvent_StartFailed);
				}
				break;
				case SL_WLAN_PROVISIONING_ERROR_ABORT_PROVISIONING_ALREADY_STARTED:
                {
                	UART_PRINT("[WLAN EVENT] Provisioning already started");
                }
				break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_NETWORK_NOT_FOUND:
                {
                    UART_PRINT("[WLAN EVENT] Confirmation fail: network not found\r\n");
                    SignalProvisioningEvent(PrvnEvent_ConfirmationFailed);
                }
                break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_FAIL_CONNECTION_FAILED:
                {
                    UART_PRINT("[WLAN EVENT] Confirmation fail: Connection failed\r\n");
                    SignalProvisioningEvent(PrvnEvent_ConfirmationFailed);
                }
                break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_CONNECTION_SUCCESS_IP_NOT_ACQUIRED:
                {
                    UART_PRINT("[WLAN EVENT] Confirmation fail: IP address not acquired\r\n");
                    SignalProvisioningEvent(PrvnEvent_ConfirmationFailed);
                }
                break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS_FEEDBACK_FAILED:
                {
                    UART_PRINT("[WLAN EVENT] Connection Success (feedback to Smartphone app failed)\r\n");
                    SignalProvisioningEvent(PrvnEvent_ConfirmationFailed);
                }
                break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_STATUS_SUCCESS:
                {
                    UART_PRINT("[WLAN EVENT] Confirmation Success!\r\n");
                    SignalProvisioningEvent(PrvnEvent_ConfirmationSuccess);
                }
                break;

                case SL_WLAN_PROVISIONING_AUTO_STARTED:
                {
                    UART_PRINT("[WLAN EVENT] Auto-Provisioning Started\r\n");
                    /* stop auto provisioning - may trigger in case of returning to default */
                    SignalProvisioningEvent(PrvnEvent_Stopped);
                }
                break;

                case SL_WLAN_PROVISIONING_STOPPED:
                {
					UART_PRINT("[WLAN EVENT] Provisioning stopped\r\n");
					if(ROLE_STA == pWlanEvent->Data.ProvisioningStatus.Role)
					{
						UART_PRINT(" [WLAN EVENT] - WLAN Connection Status:%d\r\n",pWlanEvent->Data.ProvisioningStatus.WlanStatus);

						if(SL_WLAN_STATUS_CONNECTED == pWlanEvent->Data.ProvisioningStatus.WlanStatus)
						{
							UART_PRINT(" [WLAN EVENT] - Connected to SSID:%s\r\n",pWlanEvent->Data.ProvisioningStatus.Ssid);

							memcpy (gApplicationControlBlock.connectionSSID, pWlanEvent->Data.ProvisioningStatus.Ssid, pWlanEvent->Data.ProvisioningStatus.Ssidlen);
							gApplicationControlBlock.ssidLen = pWlanEvent->Data.ProvisioningStatus.Ssidlen;

							/* Provisioning is stopped by the device and provisioning is done successfully */
							SignalProvisioningEvent(PrvnEvent_Stopped);

							break;
						}
						else
						{
							CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Connection);
							CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);
							CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired);
							CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6gAcquired);

							/* Provisioning is stopped by the device and provisioning is not done yet, still need to connect to AP */
							SignalProvisioningEvent(PrvnEvent_WaitForConn);

							break;
						}
					}
                }

                SignalProvisioningEvent(PrvnEvent_Stopped);

                break;

                case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNCED:
                {
                    UART_PRINT("[WLAN EVENT] Smart Config Synced!\r\n");
                }
                break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_WLAN_CONNECT:
				{
					SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Connection);
					CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);
					CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired);
					CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6gAcquired);

					UART_PRINT("[WLAN EVENT] Connection to AP succeeded\r\n");
				}
				break;

                case SL_WLAN_PROVISIONING_CONFIRMATION_IP_ACQUIRED:
				{
					SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);

					UART_PRINT("[WLAN EVENT] IP address acquired\r\n");
				}
				break;

                case SL_WLAN_PROVISIONING_SMART_CONFIG_SYNC_TIMEOUT:
                {
					UART_PRINT("[WLAN EVENT] Smart Config Sync timeout\r\n");
				}
				break;

                default:
                {
                    UART_PRINT("[WLAN EVENT] Unknown Provisioning Status: %d\r\n",pWlanEvent->Data.ProvisioningStatus.ProvisioningStatus);
                }
                break;
            }
        }
        break;

        default:
        {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                                                            pWlanEvent->Id);

            SignalProvisioningEvent(PrvnEvent_Error);
        }
        break;
    }
}

//*****************************************************************************
//
//! The Function Handles the Fatal errors
//!
//! \param[in]  slFatalErrorEvent - Pointer to Fatal Error Event info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
{
	_u8 msg = 4;
	_i32 msgqRetVal;

    switch (slFatalErrorEvent->Id)
    {
        case SL_DEVICE_EVENT_FATAL_DEVICE_ABORT:
        {
            UART_PRINT("[ERROR] - FATAL ERROR: Abort NWP event detected: "
                        "AbortType=%d, AbortData=0x%x\n\r",
                        slFatalErrorEvent->Data.DeviceAssert.Code,
                        slFatalErrorEvent->Data.DeviceAssert.Value);
        }
        break;

        case SL_DEVICE_EVENT_FATAL_DRIVER_ABORT:
        {
            UART_PRINT("[ERROR] - FATAL ERROR: Driver Abort detected. \n\r");
        }
        break;

        case SL_DEVICE_EVENT_FATAL_NO_CMD_ACK:
        {
            UART_PRINT("[ERROR] - FATAL ERROR: No Cmd Ack detected "
                       "[cmd opcode = 0x%x] \n\r",
                       slFatalErrorEvent->Data.NoCmdAck.Code);
        }
        break;

        case SL_DEVICE_EVENT_FATAL_SYNC_LOSS:
        {
            UART_PRINT("[ERROR] - FATAL ERROR: Sync loss detected n\r");
        }
        break;

        case SL_DEVICE_EVENT_FATAL_CMD_TIMEOUT:
        {
            UART_PRINT("[ERROR] - FATAL ERROR: Async event timeout detected "
                       "[event opcode =0x%x]  \n\r",
                       slFatalErrorEvent->Data.CmdTimeout.Code);
        }
        break;

		default:
			UART_PRINT("[ERROR] - FATAL ERROR: Unspecified error detected \n\r");
			break;
    }

	msgqRetVal = mq_send(controlMQueue, (char *)&msg, 1, 0);
	if(msgqRetVal < 0)
	{
		UART_PRINT("[Control task] could not send element to msg queue\n\r");
		while (1);
	}
}

//*****************************************************************************
//
//! This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
	SlNetAppEventData_u *pNetAppEventData = NULL;

	if (NULL == pNetAppEvent)
	{
		return;
	}

	pNetAppEventData = &pNetAppEvent->Data;

	 switch(pNetAppEvent->Id)
	{
		case SL_NETAPP_EVENT_IPV4_ACQUIRED:
		{
			SlIpV4AcquiredAsync_t   *pEventData = NULL;

			SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpAcquired);

			/* Ip Acquired Event Data */
			pEventData = &pNetAppEvent->Data.IpAcquiredV4;

			/* Gateway IP address */
			gApplicationControlBlock.gatewayIP = pEventData->Gateway;
			UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
						"Gateway=%d.%d.%d.%d\n\r",
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,3),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,2),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,1),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,0),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,3),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,2),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,1),
						SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Gateway,0));

			sem_post(&Provisioning_ControlBlock.connectionAsyncEvent);
		}
		break;

		case SL_NETAPP_EVENT_IPV6_ACQUIRED:
		{
			if(!GET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired))
			{
				SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6lAcquired);
				UART_PRINT("[NETAPP EVENT] Local IPv6 Acquired\n\r");
			}
			else
			{
				SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_Ipv6gAcquired);
				UART_PRINT("[NETAPP EVENT] Global IPv6 Acquired\n\r");
			}

			sem_post(&Provisioning_ControlBlock.connectionAsyncEvent);
		}
		break;

		case SL_NETAPP_EVENT_DHCPV4_LEASED:
		{
			SET_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpLeased);

			UART_PRINT("[NETAPP EVENT] IPv4 leased %d.%d.%d.%d for device %02x:%02x:%02x:%02x:%02x:%02x\n\r",\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpLeased.IpAddress,3),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpLeased.IpAddress,2),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpLeased.IpAddress,1),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpLeased.IpAddress,0),\
				pNetAppEventData->IpLeased.Mac[0],\
				pNetAppEventData->IpLeased.Mac[1],\
				pNetAppEventData->IpLeased.Mac[2],\
				pNetAppEventData->IpLeased.Mac[3],\
				pNetAppEventData->IpLeased.Mac[4],\
				pNetAppEventData->IpLeased.Mac[5]);

		}
		break;

        case SL_NETAPP_EVENT_DHCPV4_RELEASED:
        {
        	CLR_STATUS_BIT(gApplicationControlBlock.status, AppStatusBits_IpLeased);

        	UART_PRINT("[NETAPP EVENT] IPv4 released %d.%d.%d.%d for device %02x:%02x:%02x:%02x:%02x:%02x\n\r",\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpReleased.IpAddress,3),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpReleased.IpAddress,2),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpReleased.IpAddress,1),\
				(_u8)SL_IPV4_BYTE(pNetAppEventData->IpReleased.IpAddress,0),\
				pNetAppEventData->IpReleased.Mac[0],\
				pNetAppEventData->IpReleased.Mac[1],\
				pNetAppEventData->IpReleased.Mac[2],\
				pNetAppEventData->IpReleased.Mac[3],\
				pNetAppEventData->IpReleased.Mac[4],\
				pNetAppEventData->IpReleased.Mac[5]);

			UART_PRINT("Reason: ");
			switch(pNetAppEventData->IpReleased.Reason)
			{
				case SL_IP_LEASE_PEER_RELEASE: UART_PRINT("Peer released\n\r");
				break;

				case SL_IP_LEASE_PEER_DECLINE: UART_PRINT("Peer declined\n\r");
				break;

				case SL_IP_LEASE_EXPIRED: UART_PRINT("Lease expired\n\r");
				break;
			}
        }
        break;

        case SL_NETAPP_EVENT_DHCP_IPV4_ACQUIRE_TIMEOUT:
        {
            UART_PRINT("[NETAPP EVENT] DHCP IPv4 Acquire timeout\n\r");
        }
        break;

		default:
		{
			UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
					   pNetAppEvent->Id);
		}
		break;
	}
}


//*****************************************************************************
//
//! This function handles HTTP server events
//!
//! \param[in]  pServerEvent    - Contains the relevant event information
//! \param[in]  pServerResponse - Should be filled by the user with the
//!                               relevant response information
//!
//! \return None
//!
//****************************************************************************
void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent,
                                SlNetAppHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
	UART_PRINT("[HTTP SERVER EVENT] Unexpected HTTP server event \n\r");
}

//*****************************************************************************
//
//! This function handles General Events
//!
//! \param[in]  pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
	_u8 msg = 4;
	_i32 msgqRetVal;

    /*
        Most of the general errors are not FATAL are are to be handled
        appropriately by the application.
    */
    if(NULL == pDevEvent) return;
    switch(pDevEvent->Id)
    {
        case SL_DEVICE_EVENT_RESET_REQUEST:
        {
        	UART_PRINT("[GENERAL EVENT] Reset Request Event\r\n");
        }
        break;

        default:
        {
        	UART_PRINT("[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
                                                pDevEvent->Data.Error.Code,
                                                pDevEvent->Data.Error.Source);

        	msgqRetVal = mq_send(controlMQueue, (char *)&msg, 1, 0);
			if(msgqRetVal < 0)
			{
					UART_PRINT("[Control task] could not send element to msg queue\n\r");
					while (1);
			}
		}
		break;
	}
}


//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]  pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
	if(SL_SOCKET_ASYNC_EVENT == pSock->Event)
	{
		UART_PRINT("[SOCK ERROR] an event received on socket %d\r\n",pSock->SocketAsyncEvent.SockAsyncData.Sd);
		switch(pSock->SocketAsyncEvent.SockAsyncData.Type)
		{
		case SL_SSL_NOTIFICATION_CONNECTED_SECURED:
		UART_PRINT("[SOCK ERROR] SSL handshake done");
			break;
		case SL_SSL_NOTIFICATION_HANDSHAKE_FAILED:
		UART_PRINT("[SOCK ERROR] SSL handshake failed with error %d\r\n", pSock->SocketAsyncEvent.SockAsyncData.Val);
			break;
		case SL_SSL_ACCEPT:
			UART_PRINT("[SOCK ERROR] Recoverable error occurred during the handshake %d\r\n",pSock->SocketAsyncEvent.SockAsyncData.Val);
			break;
		case SL_OTHER_SIDE_CLOSE_SSL_DATA_NOT_ENCRYPTED:
			UART_PRINT("[SOCK ERROR] Other peer terminated the SSL layer.\r\n");
			break;
		case SL_SSL_NOTIFICATION_WRONG_ROOT_CA:
			UART_PRINT("[SOCK ERROR] Used wrong CA to verify the peer.\r\n");

			break;
		default:
			break;
		}
	}

	/* This application doesn't work w/ socket - Events are not expected */
	 switch( pSock->Event )
	 {
	   case SL_SOCKET_TX_FAILED_EVENT:
		   switch( pSock->SocketAsyncEvent.SockTxFailData.Status)
		   {
			  case SL_ERROR_BSD_ECLOSE:
				   UART_PRINT("[SOCK ERROR] - close socket (%d) operation "
								"failed to transmit all queued packets\n\r",
								pSock->SocketAsyncEvent.SockTxFailData.Sd);
			   break;
			   default:
					UART_PRINT("[SOCK ERROR] - TX FAILED  :  socket %d , "
								"reason (%d) \n\n",
								pSock->SocketAsyncEvent.SockTxFailData.Sd,
								pSock->SocketAsyncEvent.SockTxFailData.Status);
			   break;
		   }
		   break;

	   default:
			UART_PRINT("[SOCK EVENT] - Unexpected Event [%x0x]\n\n",pSock->Event);
			break;
	 }
}

void SimpleLinkNetAppRequestMemFreeEventHandler (uint8_t *buffer)
{
	/* Unused in this application */
}
