#include "instructions.h"
#include "periph.h"
#include "state.h"
#include <limits.h>
#include <stdlib.h>

#define REGISTER_FROM(value, nibble)                                           \
  (enum gp_registers_t)(((value) & (0xF << ((nibble) * (CHAR_BIT / 2)))) >>    \
                        ((nibble) * (CHAR_BIT / 2)))
#define VALUE_FROM(value) ((value) & 0xFF)
#define ADDRESS_FROM(value) ((address_t){(value) & 0xFFF})

#define INSTRUCTION static void

/* 0x00E0 */
INSTRUCTION clear(void) { display_clear(); }

/* 0x00EE */
INSTRUCTION ret(void) {
  if (STATE.nest == 0)
    return;

  STATE.nest--;
  STATE.registers.PC = state_sp_ld(); // load address
}

/* 0x1NNN */
INSTRUCTION jump(address_t a) { STATE.registers.PC = a; }

/* 0x2NNN */
INSTRUCTION call(address_t a) {
  if (STATE.nest == MAX_NEST)
    return;

  STATE.nest++;
  state_sp_str(STATE.registers.PC); // store
  STATE.registers.PC = a;           // load address
}

/* 0x3XNN */
INSTRUCTION rl_eq_si(enum gp_registers_t v, uint8_t value) {
  STATE.registers.PC.v +=
      *state_register_value(v) == value ? INSTRUCTION_SIZE : 0;
}

/* 0x4XNN */
INSTRUCTION rl_neq_si(enum gp_registers_t v, uint8_t value) {
  STATE.registers.PC.v +=
      *state_register_value(v) != value ? INSTRUCTION_SIZE : 0;
}

/* 0x5XY0 */
INSTRUCTION rr_eq_si(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t _vx = *state_register_value(vx);
  gp_register_value_t _vy = *state_register_value(vy);
  STATE.registers.PC.v += _vx == _vy ? INSTRUCTION_SIZE : 0;
}

/* 0x6XNN */
INSTRUCTION rl_ld(enum gp_registers_t v, uint8_t value) {
  gp_register_value_t *reg = state_register_value(v);
  *reg = value;
}

/* 0x7XNN */
INSTRUCTION rl_add(enum gp_registers_t v, uint8_t value) {
  gp_register_value_t *reg = state_register_value(v);
  *reg += value;
}

/* 0x8XY0 */
INSTRUCTION rr_ld(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t *_vy = state_register_value(vy);
  *_vx = *_vy;
}

/* 0x8XY1 */
INSTRUCTION rr_orr(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t *_vy = state_register_value(vy);
  *_vx |= *_vy;
  STATE.registers.VF = 0;
}

/* 0x8XY2 */
INSTRUCTION rr_and(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t *_vy = state_register_value(vy);
  *_vx &= *_vy;
  STATE.registers.VF = 0;
}

/* 0x8XY3 */
INSTRUCTION rr_xor(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t *_vy = state_register_value(vy);
  *_vx ^= *_vy;
  STATE.registers.VF = 0;
}

/* 0x8XY4 */
INSTRUCTION rr_add(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t *_vy = state_register_value(vy);
  uint16_t res = (uint16_t)*_vx + (uint16_t)*_vy;
  gp_register_value_t vf = res > UINT8_MAX ? 1 : 0; // set VF if overflow
  *_vx = res & (uint8_t)0xFF;                       // store lower 8 bits
  STATE.registers.VF = vf;
}

/* 0x8XY5 */
INSTRUCTION rr_sub(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t _vy = *state_register_value(vy);
  gp_register_value_t vf = *_vx >= _vy ? 1 : 0; // clear VF if underflow
  *_vx = *_vx - _vy;
  STATE.registers.VF = vf;
}

/* 0x8XY6 */
INSTRUCTION rr_ld_shr(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t _vy = *state_register_value(vy);
  gp_register_value_t *_vx = state_register_value(vx);
  *_vx = _vy;
  gp_register_value_t vf = *_vx & 0x1;
  *_vx >>= 1;
  STATE.registers.VF = vf;
}

