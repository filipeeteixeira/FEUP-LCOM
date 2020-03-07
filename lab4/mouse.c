#include <lcom/lcf.h>
#include "mouse.h"
#include "utils.h"
#include "timer.h"
#include "math.h"

// Summary of Mouse Functionality

// "KBC-commands" are written to port 0x64
// To issue a command to the mouse, the driver must first write command 0xD4 to port 0x64 and then mouse command to port 0x60
// Mouse will send an acknowledgment (read from port 0x60)
// To ensure sync: bit 3 of byte 1, must be 1

// If the device is in Stream mode and has been enabled with 0xF4 command, then disable the device with 0xF5 command
// If ack returns 0xFE, it should retry the offending command - should retransmit the entire command

// Use not only the IRQ_REENABLE policy but also the IRQ_EXCLUSIVE policy

// Use tickdelay() to wait for ack


uint8_t received_byte;
int mouse_hook_id = 2;

void(mouse_ih)() {
	uint8_t status;

	if (util_sys_inb(STATUS_REGISTER, &status) != 0) { // Get status from status register
		printf("Sys_inb failed\n");
		return;
	}

	
	if ((status & OUTPUT_BUF_FULL) && ((status & (PARITY_ERROR | TIMEOUT_ERROR)) == 0)) { // If OUTPUT_BUF_FULL is 1 and no errors, ready to read
			if (util_sys_inb(MOUSE_BUF, &received_byte) != 0) { // Reads
				printf("Couldn't get data from Output buffer\n");
				return;
			}
			return;
	}
	
}

int mouse_subscribe (uint8_t* bit_no) {
	*bit_no = mouse_hook_id;

	if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != 0) { // Subscription
		printf("Could not subscribe the interrupts\n");
		return 1;
	}
	return 0;
}

int mouse_unsubscribe() {

	if (sys_irqrmpolicy(&mouse_hook_id)) { // Un-subscription
		printf("Could not unsubscribe the interrupts\n");
		return 1;
	}

	return 0;
}

int parse_packet(struct packet *pkt) {

	int y_msb, x_msb;

	if ((*pkt).bytes[0] & LEFT_BUTTON)
		(*pkt).lb = true;
	else
		(*pkt).lb = false;

	if ((*pkt).bytes[0] & RIGHT_BUTTON)
		(*pkt).rb = true;
	else
		(*pkt).rb = false;

	if ((*pkt).bytes[0] & MIDDLE_BUTTON)
		(*pkt).mb = true;
	else
		(*pkt).mb = false;

	if ((*pkt).bytes[0] & X_SIGN)
		x_msb = 1;
	else
		x_msb = 0;
	
	if ((*pkt).bytes[0] & Y_SIGN)
		y_msb = 1;
	else
		y_msb = 0;

	if ((*pkt).bytes[0] & X_OVFL)
		(*pkt).x_ov = true;
	else
		(*pkt).x_ov = false;

	if ((*pkt).bytes[0] & Y_OVFL)
		(*pkt).y_ov = true;
	else
		(*pkt).y_ov = false;

	int16_t delta_x = ValueTo2s((*pkt).bytes[1], x_msb);
	int16_t delta_y = ValueTo2s((*pkt).bytes[2], y_msb);

	(*pkt).delta_x = delta_x;
	(*pkt).delta_y = delta_y;


	return 0;
}

int16_t ValueTo2s(uint8_t value, int msb) {
	if (msb == 0)
		return value;
	
	else
		return value - 256;
}

void mouse_polling(){
	uint8_t status;

	struct packet pkt;

	if (util_sys_inb(STATUS_REGISTER, &status) != 0) { // Get status from status register
		printf("Sys_inb failed\n");
		return;
	}
 	if ((status & OUTPUT_BUF_FULL) != 1) //output buffer must be full to read mouse bytes
  {
		printf("Output buffer must be full\n");
		return;
	}
	else if ((status & (PARITY_ERROR | TIMEOUT_ERROR)) != 0) 
	{
		printf("Error on timeout or parity\n");
		return;
	}
	else{
		for (int i = 0; i < 3; i++) //3 bytes on mouse packet
			{
				if (util_sys_inb(MOUSE_BUF, &received_byte) != OK) //Reads the output buffer
				{
					printf("Error reading output buffer");
					return;
				}

				if (i == 0)
					if (received_byte & A2) //check sync on first byte
						pkt.bytes[0] = received_byte;

				if (i == 1)
					pkt.bytes[1] = received_byte;

				if (i == 2)
					pkt.bytes[2] = received_byte;
			}

			parse_packet(&pkt); 
			mouse_print_packet(&pkt); //write the data packet
	}
	
}

