// touch.h
#ifndef TOUCH_H
#define TOUCH_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "sensor_led.h"
#include "sensor_config.h"

#define HEAD_1_TOUCH CONFIG_TOUCH1
#define HEAD_2_TOUCH CONFIG_TOUCH2
typedef void (*TouchCallback)(uint8_t touch_num);

class TouchPadManager
{
public:
    TouchPadManager();
    void init();
    void createTask();
    void setCallback(TouchCallback callback);

private:
    touch_pad_t pinTouch1;
    touch_pad_t pinTouch2;
    static TaskHandle_t taskHandle;
    static TouchCallback touchCallback;

    static void ISR_touch_cb(void *arg);
    static void touchReadTask(void *arg);
};

#endif // TOUCH_H