
#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os2.h" 

typedef union tokenType_{
	struct __attribute__((__packed__)){
		uint8_t token_id;
		uint8_t stations[MAX_STATION_NB];
	};
	uint8_t token_data[MAX_STATION_NB + 1];
}tokenType;


const osMessageQueueAttr_t queue_mac_buffer_attr = {
	.name = "MAC_BUFFER"
};
osMessageQueueId_t  queue_mac_buffer_id;


void MacSender(void *argument){
	
	
	
	queue_mac_buffer_id = osMessageQueueNew(16,sizeof(struct queueMsg_t),&queue_mac_buffer_attr);
	
	
	// TODO
	while(1){
		osStatus_t retCode;
		
		struct queueMsg_t queueMsgMacS;					// queue message from any
		struct queueMsg_t queueMsgPhyS;					// queue message to phy
		
		
		retCode = osMessageQueueGet(queue_macS_id, &queueMsgMacS, NULL, osWaitForever);
		CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
		
		switch(queueMsgMacS.type){
			case NEW_TOKEN:
			{
				tokenType token;
				token.token_id = TOKEN_TAG;
				
				
				
				for(int i = 0; i<MAX_STATION_NB; i++){ // init all stations
					token.stations[i]= 0x00;
				}
				token.stations[MYADDRESS-1] = ((1 << CHAT_SAPI) | (1 << TIME_SAPI)); // set my services				
				
				
				queueMsgPhyS.type = TO_PHY;			// message type
				queueMsgPhyS.anyPtr = &(token.token_data);					// pointer to token
				//------------------------------------------------------------------------
				// QUEUE SEND	(send TO_PHY to PHY sender)
				//------------------------------------------------------------------------
				retCode = osMessageQueuePut(
					queue_phyS_id,
					&queueMsgPhyS,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
			}
				break;
			
			case START:			
			case STOP:
			case DATA_IND:
				
				retCode = osMessageQueuePut(
					queue_mac_buffer_id,
					&queueMsgMacS,
					osPriorityNormal,
					0);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
				break;
			
			case TOKEN:
					
				break;
			
			case DATABACK:
				
				break;
			
			default:
				break;
		}
	}
	
}
