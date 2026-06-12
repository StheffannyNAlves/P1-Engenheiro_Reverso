// Initial firmware
#include "app/app.h"
#include "bsp/board.h"
#include "display/lcd.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include "platform/platform_init.h"
#include "swd/swd_proto.h"
#include "target/target_ctrl.h"
#include "transport/usb_transport.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // Added for debugging

// Initial States
typedef enum {
    BOOT,
    PLATFORM_INIT,
    TARGET_HOLD_RESET,
    ENTER_SWD,
    READ_IDCODE,
    IDLE,
    ERROR,
    WATCHDOG_RESET
} fsm_state;

fsm_state current_state = BOOT;
bool is_lcd_ready = false; // Lock to initialize the bus only once

void fsm_run() {
    char msg[64] = {0};
    char id_str[32] = {0};
    switch (current_state) {
    case BOOT:
        // printf("Estado: BOOT\n");
        if (watchdog_caused_reboot()) {
            current_state = WATCHDOG_RESET;
        } else {
            current_state = PLATFORM_INIT;
        }
        break;

    case PLATFORM_INIT:
        printf("PLATFORM_INIT\n");
        board_init();
        transport_usb_init(); // printf("Estado: ENTER_SWD - Modo SWD configurado\n");

        target_ctrl_init();

        // Initialize the display only once
        if (!is_lcd_ready) {
            lcd_bus_init();
            lcd_init();
            is_lcd_ready = true;

            // Push the initial text to the screen
            lcd_set_state("SWD PROBE v1.0");
            lcd_set_error("SYS: BOOTING...");
        }
        // Call initialization only ONCE to avoid pointer corruption
        if (!platform_init()) {
            lcd_set_state("CRITICAL ERROR");
            lcd_set_error("PLATFORM FAIL");
            current_state = ERROR;
            break;
        }

        // Create a non-blocking 1.5 second timer using the Pico native timer
        static absolute_time_t timeout = {0};
        if (to_us_since_boot(timeout) == 0) {
            timeout = make_timeout_time_ms(1500); // Define o alvo de tempo futuro
        }

        // While 1.5 seconds have not passed, the Pico stays alive processing background tasks
        if (get_absolute_time() < timeout) {
            break; // Sai do switch e executa o resto do main (USB Task), mantendo o I2C limpo
        }

        // After the time expires, reset the static variable for the next boot and advance the state
        timeout = nil_time;
        target_reset_high();
        current_state = IDLE;
        break;

    case IDLE:
        // printf("Estado: IDLE\n");
        lcd_set_state("[ IDLE STATE ]  ");
        lcd_set_error("AWAITING HOST...");
        transport_command_t cmd = transport_process_command();
        // printf("Comando recebido: %d\n", cmd);
        if (cmd == CMD_START_SESSION) {
            current_state = TARGET_HOLD_RESET;
        }
        break;

    case TARGET_HOLD_RESET:
        // printf("Estado: TARGET_HOLD_RESET\n");
        target_reset_low();
        sleep_ms(10);
        lcd_set_state("[ TARGET RST ]");
        lcd_set_error("ASSERTING LOW...");

        current_state = ENTER_SWD;
        break;

    case ENTER_SWD:
        // printf("Estado: ENTER_SWD - Iniciando\n");
        lcd_set_state("[ LINK CONFIG ]");
        lcd_set_error("TX: JTAG -> SWD");

        transport_send_event((const uint8_t *)"DBG:ENTER_SWD\r\n", 15);
        app_enter_swd();
        target_reset_high();
        sleep_ms(2);

        // printf("Estado: ENTER_SWD - Modo SWD configurado\n");
        transport_send_event((const uint8_t *)"DBG:SWD_MODE\r\n", 14);
        current_state = READ_IDCODE;
        // printf("State: ENTER_SWD - Transition to READ_IDCODE\n");
        break;

    case READ_IDCODE:
        // printf("Estado: READ_IDCODE\n");
        lcd_set_state("[ DPIDR READ ]");
        lcd_set_error("REQ CORE ID...");

        uint32_t idcode = swd_read_idcode();
        // printf("IDCODE lido: %08lX\n", idcode);
        snprintf(msg, sizeof(msg), "DBG:IDCODE=0x%08X\r\n", (unsigned int)idcode);
        snprintf(id_str, sizeof(id_str), "ID: 0x%08X", (unsigned int)idcode);

        transport_send_event((const uint8_t *)msg, strlen(msg));
        if (idcode != 0x0BC12477) {
            transport_send_event((const uint8_t *)"DBG:IDCODE_FAIL\r\n", 21);
            lcd_set_state("SWD FAULT");
            lcd_set_error(id_str);
            current_state = ERROR;
        } else {
            transport_send_event((const uint8_t *)"DBG:IDCODE_OK\r\n", 17);
            lcd_set_state("IDCODE OK");
            lcd_set_error(id_str);
            sleep_ms(1500);
            current_state = IDLE;
        }
        break;

    case ERROR:
        printf("Estado: ERROR\n");

        // 1. Mostra o erro com o caractere customizado imediatamente
        lcd_set_state("\001 ERROR \001");
        lcd_set_error("BUS UNRESPONSIVE");

        // 2. FASE ESTÁTICA DO CARACTERE: Segura a tela travada por 3 segundos inteiros
        for (int i = 0; i < 3000; i += 10) {
            transport_usb_task(); // Mantém o USB respondendo ao Python durante os 3 segundos
            sleep_ms(10);
        }

        // 3. FASE PISCA-PISCA INFINITA: Começa a piscar o alerta de 500ms em 500ms
        bool tela_acesa = true;
        while (1) {
            tela_acesa = !tela_acesa;

            if (tela_acesa) {
                lcd_set_state("\001  !!!SYSTEM HAULT!!!  \001");
                lcd_set_error("BUS UNRESPONSIVE");
            } else {
                lcd_set_state(""); // Apaga a linha de cima para dar o efeito de pisca
            }

            // Segura o estado do pisca por 500ms mantendo o USB vivo
            for (int j = 0; j < 500; j += 10) {
                transport_usb_task(); // O script nunca vai cair por timeout
                sleep_ms(10);
            }
        }

    case WATCHDOG_RESET:
        // printf("Estado: WATCHDOG_RESET\n");
        lcd_set_state("WATCHDOG RESET");
        lcd_set_error("RECOVERING...");
        transport_set_watchdog_flag();
        current_state = PLATFORM_INIT;
        break;

    default:
        // printf("Estado: DESCONHECIDO\n");
        break;
    }
}

int main() {
    while (1) {
        fsm_run();
        transport_usb_task();
        // if (current_state != ERROR){
        //   watchdog_update();
        // }
    }

    return 0;
}