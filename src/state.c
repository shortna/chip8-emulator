#include "state.h"
#include "log.h"
#include "periph.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

state_t STATE = {};

static int state_load_program(FILE *prog) {
  address_t program_size, avaliable_size;
  avaliable_size = (address_t){sizeof(STATE.mmap->memory)};

  fseek(prog, 0, SEEK_END); // seek to end of file
  program_size = (address_t){ftell(prog)};
  fseek(prog, 0, SEEK_SET); // seek back to beginning of file

  EXPECT(program_size.v >= 0 && program_size.v <= avaliable_size.v,
         ({ return -1; }));
  EXPECT(fread((uint8_t *)STATE.mmap + STATE.registers.PC.v,
               sizeof(*STATE.mmap->memory), avaliable_size.v, prog) != 0,
         ({ return -1; }));
  return 0;
}

void state_reset(instructions_per_second_t ips, address_t program_start) {
  EXPECT(STATE.mmap != nullptr, ({ LOG_PANIC("STATE was not initialized"); }));
  memset(&STATE.registers, 0, sizeof(STATE.registers));
  memset(STATE.mmap->_stack, 0, sizeof(STATE.mmap->_stack));

  {
    const uint8_t _base_sprites[][BASE_SPRITES_SIZE] = {
        {0xF0, 0x90, 0x90, 0x90, 0xF0}, // 0
        {0x20, 0x60, 0x20, 0x20, 0x70}, // 1
        {0xF0, 0x10, 0xF0, 0x80, 0xF0}, // 2
        {0xF0, 0x10, 0xF0, 0x10, 0xF0}, // 3
        {0x90, 0x90, 0xF0, 0x10, 0x10}, // 4
        {0xF0, 0x80, 0xF0, 0x10, 0xF0}, // 5
        {0xF0, 0x80, 0xF0, 0x90, 0xF0}, // 6
        {0xF0, 0x10, 0x20, 0x40, 0x40}, // 7
        {0xF0, 0x90, 0xF0, 0x90, 0xF0}, // 8
        {0xF0, 0x90, 0xF0, 0x10, 0xF0}, // 9
        {0xF0, 0x90, 0xF0, 0x90, 0x90}, // A
        {0xE0, 0x90, 0xE0, 0x90, 0xE0}, // B
        {0xF0, 0x80, 0x80, 0x80, 0xF0}, // C
        {0xE0, 0x90, 0x90, 0x90, 0xE0}, // D
        {0xF0, 0x80, 0xF0, 0x80, 0xF0}, // E
        {0xF0, 0x80, 0xF0, 0x80, 0x80}, // F
    };
    memcpy(STATE.mmap->sprites, _base_sprites,
           ARRAY_SIZE(_base_sprites) * ARRAY_SIZE(*_base_sprites));
  }

  STATE.nest = 0;
  STATE.ips = ips;
  STATE.registers.PC = program_start;
  STATE.registers.SP = (sp_t){STACK_START};
  LOG_INFO("STATE was reinitialized");
}

int state_init(instructions_per_second_t ips, address_t program_start, FILE *prog) {
  STATE.mmap = calloc(1, MEMORY_SIZE);
  EXPECT(STATE.mmap != nullptr, ({ LOG_PANIC("Failed to malloc memory"); }));
  state_reset(ips, program_start);
  EXPECT(state_load_program(prog) != -1, ({
           LOG_ERROR("Failed to read program");
           return -1;
         }));
  LOG_STATE();
  return 0;
}

address_t state_sp_ld(void) {
  if (STATE.registers.SP.v >= STACK_START)
    return STATE.registers.PC;

  STATE.registers.SP.v += sizeof(STATE.registers.SP);
  address_t *a = state_memory_pointer(STATE.registers.SP);
  return *a;
}

void state_sp_str(address_t offset) {
  if (STATE.registers.SP.v <= STACK_END)
    return;

  address_t *p = state_memory_pointer(STATE.registers.SP);
  *p = offset;
  STATE.registers.SP.v -= sizeof(STATE.registers.SP);
}
