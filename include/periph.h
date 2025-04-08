#ifndef PERIPH_H
#define PERIPH_H

#include <stdint.h>

#define WIDTH (64)
#define HEIGHT (32)
#define MAX_SPRITE_SIZE (15)
#define BASE_SPRITES_SIZE (5)
#define SPRITE_WIDTH (8)

typedef uint8_t ubyte_t;
typedef struct {
  ubyte_t *data;
  ubyte_t size : 4; // between 1 - 15
} sprite_t;

typedef enum : uint16_t {
  // first row
  CHIP_KEY_1 = 0x1,
  CHIP_KEY_2 = 0x2,
  CHIP_KEY_3 = 0x3,
  CHIP_KEY_C = 0xC,

  // second row
  CHIP_KEY_4 = 0x4,
  CHIP_KEY_5 = 0x5,
  CHIP_KEY_6 = 0x6,
  CHIP_KEY_D = 0xD,

  // third row
  CHIP_KEY_7 = 0x7,
  CHIP_KEY_8 = 0x8,
  CHIP_KEY_9 = 0x9,
  CHIP_KEY_E = 0xE,

  // fourth row
  CHIP_KEY_A = 0xA,
  CHIP_KEY_0 = 0x0,
  CHIP_KEY_B = 0xB,
  CHIP_KEY_F = 0xF,

  CHIP_KEY_NONE = UINT16_MAX,
} keys_t;

typedef struct _win_st WINDOW;

extern int display_init(void);
extern void display_exit(void);
extern void display_clear(void);
extern bool display_draw(uint32_t y, uint32_t x, sprite_t s);
extern void sound_beep(void);
extern keys_t keyboard_get_key_nonblocking(void);
extern keys_t keyboard_get_key_blocking(void);

#endif
