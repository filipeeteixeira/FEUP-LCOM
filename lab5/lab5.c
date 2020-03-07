	// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8042.h"
#include "videocard.h"
#include "keyboard.h"
#include "sprite.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
	if (mode_setup(mode) != 0)
		return 1;

	sleep(delay);

	if (vg_exit() != 0)
		return 1;	

	return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

      message msg;
      int ipc_status;
      uint16_t r;
      uint8_t irq_set = 0;
      extern uint8_t key_pressed;

      if (keyboard_subscribe(&irq_set) != 0)
        return 1;
      
      // changing to the specified video mode
      vg_init(mode);
      
      vg_draw_rectangle(x,y,width,height,color);
      
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

                break;
              }
            default:

              break;
            }
        }
        else{
          
        }
      }

      if (keyboard_unsubscribe(&irq_set) != 0)
        return 1;

    vg_exit();

    

  return 0;   
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
	unsigned rectangle_width = getHres() / no_rectangles, rectangle_height = getVres() / no_rectangles;
	uint32_t color; // Color at any given time
	color = first;

	if (vg_init(mode) != 0)
		return 1;
	

	for (unsigned i = 0; i < no_rectangles; i++) // Row
	{
		for (unsigned j = 0; j < no_rectangles; j++) // Column
		{
			color_setup(&color, i, j, no_rectangles, first, step);

			vg_draw_rectangle(i * rectangle_width, j * rectangle_height, rectangle_width, rectangle_height, color); // Coordinates start at 0, cycle goes to iteration no_rectangles - 1
		}
	}

	// INSERIR PEDA�O DE C�DIGO IGUAL AO RECTANGLE (subscribe, while e unsubscibe)


	if (vg_exit() != 0)
		return 1;

	return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  /* To be completed */
  printf("%s(%8p, %u, %u): under construction\n", __func__, xpm, x, y);

  return 1;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  /* To be completed */
  printf("%s(%8p, %u, %u, %u, %u, %d, %u): under construction\n",
         __func__, xpm, xi, yi, xf, yf, speed, fr_rate);

  return 1;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
