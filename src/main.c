// Initial firmware
#include "bsp/board.h"
#include "pico/stdlib.h"
#include "target/target_ctrl.h"
#include "transport/usb_transport.h"
#include <stdbool.h>
#include <stdint.h>

// Initial States
typedef enum { BOOT, PLATAFORM_INIT, IDLE, ERROR } fsm_state;

fsm_state current_state = BOOT;

void fsm_run() {
  switch (current_state) {
  case BOOT:
    current_state = PLATAFORM_INIT;
    break;
  case PLATAFORM_INIT:
    board_init();
    transport_usb_init();
    target_ctrl_init();
    current_state = IDLE;
    break;

  case IDLE:
    transport_process_commands();
    break;
  case ERROR:
    break;
  }
}

int main() {
  while (1) {
    fsm_run();

    transport_usb_task();
  }

  return 0;
}