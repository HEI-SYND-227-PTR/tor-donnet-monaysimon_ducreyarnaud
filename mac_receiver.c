#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os2.h" 







bool isToken(uint8_t* msg){
	return (msg[0] == TOKEN_TAG);
}

bool processChecksum(msgType* msg){
	uint8_t sum = 0;
	
	for(uint8_t i = 0; i < (msg->length + 2 + 1); i++){
		sum += ((uint8_t*)msg)[i]; 
	}
	
	msgStatus temp;
	temp.status = msg->data[msg->length];
	
	return ((sum & 0x3F) == temp.checksum);
}


void MacReceiver(void *argument)
{
	while(1){
		osStatus_t retCode;
		
		struct queueMsg_t queueMsgMacR;					// queue message from any
		struct queueMsg_t queueMsgToSend;					// queue message to phy
		
		
		retCode = osMessageQueueGet(queue_macR_id, &queueMsgMacR, NULL, osWaitForever);
		CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
		
		if(queueMsgMacR.type == FROM_PHY){
			if(isToken((uint8_t*)(queueMsgMacR.anyPtr))){ // check if token
				
				queueMsgToSend.type = TOKEN;
				queueMsgToSend.anyPtr = queueMsgMacR.anyPtr; // prepare msg
				
				
				retCode = osMessageQueuePut( // send to mac sender
					queue_macS_id,
					&queueMsgToSend,
					osPriorityNormal,
					0);
				CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE);
			}
			else{
								
				// is message
				msgType* msgRecieved;
				msgRecieved = queueMsgMacR.anyPtr;
				
				if(msgRecieved->control.dest == BROADCAST_ADDRESS){
					if(processChecksum(msgRecieved)){
						// broadcast
						if(msgRecieved->control.destSapi == TIME_SAPI){
							
							uint8_t * tempData = osMemoryPoolAlloc(memPool, 0);
							for(uint8_t i = 0; i < msgRecieved->length; i++){
								tempData[i] = msgRecieved->data[i];
							}
							tempData[msgRecieved->length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = tempData;	
							queueMsgToSend.addr = msgRecieved->control.src;
							queueMsgToSend.sapi = msgRecieved->control.srcSapi;
							
							retCode = osMessageQueuePut( // send msg
								queue_timeR_id,
								&queueMsgToSend,
								osPriorityNormal,
								osWaitForever);
							
							CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
						}
						else if(msgRecieved->control.destSapi == CHAT_SAPI){
							if(gTokenInterface.connected){
								
								uint8_t * tempData = osMemoryPoolAlloc(memPool, 0);
							for(uint8_t i = 0; i < msgRecieved->length; i++){
								tempData[i] = msgRecieved->data[i];
							}
							tempData[msgRecieved->length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = tempData;	
							queueMsgToSend.addr = msgRecieved->control.src;
							queueMsgToSend.sapi = msgRecieved->control.srcSapi;
							
							retCode = osMessageQueuePut( // send msg
								queue_chatR_id,
								&queueMsgToSend,
								osPriorityNormal,
								osWaitForever);
							
							CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
							}
						}
						else{
						}
					}
				}
				else if(msgRecieved->control.dest == MYADDRESS){
					
					if(((1 << msgRecieved->control.destSapi) & gTokenInterface.station_list[MYADDRESS])>0){
						msgRecieved->data[msgRecieved->length] |= gTokenInterface.connected << 1; // READ <= connected
					}
					
					
					
					if(processChecksum(msgRecieved)){
						msgRecieved->data[msgRecieved->length] |= 1; // ACK <= 1
						// message to us
						if(msgRecieved->control.destSapi == TIME_SAPI){							
							uint8_t * tempData = osMemoryPoolAlloc(memPool, 0);
							for(uint8_t i = 0; i < msgRecieved->length; i++){
								tempData[i] = msgRecieved->data[i];
							}
							tempData[msgRecieved->length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = tempData;	
							queueMsgToSend.addr = msgRecieved->control.src;
							queueMsgToSend.sapi = msgRecieved->control.srcSapi;
							
							retCode = osMessageQueuePut( // send msg
								queue_timeR_id,
								&queueMsgToSend,
								osPriorityNormal,
								osWaitForever);
							
							CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
						}
						else if(msgRecieved->control.destSapi == CHAT_SAPI){
							if(gTokenInterface.connected){								
								
								uint8_t * tempData = osMemoryPoolAlloc(memPool, 0);
								
								for(uint8_t i = 0; i < msgRecieved->length; i++){
									tempData[i] = msgRecieved->data[i];
								}
								tempData[msgRecieved->length] = '\0';
								
								queueMsgToSend.type = DATA_IND; // create queue msg
								queueMsgToSend.anyPtr = tempData;	
								queueMsgToSend.addr = msgRecieved->control.src;
								queueMsgToSend.sapi = msgRecieved->control.srcSapi;
								
								retCode = osMessageQueuePut( // send msg
									queue_chatR_id,
									&queueMsgToSend,
									osPriorityNormal,
									osWaitForever);
								
								CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
							}
						}
					}
				}
				
				
				if(msgRecieved->control.src == MYADDRESS){
					// send databack
								
					queueMsgToSend.type = DATABACK; // create queue msg
					queueMsgToSend.anyPtr = msgRecieved;	
					queueMsgToSend.addr = msgRecieved->control.src;
					queueMsgToSend.sapi = msgRecieved->control.srcSapi;
					
					retCode = osMessageQueuePut( // send msg
						queue_timeR_id,
						&queueMsgToSend,
						osPriorityNormal,
						osWaitForever);
					
					CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
				}
				else{
					// send to phy
										
					queueMsgToSend.type = TO_PHY; // create queue msg
					queueMsgToSend.anyPtr = msgRecieved;	
					
					retCode = osMessageQueuePut( // send msg
						queue_phyS_id,
						&queueMsgToSend,
						osPriorityNormal,
						osWaitForever);
					
					CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
				}
								
			}
			
		}
		
	}
	
}
