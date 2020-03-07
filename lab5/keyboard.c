#include "keyboard.h"

int kbd_hook_id = 1;
uint8_t key_pressed; //scancode
int kbd_counter = 0;

int sys_inb_increment_counter(int port, uint8_t *value) {
  kbd_counter++;
  return util_sys_inb(port, value);
}

int(keyboard_subscribe)(uint8_t *bit_no) {
  *bit_no = BIT(kbd_hook_id);

  if (!(0 <= kbd_hook_id && kbd_hook_id <= 31)) { // Checks if hook_id has an adequate range
    printf("Hook_id not in range\n");
    return 1;
  }

  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != 0) { // Subscription
    return 1;
  }

  return 0;
}

int keyboard_unsubscribe() {

  if (sys_irqrmpolicy(&kbd_hook_id) != 0) // Unsubscription
    return 1;

  return 0;
}

int verify_byte(uint8_t separate_bytes[], uint8_t *size, bool *is_make_code, bool *is_one_byte) { // Verifies if scancode is one byte and prepares kbd_print_scancode

  if (*is_one_byte == true) {

    *size = 1;

    separate_bytes[0] = key_pressed;

    if (key_pressed & MAKE_CODE_VERIFICATION)
      *is_make_code = false;
    else
      *is_make_code = true;
  }

  else {

    *size = 2;

    separate_bytes[0] = TWO_BYTES; // Has to start with 0xE0, not only the code

    separate_bytes[1] = key_pressed;

    if (key_pressed & MAKE_CODE_VERIFICATION)
      *is_make_code = false;

    else
      *is_make_code = true;
  }

  return 0;
}

int interrupt_enabler() {
  uint8_t command_byte;

  if (sys_outb(STATUS_REGISTER, 0x20) != 0) // 0x20 is the "not named register"
    return 1;

  if (sys_inb_increment_counter(KEYBOARD_BUF, &command_byte) != 0) // Gets scancode, need to use 0x60
    return 1;

  command_byte = command_byte | BIT(0); // Last bit has to be 1 for keyboard interruptions, see slides

  if (sys_outb(STATUS_REGISTER, KEYBOARD_BUF) != 0)
    return 1;

  if (sys_outb(KEYBOARD_BUF, command_byte) != 0)
    return 1;

  return 0;
}

void(kbc_ih)() {

  uint8_t status;

  if (sys_inb_increment_counter(STATUS_REGISTER, &status) != 0) { //Get status from status register
    return;
  }
  while (1) {
    if ((status & OUTPUT_BUF_FULL)) { // If OUTPUT_BUF_FULL is 1, ready to read
      if (sys_inb_increment_counter(KEYBOARD_BUF, &key_pressed) != 0) { // Reads key
        printf("Couldn't get data from Output buffer\n");
        return;
      }

      if ((status & (PARITY_ERROR | TIMEOUT_ERROR)) == 0) {
        return;
      }
      else
        printf("Errors on parity or timeout");
    }
  }
    tickdelay(micros_to_ticks(DELAY_US));
}

void(kbc_poll_ih)() {

  uint8_t status;

  while (1) {
    if (sys_inb_increment_counter(STATUS_REGISTER, &status) != 0) {
      return;
    }

    if ((status & OUTPUT_BUF_FULL)) {
      if (sys_inb_increment_counter(KEYBOARD_BUF, &key_pressed) != 0) { // Reads key
        printf("Couldn get data from Output buffer");
        return;
      }

      if ((status & (PARITY_ERROR | TIMEOUT_ERROR | AUX)) == 0) { // Also need to verify if AUX is cleared
        return;
      }
	  else
		  printf("Errors on parity or timeout\n");
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}
