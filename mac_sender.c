
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

tokenType currentToken;


const osMessageQueueAttr_t queue_mac_buffer_attr = {
	.name = "MAC_BUFFER"
};
osMessageQueueId_t  queue_mac_buffer_id;


msgType* msgToSend;

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
				gTokenInterface.station_list[MYADDRESS] = ((1 << CHAT_SAPI) | (1 << TIME_SAPI)); // set my services
				token.stations[MYADDRESS] = gTokenInterface.station_list[MYADDRESS];
				
				
				queueMsgPhyS.type = TO_PHY;			// message type
				queueMsgPhyS.anyPtr = &(token.token_data);					// pointer to token
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
			{		
				currentToken = *((tokenType*)(queueMsgMacS.anyPtr)); // save the current token
				
				for(uint8_t i = 0; i<MAX_STATION_NB; i++){ // update token list
					gTokenInterface.station_list[i] = currentToken.stations[i];
				}
				
			
				
				struct queueMsg_t bufferedMessage;					// message in fifo
				bool messageRead = false;
				while(!messageRead){
					retCode = osMessageQueueGet(queue_mac_buffer_id, &bufferedMessage, NULL, 0);
					if(retCode == osOK){
							switch(bufferedMessage.type){
								
								case START:
										gTokenInterface.connected = true;
										gTokenInterface.station_list[MYADDRESS] = ((1 << CHAT_SAPI) | (1 << TIME_SAPI)); // set my services
										currentToken.stations[MYADDRESS-1] = gTokenInterface.station_list[MYADDRESS];
									break;
								
								case STOP:
									gTokenInterface.connected = false;
									gTokenInterface.station_list[MYADDRESS] = ((0 << CHAT_SAPI) | (1 << TIME_SAPI)); // set my services
									currentToken.stations[MYADDRESS-1] = gTokenInterface.station_list[MYADDRESS];
									break;
								
								case DATA_IND:
								{	
								
									
									msgToSend = osMemoryPoolAlloc(memPool, 0);
									
									// control
									uint8_t srcSapi = bufferedMessage.sapi;
									uint8_t src = MYADDRESS;
									uint8_t destSapi = bufferedMessage.sapi;
									uint8_t dest = bufferedMessage.addr;
									
									
									uint8_t srcByte = 0x00;
									srcByte = srcSapi | (src << 3);
									
									uint8_t destByte = 0x00;
									destByte = destSapi | (dest << 3);
									
									msgToSend->controlInt = (((uint16_t)destByte << 8) | (srcByte));
									
									//lenght - data
									uint8_t length = 0x00;
									
									uint8_t i = 0;
									while(((uint8_t*)(bufferedMessage.anyPtr))[i] != 0x00){
										msgToSend->data[length++] = ((uint8_t*)(bufferedMessage.anyPtr))[i++];
									}
									
									msgToSend->length = length;
									
									// satuts
									uint8_t sum = 0;
									
									for(uint8_t j = 0; j < (length + 2 + 1); j++){
										sum += ((uint8_t*)&msgToSend)[j]; 
									}
									
									uint8_t status = 0x00;
									
									status = sum << 2;
									
									msgToSend->data[length] = status;
									
									
									
									queueMsgPhyS.type = TO_PHY;			// message type
									queueMsgPhyS.anyPtr = msgToSend;					// pointer to token
									retCode = osMessageQueuePut(
										queue_phyS_id,
										&queueMsgPhyS,
										osPriorityNormal,
										osWaitForever);
									CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
								
								
								
								
								
									//TODO
									messageRead = true;
								}
									break;
								
								
								default:
									break;
							}
							
							
							
					}
					else{
						queueMsgPhyS.type = TO_PHY;			// message type
						queueMsgPhyS.anyPtr = &(currentToken.token_data);					// pointer to token
						retCode = osMessageQueuePut(
							queue_phyS_id,
							&queueMsgPhyS,
							osPriorityNormal,
							osWaitForever);
						CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
						
						messageRead = true;
					}
				}
				
				//send token list
				queueMsgPhyS.type = TOKEN_LIST;			// message type
				retCode = osMessageQueuePut(
					queue_lcd_id,
					&queueMsgPhyS,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
				
	
			}
				break;
			
			case DATABACK: // FAUX!
				queueMsgPhyS.type = TO_PHY;			// message type
				queueMsgPhyS.anyPtr = &(currentToken.token_data);					// pointer to token
				retCode = osMessageQueuePut(
					queue_phyS_id,
					&queueMsgPhyS,
					osPriorityNormal,
					osWaitForever);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
				break;
			
			default:
				break;
		}
	}
	
}
