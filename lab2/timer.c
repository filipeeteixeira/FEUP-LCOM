#include "i8254.h"
#include "utils.h"

#include <lcom/lcf.h>
#include <lcom/timer.h>
#include <stdint.h>

int hook_id = 0;
int counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
	uint8_t current_configuration;

	if(timer_get_conf(timer, &current_configuration) != 0)
		return 1;


	uint8_t new_configuration = (current_configuration & 0x0F) | TIMER_LSB_MSB; // 4 LSB are preserved, bits 4 and 5 are set to 1


	if (freq > TIMER_FREQ)
		return 1;

	uint16_t divisor = TIMER_FREQ / freq; // We achieve freq by then calculating freq = TIMER_FREQ / divisor

	uint8_t lsb, msb;
	
	util_get_LSB(divisor, &lsb);
	util_get_MSB(divisor, &msb);

	switch (timer)
	{
	case 0:
		new_configuration = new_configuration | TIMER_SEL0;
		if(sys_outb(TIMER_CTRL, new_configuration) != 0)
			return 1;
			
		if(sys_outb(TIMER_0, lsb) != 0)
			return 1;
			
		if(sys_outb(TIMER_0, msb) != 0)
			return 1;
			
		break;
	
	case 1:
		new_configuration = new_configuration | TIMER_SEL1;
		if (sys_outb(TIMER_CTRL, new_configuration) != 0)
			return 1;
			
		if(sys_outb(TIMER_1, lsb) != 0)
			return 1;
			
		if(sys_outb(TIMER_1, msb) != 0)
			return 1;
			
		break;
	
	case 2:
		new_configuration = new_configuration | TIMER_SEL2;	
		if(sys_outb(TIMER_CTRL, new_configuration) != 0)
			return 1;
			
		if(sys_outb(TIMER_2, lsb) != 0)
			return 1;
			
		if(sys_outb(TIMER_2, msb) != 0)
			return 1;
			
		break;
	
	default:
		return 1;
	}

	return 0;
}

int(timer_subscribe_int)(uint8_t *bit_no) {
	*bit_no = BIT(hook_id);

	sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id);

	if (!(0 <= hook_id && hook_id <= 31))  // Checks if hook_id has an adequate range
		return 1;
		
	return 0;
}

int(timer_unsubscribe_int)() {
	sys_irqrmpolicy(&hook_id);

  return 0;
}

void(timer_int_handler)() {
	 counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  
  if (timer < 0 || timer > 2){
    printf("Invalid Timer.");
    return 1;
  }

  uint8_t rbc = TIMER_RB_CMD | TIMER_RB_COUNT_ | 1 << (timer + 1); // Read-back command setup
  
  if(sys_outb(TIMER_CTRL, rbc) != 0)
	return 1;

  if(util_sys_inb(TIMER_0 + timer, st) != 0)
	  return 1;

  return 0;
}

int(timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  
  if (timer < 0 || timer > 2){
    printf("Invalid Timer.");
    return 1;
  }

  union timer_status_field_val status_value;

  if (field == tsf_all){
    status_value.byte = st;
  }
  else if (field == tsf_initial) {
    uint8_t info_init;
    info_init = st >> 4;
    info_init = info_init & 0x03;
    status_value.in_mode = info_init;
  }
  else if (field == tsf_mode) {
    uint8_t info_mode;
    info_mode = st >> 1;
    info_mode = info_mode & 0x07;
    status_value.count_mode = info_mode;
  }
  else if (field == tsf_base) {
    uint8_t info_bcd;
    info_bcd = st & 0x01;
    status_value.count_mode = info_bcd;
  }
  else
  {
    printf("Invalid field.");
    return 1;
  }

  int to_print = timer_print_config(timer, field, status_value);

  if (to_print != 0){
    printf("Error printing the timer config.");
    return to_print;

  }
  
  return 0;
}