/* 0x8XY7 */
INSTRUCTION rr_sub_reversed(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t *_vx = state_register_value(vx);
  gp_register_value_t _vy = *state_register_value(vy);
  gp_register_value_t vf = _vy >= *_vx ? 1 : 0; // clear VF if underflow
  *_vx = _vy - *_vx;
  STATE.registers.VF = vf;
}

/* 0x8XYE */
INSTRUCTION rr_ld_shl(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t _vy = *state_register_value(vy);
  gp_register_value_t *_vx = state_register_value(vx);
  *_vx = _vy;
  gp_register_value_t vf = (*_vx) >> 7;
  *_vx <<= 1;
  STATE.registers.VF = vf;
}

/* 0x9XY0 */
INSTRUCTION rr_neq_si(enum gp_registers_t vx, enum gp_registers_t vy) {
  gp_register_value_t _vx = *state_register_value(vx);
  gp_register_value_t _vy = *state_register_value(vy);
  STATE.registers.PC.v += _vx != _vy ? INSTRUCTION_SIZE : 0;
}

/* 0xANNN */
INSTRUCTION I_ld(address_t a) { STATE.registers.I = a; }

/* 0xBNNN */
INSTRUCTION jump_v0(address_t a) {
  STATE.registers.PC.v = a.v + STATE.registers.V0;
}

/* 0xCXNN */
INSTRUCTION get_rand(enum gp_registers_t v, uint8_t value) {
  gp_register_value_t *_v = state_register_value(v);
  *_v = rand() & value;
}

/* 0xDXYN */
INSTRUCTION draw(enum gp_registers_t vx, enum gp_registers_t vy,
                 half_byte_t value) {
  gp_register_value_t _vx = *state_register_value(vx);
  gp_register_value_t _vy = *state_register_value(vy);

  sprite_t s;
  s.data = state_memory_pointer(STATE.registers.I);
  s.size = value.v;
  STATE.registers.VF = display_draw(_vy, _vx, s);
}

/* 0xEX9E */
INSTRUCTION rk_eq_si(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  STATE.registers.PC.v +=
      _v == keyboard_get_key_nonblocking() ? INSTRUCTION_SIZE : 0;
}

/* 0xEXA1 */
INSTRUCTION rk_neq_si(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  STATE.registers.PC.v +=
      _v != keyboard_get_key_nonblocking() ? INSTRUCTION_SIZE : 0;
}

/* 0xFX07 */
INSTRUCTION get_delay_timer(enum gp_registers_t v) {
  gp_register_value_t *_v = state_register_value(v);
  *_v = STATE.timers.delay;
}

/* 0xFX0A */
INSTRUCTION get_key(enum gp_registers_t v) {
  gp_register_value_t *_v = state_register_value(v);
  *_v = keyboard_get_key_blocking();
}

/* 0xFX15 */
INSTRUCTION set_delay_timer(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  STATE.timers.delay = _v;
}
/* 0xFX18 */
INSTRUCTION set_sound_timer(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  STATE.timers.sound = _v;
}

/* I = I + v. */
/* 0xFX1E */
INSTRUCTION I_add(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  STATE.registers.I.v += _v;
}

/* Set I = location of sprite for digit Vx. */
/* 0xFX29 */
INSTRUCTION I_ld_sprite(enum gp_registers_t v) {
  // sprite size for those always 5 bytes
  STATE.registers.I.v = *state_register_value(v) * BASE_SPRITES_SIZE;
}

/* Stores the binary-coded decimal representation of VX, */
/* with the hundreds digit in memory at location in I, */
/* the tens digit at location I+1, and the ones digit at location I+2. */
/* 0xFX33 */
INSTRUCTION bcd_str(enum gp_registers_t v) {
  gp_register_value_t _v = *state_register_value(v);
  uint8_t hundreds = _v / 100;
  uint8_t tens = (_v / 10) % 10;
  uint8_t ones = _v % 10;

  uint8_t *p = (uint8_t *)STATE.mmap + STATE.registers.I.v;
  p[0] = hundreds;
  p[1] = tens;
  p[2] = ones;
}