int(write_command)(uint32_t cmd){ //to issue a command to the mouse, the driver must first write command 0xD4 to the KBC
	uint8_t status=0;
	uint8_t ackn;

	while(1){
			if (util_sys_inb(STATUS_REGISTER, &status) != 0) { // Get status from status register
				printf("Sys_inb failed\n");
				return 1;
			}

			if ((status & (AUX | INPUT_BUF_FULL)) == 0){ // check the IBF bit in the STATUS_REG, before writing to port
				if (sys_outb(CONTROL_REGISTER, WRITE_MOUSE_BYTE) !=0){ //allows the device driver to issue commands directly to the mouse
					printf("Sys_outb failed\n");
					return 1;
				}
			}

			if (util_sys_inb(STATUS_REGISTER, &status) != 0) { // verify the status of the buffer
				printf("Sys_inb failed\n");
				return 1;
			}

			if ((status & (AUX | INPUT_BUF_FULL)) == 0){ //aux grants if the data is coming from the mouse
				if (sys_outb(MOUSE_BUF,cmd) !=0){ //writes command byte
					printf("Sys_outb failed\n");
					return 1;
				}
			}

			if (util_sys_inb(MOUSE_BUF, &ackn) != 0) { // get acknoledgmnet to verify the status of the buffer
				printf("Sys_inb failed\n");
				return 1;
			}
			if(ackn == ACK){
				return 0;
			}
			else if(ackn == ERROR){
				return 1;
			}
		}
		return 0;
}


void gesture_handler(struct packet* pkt, bool *concluded, uint8_t x_len, uint8_t tolerance, struct mouse_ev* m_ev) {
	static enum state_enum state = INITIAL;
	static int16_t tempX, tempY; // Mouse movement in the cartesian plane
	
	switch (state)
	{
		case INITIAL:
			tempX = 0, tempY = 0;
			if (m_ev->type == LB_PRESSED)
				state = LEFT_SIDE;
			break;

		case LEFT_SIDE:
			if (m_ev->type == LB_RELEASED) {
				if ((tempX >= x_len) && fabs(tempY / (float)tempX) > 1) {
					state = SWITCH_SIDES;
					break;
				}
				else
					state = INITIAL;
			}

			else if (m_ev->type == MOUSE_MOV) {
				if (((m_ev->delta_x > 0) && (m_ev->delta_y > 0)) || (abs(m_ev->delta_x) <= tolerance && abs(m_ev->delta_y) <= tolerance))
				{
					tempX += m_ev->delta_x;
					tempY += m_ev->delta_y;

				}
				else
					state = INITIAL;
				break;
			}
		case SWITCH_SIDES:
			tempX = 0;
			tempY = 0;

			if (m_ev->type == BUTTON_EV || m_ev->type == LB_PRESSED)
				state = INITIAL;
			else if (!(((m_ev->delta_x > 0) && (m_ev->delta_y > 0)) || (abs(m_ev->delta_x) <= tolerance && abs(m_ev->delta_y) <= tolerance))) {
				state = INITIAL;
			}
			else
				state = RIGHT_SIDE;
			break;

		case RIGHT_SIDE:
			
			if (m_ev->type == RB_RELEASED) {
				
				if ((tempX >= x_len) && fabs(tempY / (float)tempX) > 1) {
					
					state = INITIAL;
					*concluded = true;
				}
				else
					state = INITIAL;
			}

			else if (m_ev->type == MOUSE_MOV) {
				if (((m_ev->delta_x > 0) && (m_ev->delta_y > 0)) || (abs(m_ev->delta_x) <= tolerance && abs(m_ev->delta_y) <= tolerance))
				{
					tempX += m_ev->delta_x;
					tempY += m_ev->delta_y;
				}
				else
					state = INITIAL;
			}	
			break;
	}
	
}
