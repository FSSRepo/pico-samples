#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "st7735.h"
#include "pwm_lib.h"
#include "logo.h"

#define MOTOR_PWM_PIN 16

#define SPI_PORT spi0
#define SPI_MOSI 3
#define SPI_SCK 2
#define SPI_CS 7
#define SPI_DC 6
#define SPI_RST 5

// HC-05 Bluetooth UART
#define HC05_UART uart1
#define HC05_TX_PIN 8
#define HC05_RX_PIN 9
#define HC05_BAUDRATE 9600
#define HC05_STATE_PIN 10

static bool motor_on = false;

static void hc05_init() {
    uart_init(HC05_UART, HC05_BAUDRATE);
    gpio_set_function(HC05_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC05_RX_PIN, GPIO_FUNC_UART);

    gpio_init(HC05_STATE_PIN);
    gpio_set_dir(HC05_STATE_PIN, GPIO_IN);
    gpio_pull_down(HC05_STATE_PIN);
}

static bool hc05_is_connected() {
    return gpio_get(HC05_STATE_PIN);
}

static float simulated_temp = 25.0f;
static float simulated_hum = 55.0f;

static void update_simulated_values() {
    // Simula pequeña variacion aleatoria para que parezca real
    int t = rand() % 5 - 2;  // -2 a +2
    int h = rand() % 7 - 3;  // -3 a +3
    simulated_temp += t * 0.1f;
    simulated_hum += h * 0.1f;
    if (simulated_temp < 10.0f) simulated_temp = 10.0f;
    if (simulated_temp > 40.0f) simulated_temp = 40.0f;
    if (simulated_hum < 20.0f) simulated_hum = 20.0f;
    if (simulated_hum > 90.0f) simulated_hum = 90.0f;
}

static void hc05_send_json(float temp, float hum, bool bt_state, bool pump_on) {
    char msg[128];
    snprintf(msg, sizeof(msg), "{\"temp\":%.1f,\"hum\":%.1f,\"bt_state\":%s,\"pump\":%s}\n",
             temp, hum, bt_state ? "true" : "false", pump_on ? "true" : "false");
    uart_puts(HC05_UART, msg);
}

static void process_uart_commands(PWM* motor, uint32_t top) {
    static char rx_buf[64];
    static int rx_idx = 0;

    while (uart_is_readable(HC05_UART)) {
        char c = uart_getc(HC05_UART);
        if (c == '\n' || c == '\r') {
            if (rx_idx > 0) {
                rx_buf[rx_idx] = '\0';

                // Comando: {"pump":true} o {"pump":false}
                char* p = strstr(rx_buf, "\"pump\"");
                if (p) {
                    p = strchr(p, ':');
                    if (p) {
                        if (strstr(p + 1, "true")) {
                            motor_on = true;
                        } else if (strstr(p + 1, "false")) {
                            motor_on = false;
                        }
                    }
                }

                rx_idx = 0;
            }
        } else {
            if (rx_idx < (int)sizeof(rx_buf) - 1) {
                rx_buf[rx_idx++] = c;
            }
        }
    }

    if (motor_on) {
        uint32_t duty = (uint32_t)(top * 0.7f);
        motor->duty(duty);
    } else {
        motor->duty(0);
    }
}

int main() {
    stdio_init_all();
    srand((unsigned)time_us_32());

    hc05_init();

    ST7735* display = new ST7735(SPI_PORT, SPI_MOSI, SPI_SCK, SPI_CS, SPI_DC, SPI_RST);
    display->start();
    display->fillScreen(ST7735_BLACK);
    display->drawImage(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DATA);
    sleep_ms(2000);

    PWM* motor = new PWM(MOTOR_PWM_PIN);
    uint32_t top = motor->freq(1000);
    motor->duty(0);

    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];

    while (true) {
        process_uart_commands(motor, top);
        update_simulated_values();

        bool bt_connected = hc05_is_connected();
        display->fillScreen(ST7735_BLACK);

        snprintf(line1, sizeof(line1), "Temp: %.1f C", simulated_temp);
        snprintf(line2, sizeof(line2), "Hum:  %.1f %%", simulated_hum);
        snprintf(line3, sizeof(line3), "BT: %s", bt_connected ? "CONECT" : "DESC");
        snprintf(line4, sizeof(line4), "Bomba: %s", motor_on ? "ON" : "OFF");

        display->text(line1, 2, 3, FONT_8x8, ST7735_GREEN);
        display->text(line2, 2, 13, FONT_8x8, ST7735_CYAN);
        display->text(line3, 2, 23, FONT_8x8, bt_connected ? ST7735_GREEN : ST7735_RED);
        display->text(line4, 2, 33, FONT_8x8, motor_on ? ST7735_YELLOW : ST7735_WHITE);

        hc05_send_json(simulated_temp, simulated_hum, bt_connected, motor_on);

        sleep_ms(1000);
    }

    return 0;
}
