#include "periph.h"
#include "log.h"
#include <locale.h>
#include <ncurses.h>

WINDOW *WIN = nullptr;

int display_init(void) {
  setlocale(LC_ALL, "");
  EXPECT(initscr(), ({
           LOG_ERROR("failed to init curses");
           return -1;
         }));
  cbreak();
  noecho();
  intrflush(stdscr, FALSE);
  curs_set(0);
  LOG_INFO("curses were intialized");

  // + 2 for borders
  uint32_t h = HEIGHT + 2;
  uint32_t w = WIDTH + 2;

  uint32_t rows, cols;
  getmaxyx(stdscr, rows, cols);
  LOG_INFO("screen size %ux%u", cols, rows);
  EXPECT(rows > h && cols > w, ({
           LOG_ERROR("screen to small");
           goto err;
         }));

  uint32_t start_y = rows / 2 - h / 2;
  uint32_t start_x = cols / 2 - w / 2;

  WIN = newwin(h, w, start_y, start_x);
  LOG_INFO("newwin data: %p, w: %u, h: %u, x: %u, y: %u", WIN, w, h, start_y,
           start_x);
  EXPECT(WIN, ({
           LOG_ERROR("failed to create subwindow.");
           goto err;
         }));

  display_clear();
  return 0;

err:
  display_exit();
  return -1;
}

void display_exit(void) {
  LOG_INFO("exiting display");
  if (WIN != nullptr)
    EXPECT(delwin(WIN) == OK, LOG_ERROR("failed to delete CHIP-window"));
  EXPECT(endwin() == OK, LOG_ERROR("endwin failed"));
}

void display_clear(void) {
  werase(WIN);
  box(WIN, '|', '-');
  wrefresh(WIN);
}

#define PIXEL '#'
#define BLANK ' '
bool display_draw(uint32_t y, uint32_t x, sprite_t s) {
  EXPECT(s.size > 0, ({
           LOG_ERROR("sprite size is out of range (0; 15]. sprite size: %u, "
                     "sprite data: %p",
                     s.size, s.data);
           return false;
         }));

  // 0,0 is the border of a screen. offset by one
  x = (x % WIDTH) + 1;
  y = (y % HEIGHT) + 1;

  bool overlap = false;
  uint32_t _x = x;
  uint32_t _y = y;
  uint8_t sprite_data[MAX_SPRITE_SIZE];

  for (uint32_t byte = 0; byte < s.size; byte++, _y += 1) {
    sprite_data[byte] = s.data[byte]; // copy byte
    uint8_t drawn = 0;
    // get already drawn byte
    for (int32_t bit = UINT8_WIDTH - 1; bit >= 0; bit--) {
      _x = x + (UINT8_WIDTH - bit - 1);
      drawn |= (mvwinch(WIN, _y, _x) == BLANK ? 0 : 1) << bit;
    }
    // check overlaps
    if ((drawn & sprite_data[byte]) != 0) {
      LOG_INFO("sprites overlap at byte %u. drawn sprite: %b, sprite_to_draw: "
               "%b. performing XOR",
               byte, drawn, sprite_data[byte]);
      overlap = true;
    }
    sprite_data[byte] ^= drawn; // xor in
  }

  // draw
  _x = x;
  _y = y;
  LOG_INFO("drawing sprite from %p of size %u at x(%u), y(%u)", s.data, s.size,
           _x, _y);
  for (uint32_t byte = 0; byte < s.size && _y <= HEIGHT; byte++, _y += 1) {
    for (int32_t bit = UINT8_WIDTH; bit >= 1 && x + UINT8_WIDTH - bit <= WIDTH;
         bit--) {
      _x = x + UINT8_WIDTH - bit;
      uint8_t pixel = sprite_data[byte] & BITu8(bit - 1) ? PIXEL : BLANK;
      mvwprintw(WIN, _y, _x, "%c", pixel);
    }
  }
  wrefresh(WIN);
  return overlap;
}

static constexpr int32_t KEY_LIST[] = {
    [CHIP_KEY_1] = '1', [CHIP_KEY_2] = '2', [CHIP_KEY_3] = '3',
    [CHIP_KEY_C] = '4', [CHIP_KEY_4] = 'q', [CHIP_KEY_5] = 'w',
    [CHIP_KEY_6] = 'e', [CHIP_KEY_D] = 'r', [CHIP_KEY_7] = 'a',
    [CHIP_KEY_8] = 's', [CHIP_KEY_9] = 'd', [CHIP_KEY_E] = 'f',
    [CHIP_KEY_A] = 'z', [CHIP_KEY_0] = 'x', [CHIP_KEY_B] = 'c',
    [CHIP_KEY_F] = 'v',
};

static void drop_input(void) {
  nodelay(WIN, true);
  while (wgetch(WIN) != ERR); // discard held key
  nodelay(WIN, false);
}

keys_t keyboard_get_key_nonblocking(void) {
  wtimeout(WIN, 250);
  int32_t ch = wgetch(WIN);
  wtimeout(WIN, -1);
  drop_input();

  if (ch == ERR)
    return CHIP_KEY_NONE;

  for (uint32_t i = 0; i < ARRAY_SIZE(KEY_LIST); i++) {
    if (ch == KEY_LIST[i])
      return (keys_t)i;
  }

  return CHIP_KEY_NONE;
}

keys_t keyboard_get_key_blocking(void) {
  while (true) {
    int32_t ch = wgetch(WIN);
    drop_input();
    while (wgetch(WIN) != ERR); // discard held key
    for (uint32_t i = 0; i < ARRAY_SIZE(KEY_LIST); i++) {
      if (ch == KEY_LIST[i])
        return (keys_t)i;
    }
  }
}

void sound_beep(void) {
  printf("\a");
  fflush(stdout);
}
