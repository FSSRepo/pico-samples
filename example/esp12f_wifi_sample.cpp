#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

#define UART_WIFI uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define LED_PIN 25

static const char* WIFI_SSID = "TU_SSID";
static const char* WIFI_PASS = "TU_PASSWORD";

static void send_command(const char* cmd) {
    uart_puts(UART_WIFI, cmd);
    uart_puts(UART_WIFI, "\r\n");
}

static void read_response(char* buffer, size_t len, uint32_t timeout_ms) {
    memset(buffer, 0, len);
    size_t idx = 0;
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0 && idx < len - 1) {
        if (uart_is_readable(UART_WIFI)) {
            buffer[idx++] = uart_getc(UART_WIFI);
        }
    }
}

static bool wait_for(const char* expected, uint32_t timeout_ms) {
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    size_t idx = 0;
    size_t exp_len = strlen(expected);
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0) {
        if (uart_is_readable(UART_WIFI)) {
            char c = uart_getc(UART_WIFI);
            if (idx < sizeof(buffer) - 1) {
                buffer[idx++] = c;
                buffer[idx] = '\0';
            }
            if (strstr(buffer, expected) != NULL) {
                return true;
            }
        }
    }
    return false;
}

static bool send_and_wait(const char* cmd, const char* expected, uint32_t timeout_ms) {
    send_command(cmd);
    return wait_for(expected, timeout_ms);
}

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uart_init(UART_WIFI, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_WIFI, false, false);
    uart_set_format(UART_WIFI, 8, 1, UART_PARITY_NONE);

    sleep_ms(3000);
    printf("ESP-12F (ESP8266) WiFi Sample\r\n");

    char response[512];

    printf("Reiniciando modulo...\r\n");
    send_command("AT+RST");
    sleep_ms(3000);
    while (uart_is_readable(UART_WIFI)) {
        uart_getc(UART_WIFI);
    }

    printf("Verificando comunicacion...\r\n");
    if (!send_and_wait("AT", "OK", 2000)) {
        printf("ERROR: No se comunica con ESP-12F\r\n");
        while (true) tight_loop_contents();
    }
    printf("Modulo responde OK\r\n");

    send_and_wait("AT+CWMODE=1", "OK", 2000);
    printf("Modo STA configurado\r\n");

    printf("Conectando a WiFi: %s\r\n", WIFI_SSID);
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
    if (!send_and_wait(cmd, "OK", 15000)) {
        printf("ERROR: No se pudo conectar al WiFi\r\n");
        while (true) tight_loop_contents();
    }
    printf("Conectado al WiFi\r\n");

    send_command("AT+CIFSR");
    read_response(response, sizeof(response), 3000);
    printf("IP local: %s\r\n", response);

    printf("Configurando servidor TCP en puerto 80...\r\n");
    send_and_wait("AT+CIPMUX=1", "OK", 2000);
    send_and_wait("AT+CIPSERVER=1,80", "OK", 2000);
    printf("Servidor activo en puerto 80\r\n");

    printf("Esperando conexiones...\r\n");
    while (true) {
        if (uart_is_readable(UART_WIFI)) {
            memset(response, 0, sizeof(response));
            read_response(response, sizeof(response), 500);
            if (strlen(response) > 0) {
                printf("Recibido: %s\r\n", response);

                if (strstr(response, "+IPD") != NULL) {
                    char* ipd = strstr(response, "+IPD,");
                    if (ipd != NULL) {
                        int link_id = 0;
                        sscanf(ipd, "+IPD,%d", &link_id);

                        const char* html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"
                                           "<html><body><h1>Hola desde RP Pico + ESP-12F</h1>"
                                           "<p>LED: <a href=\"/on\">ON</a> | <a href=\"/off\">OFF</a></p></body></html>";

                        char cipsend[64];
                        snprintf(cipsend, sizeof(cipsend), "AT+CIPSEND=%d,%d", link_id, (int)strlen(html));
                        send_command(cipsend);
                        sleep_ms(100);
                        uart_puts(UART_WIFI, html);

                        sleep_ms(500);
                        char cipclose[32];
                        snprintf(cipclose, sizeof(cipclose), "AT+CIPCLOSE=%d", link_id);
                        send_command(cipclose);

                        if (strstr(response, "/on") != NULL) {
                            gpio_put(LED_PIN, 1);
                            printf("LED encendido por web\r\n");
                        } else if (strstr(response, "/off") != NULL) {
                            gpio_put(LED_PIN, 0);
                            printf("LED apagado por web\r\n");
                        }
                    }
                }
            }
        }

        if (stdio_usb_connected()) {
            int ch = getchar_timeout_us(0);
            if (ch != PICO_ERROR_TIMEOUT && ch != EOF) {
                uart_putc(UART_WIFI, (char)ch);
            }
        }
    }

    return 0;
}
