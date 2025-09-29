#include "pwm_lib.h"

PWM::PWM(uint8_t gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(gpio);
    channel = pwm_gpio_to_channel(gpio);
    pin = gpio;
}

uint PWM::freq(uint32_t frequency)
{
    uint32_t clock = clock_get_hz(clk_sys);
    uint32_t divider16 = clock / frequency / 4096 + (clock % (frequency * 4096) != 0);
    if (divider16 / 16 == 0)
    {
        divider16 = 16;
    }
    uint32_t wrap = clock * 16 / divider16 / frequency - 1;
    pwm_set_clkdiv_int_frac(slice, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice, wrap);
    return wrap;
}

void PWM::duty(uint32_t value)
{
    pwm_set_chan_level(slice, channel, value);
    pwm_set_enabled(slice, true);
}