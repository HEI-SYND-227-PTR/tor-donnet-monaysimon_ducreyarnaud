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
	uint8_t *msg_frame;
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

bool processStatus(msgType* msg, bool setRead){
	uint8_t sum;
	
	for(uint8_t i = 0; i < (msg->length + 2 + 1); i++){
		sum += msg->msg_frame[i]; // we were here, finish this func.
	}
}


void MacReceiver(void *argument)
{
	// Tout doux
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
					// broadcast
				}
				else if(msgRecieved.control.dest == MYADDRESS){
					// message to us
				}
				else{
					// send message directly to phy
				}
				
			}
			
		}
		
	}
	
}
