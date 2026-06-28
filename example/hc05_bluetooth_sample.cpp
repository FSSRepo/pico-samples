#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_BT uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define LED_PIN 25

static void send_command(const char* cmd) {
    uart_puts(UART_BT, cmd);
    uart_puts(UART_BT, "\r\n");
}

static void read_response(char* buffer, size_t len, uint32_t timeout_ms) {
    memset(buffer, 0, len);
    size_t idx = 0;
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0 && idx < len - 1) {
        if (uart_is_readable(UART_BT)) {
            buffer[idx++] = uart_getc(UART_BT);
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uart_init(UART_BT, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_BT, false, false);
    uart_set_format(UART_BT, 8, 1, UART_PARITY_NONE);

    sleep_ms(2000);
    printf("HC-05 Bluetooth Sample\r\n");
    printf("Enviando comandos AT...\r\n");

    char response[256];

    send_command("AT");
    read_response(response, sizeof(response), 1000);
    printf("AT -> %s\r\n", response);

    send_command("AT+NAME?");
    read_response(response, sizeof(response), 1000);
    printf("AT+NAME? -> %s\r\n", response);

    send_command("AT+ROLE?");
    read_response(response, sizeof(response), 1000);
    printf("AT+ROLE? -> %s\r\n", response);

    printf("Esperando datos Bluetooth (escribe desde el movil)...\r\n");

    while (true) {
        if (uart_is_readable(UART_BT)) {
            char c = uart_getc(UART_BT);
            printf("%c", c);

            if (c == '1') {
                gpio_put(LED_PIN, 1);
                uart_puts(UART_BT, "LED ON\r\n");
            } else if (c == '0') {
                gpio_put(LED_PIN, 0);
                uart_puts(UART_BT, "LED OFF\r\n");
            }
        }

        if (stdio_usb_connected()) {
            int ch = getchar_timeout_us(0);
            if (ch != PICO_ERROR_TIMEOUT && ch != EOF) {
                uart_putc(UART_BT, (char)ch);
            }
        }
    }

    return 0;
}
