#include "log.h"
#include "state.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void logger_log(const char *status, const char *fmt, ...) {
  EXPECT(status != nullptr && fmt != nullptr,
         LOG_ERROR("status(%p) or fmt(%p) is null", status, fmt));
  va_list ap;
  va_start(ap, fmt);
  time_t now = time(nullptr);
  fprintf(stderr, "TIME: %.20s, STATUS: %.10s, ", ctime(&now), status);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  fflush(stderr);

  va_end(ap);
}

void logger_log_state(void) {
#define FORMAT "%#x"
  logger_log(STATUS_INFO,
             "STATE: REGISTERS: V0: " FORMAT ", V1: " FORMAT ", V2: " FORMAT
             ", V3: " FORMAT ", V4: " FORMAT ", V5: " FORMAT ", V6: " FORMAT
             ", V7: " FORMAT ", V8: " FORMAT ", V9: " FORMAT ", VA: " FORMAT
             ", VB: " FORMAT ", VC: " FORMAT ", VD: " FORMAT ", VE: " FORMAT
             ", VF: " FORMAT ", I: " FORMAT ", PC: " FORMAT ", SP: " FORMAT
             ", TIMERS: delay_timer: " FORMAT ", sound_timer: " FORMAT
             ", NEST: " FORMAT,
             STATE.registers.V0, STATE.registers.V1, STATE.registers.V2,
             STATE.registers.V3, STATE.registers.V4, STATE.registers.V5,
             STATE.registers.V6, STATE.registers.V7, STATE.registers.V8,
             STATE.registers.V9, STATE.registers.VA, STATE.registers.VB,
             STATE.registers.VC, STATE.registers.VD, STATE.registers.VE,
             STATE.registers.VF, STATE.registers.I.v, STATE.registers.PC.v,
             STATE.registers.SP.v, STATE.timers.delay, STATE.timers.sound, STATE.nest);
#undef FORMAT
  fflush(stderr);
}
