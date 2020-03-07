// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include "videocard.h"

vbe_mode_info_t vmi_p;
int r;
struct minix_mem_range mr; /* physical memory range */
static unsigned vram_base; /* VRAM  physical addresss */
static unsigned	vram_size; /* VRAM size */
static void* video_mem;
static uint8_t bits_per_pixel;
static uint8_t bytes_per_pixel;
static uint8_t RedMaskSize, RedFieldPosition, GreenMaskSize, GreenFieldPosition, BlueMaskSize, BlueFieldPosition;
static uint16_t h_res;
static uint16_t v_res;


void* (vg_init)(uint16_t mode) {

	 //start memory map
  	//memset(&vmi_p, 0, sizeof(vmi_p)); //clear mem_info

	if (vbe_get_mode_info(mode, &vmi_p) != 0){ 
    	printf("failed to get mode \n");
    	return NULL;
  	}

	h_res = vmi_p.XResolution;
  	v_res = vmi_p.YResolution;

  	bits_per_pixel = vmi_p.BitsPerPixel;
	bytes_per_pixel = (bits_per_pixel+7)/8;

	RedMaskSize = vmi_p.RedMaskSize;
	GreenMaskSize = vmi_p.GreenMaskSize;
	BlueMaskSize = vmi_p.BlueMaskSize;

	RedFieldPosition = vmi_p.RedFieldPosition;
	GreenFieldPosition = vmi_p.GreenFieldPosition;
	BlueFieldPosition = vmi_p.BlueFieldPosition;

	if (mode_setup(mode) != 0)
		return NULL;

	memory_map();

	return video_mem;
}

void color_setup(uint32_t* color, unsigned row, unsigned column, uint8_t no_rectangles, uint32_t first, uint8_t step)
{
	if (bits_per_pixel == 8) { // Indexed mode
		*color = (first + (row * no_rectangles + column) * step) % (1 << bits_per_pixel);
	}
	else // Direct mode
	{
		uint8_t red_bits = decompose_color(first, RedMaskSize, RedFieldPosition);
		uint8_t green_bits = decompose_color(first, GreenMaskSize, GreenFieldPosition);
		uint8_t blue_bits = decompose_color(first, BlueMaskSize, BlueFieldPosition);

		uint32_t red = (red_bits + column * step) % (1 << RedMaskSize);
		uint32_t green = (green_bits + column * step) % (1 << GreenMaskSize);
		uint32_t blue = (blue_bits + column * step) % (1 << BlueMaskSize);

		*color = (red << RedFieldPosition) | (green << GreenFieldPosition) | (blue << BlueFieldPosition);
	}
}

uint8_t decompose_color(uint32_t color, unsigned mask_size, unsigned field_position) {
	uint32_t temp = color >> field_position;

	uint32_t mask = pow(2, mask_size) - 1;

	return (uint8_t)(temp & mask);
}

void memory_map() {
	/* Allow memory mapping */
	
	mr.mr_base = (phys_bytes) vram_base;
	mr.mr_limit = mr.mr_base + vram_size;

	
	if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("sys_privctl (ADD_MEM) failed: %d\n", r);
	
	vram_base = vmi_p.PhysBasePtr;
  	vram_size = h_res * v_res * bytes_per_pixel;
	
	/* Map memory */
	video_mem = vm_map_phys(SELF, (void*)mr.mr_base, vram_size);
	if (video_mem == MAP_FAILED) {
		panic("couldnt map video memory");
	}

}

int mode_setup(uint16_t mode) {
	struct reg86 r;
	memset(&r, 0, sizeof(r));

	r.ax = 0x4F02; // VBE call, function 02 -- set VBE mode
	r.bx = 1 << 14 | mode; // set bit 14: linear framebuffer
	r.intno = 0x10;

	if (sys_int86(&r) != OK) {
		printf("set_vbe_mode: sys_int86() failed \n");
		return 1;
	}
	return 0;
}

int Fillpixel(uint16_t x, uint16_t y, uint32_t color)
{
	/* if (x >= h_res || y >= v_res)
		return 1;*/
	
	uint8_t *adr = video_mem;
  		adr += (h_res * y * bytes_per_pixel); //numero total de bytes 
  		adr += (x * bytes_per_pixel);

	for (size_t i = 0; i < bytes_per_pixel; i++) {
		*adr = (color >> (i * 8));
		adr++;
		printf("adr: %u", adr);
	}
  return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
	/* if (x > h_res || y > v_res)
    	return 1;*/
	
	for (int i = 0; i < len; i++)
	{
		Fillpixel(x+i,y,color);
  	}
    return 0;
}

int (vg_draw_rectangle) (uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){
	//printf("6\n");

	//width and heigth in number of pixels
  for (size_t i = 0; i < height; i++)
  {
    
    if (vg_draw_hline(x, y + i, width, color) != 0)
      return 1;
  }
  return 0;
}

uint16_t getHres() {
	return h_res;
}

uint16_t getVres() {
	return v_res;
}
