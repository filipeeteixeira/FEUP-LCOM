#include <lcom/lcf.h>

// Handles interrupts
void(mouse_ih)();

// Subscribes
int mouse_subscribe(uint8_t* bit_no);

// Unsubscribes
int mouse_unsubscribe();

// Completes the information about the current packet
int parse_packet(struct packet* pkt);

// Converts the binary number to two's complement
int16_t ValueTo2s(uint8_t value, int msb);

// 
void mouse_polling();

// Enables data reporting
int write_command();

// Handles the mouse movement at a given time and verifies if inverted V is drawn
void gesture_handler(struct packet* pkt, bool *concluded, uint8_t x_len, uint8_t tolerance, struct mouse_ev* m_ev);

// Possible states of state machine
enum state_enum { //Enumeration that represents all the states of the state machine
	INITIAL,         // Initial state
	LEFT_SIDE,       // Left side of symbol
	SWITCH_SIDES,     // Highest point of symbol
	RIGHT_SIDE,      // Right side of symbol, if it suceeds, then it should end the verification
};
