#include <lcom/lcf.h>

#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "i8254.h"
#include "keyboard.h"
#include "timer.h"
#include "utils.h"

extern uint8_t key_pressed;
extern int kbd_counter;
extern int counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  message msg;
  int ipc_status;
  uint16_t r;
  uint8_t irq_set = 0;

  uint8_t separate_bytes[2];
  uint8_t size;
  bool is_makecode = true;
  bool is_one_byte = true;

  if (keyboard_subscribe(&irq_set) != 0) {
    return 1;
  }

  while (key_pressed != ESC_KEY) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
            if (key_pressed == TWO_BYTES) {
              is_one_byte = false;
            }
            else {
              if (is_one_byte == false) {
                verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

                if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
                  printf("Error printing scancode\n");
                  return 1;
                }
                is_one_byte = true;
              }
              else {

                verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

                if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
                  printf("Error printing scancode\n");
                  return 1;
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (keyboard_unsubscribe() != 0)
    return 1;

  kbd_print_no_sysinb(kbd_counter);

  return 0;
}

int(kbd_test_poll)() {

  uint8_t size;
  uint8_t separate_bytes[2];

  bool is_makecode = true;
  bool is_one_byte = true;

  while (separate_bytes[0] != ESC_KEY) {

    kbc_poll_ih();

    if (key_pressed == TWO_BYTES) {
      is_one_byte = false;
    }
    else {
      if (is_one_byte == false) {
        verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

        if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
          printf("Error printing scancode\n");
          return 1;
        }
        is_one_byte = true;
      }
      else {

        verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

        if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
          printf("Error printing scancode\n");
          return 1;
        }
      }
    }
  }

  if (interrupt_enabler() != 0) {
    printf("Interruption couldn't be enabled\n");
    return 1;
  }

  if (kbd_print_no_sysinb(kbd_counter) != 0) {
    return 1;
  }

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {	
  message msg;
  int ipc_status;
  uint16_t r;
  uint8_t irq_timer = 0;
  uint8_t irq_kbd = 1;
  int elapsed_time = 0;


  uint8_t separate_bytes[2];
  uint8_t size;
  bool is_makecode = true;
  bool is_one_byte = true;

  if (keyboard_subscribe(&irq_kbd) != 0) {
    return 1;
  }

  if (timer_subscribe_int(&irq_timer) != 0) {
      return 1;
  }

  while (key_pressed != ESC_KEY && elapsed_time < n) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }

    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
			if (msg.m_notify.interrupts & irq_timer) {
				timer_int_handler();
				if ((counter % sys_hz()) == 0) {
					elapsed_time++;
					timer_print_elapsed_time();
				}
			}
          if (msg.m_notify.interrupts & irq_kbd) {
            kbc_ih();
			elapsed_time = 0;
			counter = 0;

            if (key_pressed == TWO_BYTES) {
              is_one_byte = false;
            }
            else {
              if (is_one_byte == false) {
                verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

                if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
                  printf("Error printing scancode\n");
                  return 1;
                }
                is_one_byte = true;
              }
              else {

                verify_byte(separate_bytes, &size, &is_makecode, &is_one_byte); // Verifies and handles both cases

                if (kbd_print_scancode(is_makecode, size, separate_bytes) != 0) {
                  printf("Error printing scancode\n");
                  return 1;
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }
  }

  if (keyboard_unsubscribe() != 0)
    return 1;

  if (timer_unsubscribe_int() != 0) {
      return 1;
    }

  kbd_print_no_sysinb(kbd_counter);

  return 0;
}
