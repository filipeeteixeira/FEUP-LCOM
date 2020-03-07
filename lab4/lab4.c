// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "utils.h"
#include "mouse.h"
#include "timer.h"

extern uint8_t received_byte;
extern int counter; // counter from timer.c


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

// This function must configure the mouse properly,
// receive the number of mouse packets specified in its argument via interrupts
// parse them and print them
// Before exiting, it should reset the mouse to Minix's default configuration.
// Data reporting must be enabled - use int mouse_enable_data_reporting() while we don't do our own

// Use IH: void mouse_ih()
// To print: void mouse_print_packet(struct packet *pp);
// struct packet contains not only the bytes of a mouse packet but all other members aswell

// should terminate after printing the number of packets specified in its argument cnt. 
// However, before returning, it must disable data reporting and unsubscribe mouse interrupts
int (mouse_test_packet)(uint32_t cnt) {
	message msg;
	int ipc_status;
	uint16_t r;
	uint32_t irq_set;
	uint8_t bit_no;
	struct packet pkt;

	uint32_t internal_counter = 0; // To count number of times print function occurs
	int packet_counter = 0;  // To count the corresponding packet number 

	write_command(MOUSE_ENABLE);

	if (mouse_subscribe(&bit_no) != 0) {
		printf("Error while subscribing\n");
		return 1;
	}

	irq_set = BIT(bit_no);
	
	while (internal_counter < cnt) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					mouse_ih();
					if (packet_counter == 0) {
						if ((received_byte & A2) != 0) { // To ensure synced packets
							pkt.bytes[0] = received_byte;
							packet_counter++;
						}
						else
							continue;
					}
					else if (packet_counter == 1)
					{
						pkt.bytes[1] = received_byte;
						packet_counter++;
					}

					else
					{
						pkt.bytes[2] = received_byte;
						parse_packet(&pkt);
						packet_counter = 0;
						mouse_print_packet(&pkt);
						internal_counter++;
					}

				}
				break;
			default:
				break;
			}
		}
	}

	if (mouse_unsubscribe() != 0) {
		printf("Error while unsubscribing\n");
		return 1;
	}

	write_command(MOUSE_DISABLE);

	return 0;
}


// using remote mode and by polling the KBC's status register
// should configure the mouse to operate in remote mode and periodically request a packet from the mouse (period)
// should assemble, parse and print by calling mouse_print_packet()
// it must reset the mouse to stream mode with data reporting disabled before returning
// also: call uint8_t minix_get_dflt_kbc_cmd_byte();

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
		uint32_t internal_counter = 0;

		/* write_command(MOUSE_ENABLE); 
		lcf_start already does that, disables mouse interrupts and sets remote mode
		*/

		while (internal_counter < cnt){
			write_command(MOUSE_READ_RQST); //requesting data packet
			mouse_polling();
			internal_counter++;
		
			tickdelay(micros_to_ticks(period * 1000));
		}

	write_command(MOUSE_STREAM); //enables stream mode
	write_command(MOUSE_DISABLE);
	
	sys_outb(CONTROL_REGISTER, WRITE_CMD_BYTE); //Put KBC command byte to default value

	sys_outb(MOUSE_BUF, minix_get_dflt_kbc_cmd_byte()); //writes command byte
	
	return 0;
}

// Should do essentially the same as mouse_test_packet()
// Should terminate if it receives no packets in idle_time
// Measure time using timer 0 (you must not change the configuration) - can use sys_hz
// Must subscribe mouse interrupts and enable data reporting
// Disable data reporting and unsubscribe mouse interrupts before returning

int (mouse_test_async)(uint8_t idle_time) {

	message msg;
	int ipc_status, r;
	uint32_t irq_set_mouse;
	uint8_t irq_set_timer;
	uint8_t bit_no_mouse;
	struct packet pkt;

	uint32_t internal_counter = 0; // To count number of times print function occurs
	int packet_counter = 0;  // To count the corresponding packet number 
	uint8_t my_time = 0; // Counts time in seconds

	write_command(MOUSE_ENABLE);

	if (timer_subscribe_int(&irq_set_timer) != 0) {
		printf("Error while subscribing timer\n");
		return 1;
	}

	if (mouse_subscribe(&bit_no_mouse) != 0) {
		printf("Error while subscribing mouse\n");
		return 1;
	}

	irq_set_mouse = BIT(bit_no_mouse);

	while (my_time < idle_time) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}

		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set_timer) {
					timer_int_handler();
					if ((counter % sys_hz()) == 0) { //counter from timer.c
						my_time++;
						
					}
				}
				if (msg.m_notify.interrupts & irq_set_mouse) {
					mouse_ih();
					counter = 0;
					my_time = 0;

					if (packet_counter == 0)
						if ((received_byte & A2) != 0) { // To ensure synced packets
							pkt.bytes[0] = received_byte;
							packet_counter++;
						}
						else
							continue;
					else if (packet_counter == 1)
					{
						pkt.bytes[1] = received_byte;
						packet_counter++;
					}

					else
					{
						pkt.bytes[2] = received_byte;
						parse_packet(&pkt);
						packet_counter = 0;
						mouse_print_packet(&pkt);
						internal_counter++;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (mouse_unsubscribe() != 0) {
		printf("Error while unsubscribing\n");
		return 1;
	}

	if (timer_unsubscribe_int() != 0) {
		printf("Error while unsubscribing\n");
		return 1;
	}

	write_command(MOUSE_DISABLE);

	return 0;
}

// the function should terminate if the user draws an inverted V, from left to right
// The first line must be drawn while the left button, and no other, is pressed down, 
// whereas the second line must be drawn while the right button, and no other, is pressed down
// value of the slope of each line must be larger than 1
// value of the displacement of each line along the x-direction must have minimum value specified in the argument

// SEE ALGORITHM REQUIREMENTS
// must start matching the gesture from beginning


int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
	message msg;
	int ipc_status;
	uint16_t r;
	uint32_t irq_set;
	uint8_t bit_no;
	struct packet pkt;

	uint32_t internal_counter = 0; // To count number of times print function occurs
	int packet_counter = 0;  // To count the corresponding packet number 

	struct mouse_ev* m_ev;
	bool concluded = false;

	write_command(MOUSE_ENABLE);

	if (mouse_subscribe(&bit_no) != 0) {
		printf("Error while subscribing\n");
		return 1;
	}

	irq_set = BIT(bit_no);
	
	while (!concluded) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					mouse_ih();
					if (packet_counter == 0) {
						if ((received_byte & A2) != 0) { // To ensure synced packets
							pkt.bytes[0] = received_byte;
							packet_counter++;
						}
						else
							continue;
					}
					else if (packet_counter == 1)
					{
						pkt.bytes[1] = received_byte;
						packet_counter++;
					}

					else
					{
						pkt.bytes[2] = received_byte;
						parse_packet(&pkt);
						packet_counter = 0;
						mouse_print_packet(&pkt);
						internal_counter++;
						
						m_ev = mouse_detect_event(&pkt);
						gesture_handler(&pkt, &concluded, x_len, tolerance, m_ev);
					}

				}
				break;
			default:
				break;
			}
		}
	}

	if (mouse_unsubscribe() != 0) {
		printf("Error while unsubscribing\n");
		return 1;
	}

	write_command(MOUSE_DISABLE);
	
	return 0;
}

