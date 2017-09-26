#ifndef GLYPH_icon_BPP
#define GLYPH_icon_WIDTH 16
#define GLYPH_icon_HEIGHT 16
#define GLYPH_icon_BPP 1
extern
unsigned int const C_icon_colors[]
;
extern	
unsigned char const C_icon_bitmap[];
#ifdef OS_IO_SEPROXYHAL
#include "os_io_seproxyhal.h"
extern
const bagl_icon_details_t C_icon;
#endif // GLYPH_icon_BPP
#endif // OS_IO_SEPROXYHAL
