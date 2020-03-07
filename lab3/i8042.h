#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

#define BIT(n)    (1 << (n))

#define DELAY_US    20000

//#define KEYBOARD_IRQ 0x01   supostamente ja esta definida? dafuq

#define ESC_KEY    0x81

#define KEYBOARD_BUF     0x60

#define STATUS_REGISTER   0x64 

#define MAKE_CODE_VERIFICATION BIT(7)

// Status Register State
#define PARITY_ERROR   BIT(7)

#define TIMEOUT_ERROR  BIT(6)

#define	AUX  BIT(5)

#define KEYBOARD_INHIBIT   BIT(4)

#define A2_LINE	 BIT(3)

#define SYSTEM_FLAG BIT(2)

#define INPUT_BUF_FULL	BIT(1)

#define OUTPUT_BUF_FULL  BIT(0)


#define TWO_BYTES   0xE0

#endif /* _LCOM_I8254_H */
