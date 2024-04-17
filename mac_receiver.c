#include "stm32f7xx_hal.h"

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os2.h" 

bool isToken(uint8_t* msg){
	return (msg[0] == TOKEN_TAG);
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
				
			}
			
		}
		
	}
	
}
