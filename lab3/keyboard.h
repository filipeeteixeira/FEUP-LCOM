#pragma once

#include <lcom/lcf.h>
#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8254.h"
#include "i8042.h"
#include "utils.h"
#include "timer.h"


// Calls util_sys_inb() and increments a keyboard counter for testing purposes
int sys_inb_increment_counter(int port,uint8_t *value);

// Subscribes keyboard
int(keyboard_subscribe)(uint8_t* bit_no);

// Unsubscribes keyboard
int keyboard_unsubscribe();

// Breakes connection between minix and our IH
int interrupt_enabler();


// Prepares kbd_print_scancode
int verify_byte(uint8_t separate_bytes[], uint8_t *size, bool *is_makecode, bool *is_one_byte);

void (kbc_ih)();

void (kbc_poll_ih)();
