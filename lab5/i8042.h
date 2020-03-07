#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define BIT(n)    (1 << (n))

#define DELAY_US    20000

#define MOUSE_IRQ   12

/////* Buffers - input and output */////

#define KEYBOARD_BUF     0x60   //works as input and output buffer of kbc
#define MOUSE_BUF        0x60   //works as input and output buffer of kbc


/////* ScanCodes */////

#define ESC_KEY    0x81
#define MAKE_CODE_VERIFICATION BIT(7)
#define TWO_BYTES   0xE0


/////* Status Register State */////

#define PARITY_ERROR     BIT(7)
#define TIMEOUT_ERROR    BIT(6)
#define	AUX              BIT(5)
#define KEYBOARD_INHIBIT BIT(4)
#define A2	             BIT(3) // Used to ensure synced packets
#define SYSTEM_FLAG      BIT(2)
#define INPUT_BUF_FULL	 BIT(1)
#define OUTPUT_BUF_FULL  BIT(0)


#define STATUS_REGISTER  0x64 
#define CONTROL_REGISTER 0x64


/////* Mouse Acknowledgments */////

#define ACK              0xFA //if everything ok
#define NACK             0xFE //if byte is invalid
#define ERROR            0xFC //if 2 consecutive bytes are invalid


#define WRITE_MOUSE_BYTE 0xD4 	
#define WRITE_CMD_BYTE   0x60


/////* Mouse Commands */////

#define MOUSE_ENABLE     0xF4 	
#define MOUSE_DISABLE	   0xF5 
#define MOUSE_READ_RQST  0xEB  //Request mouse data packet
#define MOUSE_STREAM     0xEA  //To enable stream mode

/////* Mouse buttons and things */////

#define Y_OVFL              BIT(7)    
#define X_OVFL              BIT(6)    
#define Y_SIGN              BIT(5)    
#define X_SIGN              BIT(4)    
#define MIDDLE_BUTTON       BIT(2)    
#define RIGHT_BUTTON        BIT(1)    
#define LEFT_BUTTON         BIT(0)    

#endif /* _LCOM_I8254_H */
