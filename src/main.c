#include "instructions.h"
#include "log.h"
#include "periph.h"
#include "state.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static_assert(CHAR_BIT == 8);

#ifdef LITTLE_ENDIAN
#define BSWAP16(x) __builtin_bswap16((x))
#else
#define BSWAP16(x) (x)
#endif
#define SEED (69) // nice

static int fclose_cleanup(FILE **f) { return fclose(*f); }
static void exit_cleanup(void) { display_exit(); }

long str_parse(const char *str) {
  char *end = nullptr;
  long addr = strtol(str, &end, 10);
  EXPECT(end == str + strlen(str), ({
           printf("Cant parse %s as argument\n", str);
           return -1;
         }));

  return addr;
}

typedef struct {
  char *prog_name;
  address_t start_address;
  instructions_per_second_t ips;
} args_t;

args_t get_args(int argc, char *argv[]) {
  args_t args = {nullptr, {PROGRAM_START}, DEFAULT_IPS};
  char option;
  long res;
  while ((option = getopt(argc, argv, "s:p:i:")) != -1) {
    switch (option) {
    case 'i':
      res = str_parse(optarg);
      if (res == -1) {
        printf("Invalid argument %s\n", optarg);
        goto err;
      }
      args.ips = res;
      break;
    case 's':
      res = str_parse(optarg);
      if (res == -1 || res >= MEMORY_SIZE) {
        printf("Invalid argument %s\n", optarg);
        goto err;
      }
      args.start_address.v = res;
      break;
    case 'p':
      args.prog_name = optarg;
      break;
    default:
      printf("Invalid option %c\n", option);
      goto err;
    }
  }

  if (args.prog_name == nullptr) {
    printf("prog cant be null");
    return (args_t){};
  }

  return args;
err:
  return (args_t){};
}

int main(int argc, char *argv[]) {
  atexit(&exit_cleanup);
  srand(SEED);

  args_t args = get_args(argc, argv);
  if (args.prog_name == nullptr)
    return EXIT_FAILURE;

  [[gnu::cleanup(fclose_cleanup)]] FILE *prog = fopen(args.prog_name, "rb");
  EXPECT(prog != nullptr, ({
           printf("Failed to read program");
           return EXIT_FAILURE;
         }));

  LOG_INFO("seed: %u", SEED);
  LOG_INFO("program: %s", args.prog_name);
  LOG_INFO("start address: %#x", args.start_address.v);

  EXPECT(state_init(args.ips, args.start_address, prog) != -1, ({
           printf("Failed to init state");
           return EXIT_FAILURE;
         }));

  EXPECT(display_init() != -1, ({ return EXIT_FAILURE; }));

  uint64_t useconds = 1'000'000 / (STATE.ips ? STATE.ips : 1'000'000);
  LOG_INFO("sleep: %lu", useconds);
  while (STATE.registers.PC.v < AVALIABLE_MEMORY_END) {
    instruction_t *i = state_memory_pointer(STATE.registers.PC);
    LOG_INFO("instruction value: %#x", BSWAP16(*i));
    EXPECT(execute(BSWAP16(*i)) != -1, LOG_ERROR("Invalid instruction %u", *i));
    if (STATE.timers.delay)
      STATE.timers.delay--;
    if (STATE.timers.sound)
      STATE.timers.sound--;
    LOG_STATE();
    usleep(useconds);
  }
  return EXIT_SUCCESS;
}
