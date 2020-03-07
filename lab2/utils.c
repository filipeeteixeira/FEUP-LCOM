#include <lcom/lcf.h>
#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  uint8_t new_value;

  new_value = val & 0x0FF;
  *lsb = new_value;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  uint8_t new_value;

  new_value = val >> 8;
  *msb = new_value;
  return 0;
}

int(util_sys_inb)(int port, uint8_t *value) {
  uint32_t b;

  if(sys_inb(port, &b) != 0)
	return 1;

  *value = b & 0x000000FF;

  return 0;
}
