#include "sensor_led.h"
#include "driver/ledc.h"
#include "esp_log.h"

// static const char *TAG = "Sensor_LED";

// 初始化LED的PWM设置
void SensorLED::init_led_pwm()
{
    if (LED_IO == -1)
    {
        return;
    }
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LED_PWM_TIMER,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = LED_IO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LED_PWM_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel);
}

// 设置LED的亮度
void SensorLED::set_led_brightness(uint32_t brightness)
{
    if (LED_IO == -1)
    {
        return;
    }
    uint32_t duty = brightness >> 2; // 简单处理：降低亮度范围
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);
}

// 关闭LED
void SensorLED::turn_off_led()
{
    if (LED_IO == -1)
    {
        return;
    }
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);
}

// 处理振动事件时触发LED变化
void SensorLED::turn_on_led()
{
    if (LED_IO == -1)
    {
        return;
    }
    uint32_t duty_3V3 = 1023; // LED亮度最大
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL, duty_3V3);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_CHANNEL);
}
