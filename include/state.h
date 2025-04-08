#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stddef.h>

#define MEMORY_SIZE (4096)
#define AVALIABLE_MEMORY_END (offsetof(typeof(*STATE.mmap), _stack))
// STACK is 48 bytes up to 12 level of nesting
#define STACK_START (0xED0)
#define STACK_END (0xEA0)
#define MAX_NEST (12)
#define PROGRAM_START (0x200)
#define DEFAULT_IPS (100)

// gp - general purpose
typedef uint8_t gp_register_value_t;
typedef uint8_t timers_t;
typedef uint16_t instructions_per_second_t;

typedef struct {
  uint8_t v : 4;
} half_byte_t;

typedef struct {
  uint16_t v : 12;
} _12_bit_t;

typedef _12_bit_t address_register_t;
typedef _12_bit_t address_t;
typedef _12_bit_t pc_t;
typedef _12_bit_t sp_t;
typedef _12_bit_t program_size_t;

enum gp_registers_t : uint32_t {
  REG_V0,
  REG_V1,
  REG_V2,
  REG_V3,
  REG_V4,
  REG_V5,
  REG_V6,
  REG_V7,
  REG_V8,
  REG_V9,
  REG_VA,
  REG_VB,
  REG_VC,
  REG_VE,
  REG_VD,
  REG_VF,
};

typedef struct {
  struct {
    gp_register_value_t V0;
    gp_register_value_t V1;
    gp_register_value_t V2;
    gp_register_value_t V3;
    gp_register_value_t V4;
    gp_register_value_t V5;
    gp_register_value_t V6;
    gp_register_value_t V7;
    gp_register_value_t V8;
    gp_register_value_t V9;
    gp_register_value_t VA;
    gp_register_value_t VB;
    gp_register_value_t VC;
    gp_register_value_t VE;
    gp_register_value_t VD;
    gp_register_value_t VF; // doubles as flag
    address_register_t I;
    pc_t PC;
    sp_t SP;
  } registers;

  struct {
    timers_t delay;
    timers_t sound;
  } timers;

  struct {
    uint8_t sprites[512];         // 0x000 - 0x1FF used for sprites
    uint8_t memory[3232];         // 0x200 - 0xE9F avaliable memory
    uint8_t _stack[96];           // 0xEA0 - 0xEFF internal use
    uint8_t display_refresh[256]; // 0xF00 - 0xFFF self-explanatory
  } *mmap;

  instructions_per_second_t ips;
  uint8_t nest;
  const uint8_t __padding[5];
} state_t;

extern state_t STATE;
typedef struct _IO_FILE FILE;
extern int state_init(instructions_per_second_t ips, address_t program_start, FILE *prog);
// clears registers, stack. resets base sprites and sets PC to value of
// `program_start`. All memory modifications preserved. (except stack)
extern void state_reset(instructions_per_second_t ips, address_t program_start);
// modifies STATE.registers.SP
extern address_t state_sp_ld(void);
// modifies STATE.registers.SP
extern void state_sp_str(address_t offset);

inline void *state_memory_pointer(address_t a) {
  return (uint8_t *)STATE.mmap + a.v;
}

inline gp_register_value_t *state_register_value(enum gp_registers_t type) {
  return (gp_register_value_t *)&STATE.registers + type;
}

#endif
