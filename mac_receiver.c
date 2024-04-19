#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os2.h" 


typedef union msgType_{
	struct __attribute__((__packed__)){
		struct control_{
			unsigned blank1 : 1;
			unsigned src : 4;
			unsigned srcSapi : 3;
			unsigned blank2 : 1;
			unsigned dest : 4;
			unsigned destSapi : 3;
		}control;
		uint8_t length;
		uint8_t data[];
	};
	uint8_t *msg_frame;//not sure about it
}msgType;

typedef union msgStatus_{
	struct __attribute__((__packed__)){
		unsigned checksum : 6;
		unsigned read : 1;
		unsigned ack : 1;
	};
	uint8_t status;
}msgStatus;




bool isToken(uint8_t* msg){
	return (msg[0] == TOKEN_TAG);
}

bool processChecksum(msgType* msg){
	uint8_t sum;
	
	for(uint8_t i = 0; i < (msg->length + 2 + 1); i++){
		sum += msg->msg_frame[i]; // we were here, finish this func.
	}
	
	return (sum == ((msgStatus)(msg->data[msg->length])).checksum);
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
				msgType msgRecieved;
				msgRecieved.msg_frame = *(uint8_t**)(queueMsgMacR.anyPtr);
				
				if(msgRecieved.control.dest == BROADCAST_ADDRESS){
					if(processChecksum(&msgRecieved)){
						// broadcast
						if(msgRecieved.control.destSapi == TIME_SAPI){
							
							uint8_t data[msgRecieved.length+1]; // create msg to send
							for(uint8_t i = 0; i < msgRecieved.length; i++){
								data[i] = msgRecieved.data[i];
							}
							data[msgRecieved.length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = data;	
							queueMsgToSend.addr = msgRecieved.control.src;
							queueMsgToSend.sapi = msgRecieved.control.srcSapi;
							
							retCode = osMessageQueuePut( // send msg
								queue_timeR_id,
								&queueMsgToSend,
								osPriorityNormal,
								osWaitForever);
							
							CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
						}
						else if(msgRecieved.control.destSapi == CHAT_SAPI){
							if(gTokenInterface.connected){
								
								uint8_t data[msgRecieved.length+1]; // create msg to send
							for(uint8_t i = 0; i < msgRecieved.length; i++){
								data[i] = msgRecieved.data[i];
							}
							data[msgRecieved.length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = data;	
							queueMsgToSend.addr = msgRecieved.control.src;
							queueMsgToSend.sapi = msgRecieved.control.srcSapi;
							
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
				else if(msgRecieved.control.dest == MYADDRESS){
					if(processChecksum(&msgRecieved)){
						msgRecieved.data[msgRecieved.length] |= 0b1; // ACK <= 1
						// message to us
						if(msgRecieved.control.destSapi == TIME_SAPI){
							msgRecieved.data[msgRecieved.length] |= 0b10; // READ <= 1
							
							uint8_t data[msgRecieved.length+1]; // create msg to send
							for(uint8_t i = 0; i < msgRecieved.length; i++){
								data[i] = msgRecieved.data[i];
							}
							data[msgRecieved.length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = data;	
							queueMsgToSend.addr = msgRecieved.control.src;
							queueMsgToSend.sapi = msgRecieved.control.srcSapi;
							
							retCode = osMessageQueuePut( // send msg
								queue_timeR_id,
								&queueMsgToSend,
								osPriorityNormal,
								osWaitForever);
							
							CheckRetCode(retCode,__LINE__,__FILE__,CONTINUE); // check if no problem
						}
						else if(msgRecieved.control.destSapi == CHAT_SAPI){
							if(gTokenInterface.connected){
								msgRecieved.data[msgRecieved.length] |= 0b10; // READ <= 1
								
								uint8_t data[msgRecieved.length+1]; // create msg to send
							for(uint8_t i = 0; i < msgRecieved.length; i++){
								data[i] = msgRecieved.data[i];
							}
							data[msgRecieved.length] = '\0';
							
							queueMsgToSend.type = DATA_IND; // create queue msg
							queueMsgToSend.anyPtr = data;	
							queueMsgToSend.addr = msgRecieved.control.src;
							queueMsgToSend.sapi = msgRecieved.control.srcSapi;
							
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
				
				
				if(msgRecieved.control.src == MYADDRESS){
					// send databack
								
					queueMsgToSend.type = DATABACK; // create queue msg
					queueMsgToSend.anyPtr = msgRecieved.msg_frame;	
					queueMsgToSend.addr = msgRecieved.control.src;
					queueMsgToSend.sapi = msgRecieved.control.srcSapi;
					
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
					queueMsgToSend.anyPtr = msgRecieved.msg_frame;	
					
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