/* inclusive. */
/* 0xFX55 */
INSTRUCTION register_dump(enum gp_registers_t v_end) {
  gp_register_value_t *reg = &STATE.registers.V0;
  gp_register_value_t *dest = state_memory_pointer(STATE.registers.I);
  gp_register_value_t cur = REG_V0;
  STATE.registers.I.v += v_end + 1;
  do
    *dest++ = *reg++;
  while (cur++ != v_end);
}

/* inclusive. */
/* 0xFX65 */
INSTRUCTION register_load(enum gp_registers_t v_end) {
  gp_register_value_t *reg = &STATE.registers.V0;
  gp_register_value_t *dest = state_memory_pointer(STATE.registers.I);
  gp_register_value_t cur = REG_V0;
  STATE.registers.I.v += v_end + 1;
  do
    *reg++ = *dest++;
  while (cur++ != v_end);
}

int execute(instruction_t i) {
  STATE.registers.PC.v += INSTRUCTION_SIZE;
  uint16_t top_nibble = i & 0xF000;
  switch (top_nibble >> 12) {
  case 0x0:
    if ((i & 0x00E0) == i) {
      clear();
      break;
    } else if ((i & 0x00EE) == i) {
      ret();
      break;
    }
    return -1;
  case 0x1:
    jump(ADDRESS_FROM(i));
    break;
  case 0x2:
    call(ADDRESS_FROM(i));
    break;
  case 0x3:
    rl_eq_si(REGISTER_FROM(i, 2), VALUE_FROM(i));
    break;
  case 0x4:
    rl_neq_si(REGISTER_FROM(i, 2), VALUE_FROM(i));
    break;
  case 0x5:
    if ((i & 0xF) != 0)
      return -1;
    rr_eq_si(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
    break;
  case 0x6:
    rl_ld(REGISTER_FROM(i, 2), VALUE_FROM(i));
    break;
  case 0x7:
    rl_add(REGISTER_FROM(i, 2), VALUE_FROM(i));
    break;
  case 0x8:
    switch (i & 0xF) {
    case 0x0:
      rr_ld(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x1:
      rr_orr(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x2:
      rr_and(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x3:
      rr_xor(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x4:
      rr_add(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x5:
      rr_sub(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x6:
      rr_ld_shr(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0x7:
      rr_sub_reversed(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    case 0xE:
      rr_ld_shl(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
      break;
    default:
      return -1;
    }
    break;
  case 0x9:
    if ((i & 0xF) != 0)
      return -1;
    rr_neq_si(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1));
    break;
  case 0xA:
    I_ld(ADDRESS_FROM(i));
    break;
  case 0xB:
    jump_v0(ADDRESS_FROM(i));
    break;
  case 0xC:
    get_rand(REGISTER_FROM(i, 2), VALUE_FROM(i));
    break;
  case 0xD:
    draw(REGISTER_FROM(i, 2), REGISTER_FROM(i, 1), (half_byte_t){i & 0xF});
    break;
  case 0xE:
    if ((i & 0xFF) == 0x9E) {
      rk_eq_si(REGISTER_FROM(i, 2));
      break;
    } else if ((i & 0xFF) == 0xA1) {
      rk_neq_si(REGISTER_FROM(i, 2));
      break;
    }
    return -1;
  case 0xF:
    switch (i & 0xFF) {
    case 0x07:
      get_delay_timer(REGISTER_FROM(i, 2));
      break;
    case 0x0A:
      get_key(REGISTER_FROM(i, 2));
      break;
    case 0x15:
      set_delay_timer(REGISTER_FROM(i, 2));
      break;
    case 0x18:
      set_sound_timer(REGISTER_FROM(i, 2));
      break;
    case 0x1E:
      I_add(REGISTER_FROM(i, 2));
      break;
    case 0x29:
      I_ld_sprite(REGISTER_FROM(i, 2));
      break;
    case 0x33:
      bcd_str(REGISTER_FROM(i, 2));
      break;
    case 0x55:
      register_dump(REGISTER_FROM(i, 2));
      break;
    case 0x65:
      register_load(REGISTER_FROM(i, 2));
      break;
    default:
      return -1;
    }
    break;
  default:
    return -1;
  }
  return 0;
}
