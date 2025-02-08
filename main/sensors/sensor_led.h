#ifndef SENSOR_LED_H
#define SENSOR_LED_H

#include "driver/ledc.h"
#include "esp_types.h"
#include "sensor_config.h"

// 定义LED PWM相关宏
#define LED_PWM_TIMER LEDC_TIMER_3
#define LED_PWM_CHANNEL LEDC_CHANNEL_3
#define LED_IO CONFIG_SENSOR_LED // 根据实际情况修改LED IO端口

class SensorLED
{
public:
    // 初始化LED的PWM设置
    static void init_led_pwm();

    // 设置LED的亮度
    static void set_led_brightness(uint32_t brightness);

    // 关闭LED
    static void turn_off_led();

    // 打开LED
    static void turn_on_led();
};

#endif // SENSOR_LED_H
