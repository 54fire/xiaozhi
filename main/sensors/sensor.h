

#ifndef SENSOR_H
#define SENSOR_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "soc/soc.h"
#include "soc/soc_caps.h"
#include "driver/touch_pad.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "touch.h"
#include "sensor_config.h"
#include "sensor_led.h"

// 定义传感器事件类型
typedef enum
{
    HUG1 = 0,
    HUG2,
    HEAD1,
    HEAD2,
    LEFT_HAND,
    RIGHT_HAND
} SensorEventType;

// 定义传感器消息结构体
typedef struct
{
    SensorEventType eventType;
    int value;
} SensorMessage;

// 定义任务堆栈大小
#define PRESSURE_TASK_STACK_SIZE (4096)
#define VIBRATION_TASK_STACK_SIZE (3069)
#define TOUCH_TASK_STACK_SIZE (2048)

// 定义震动传感器IO端口
#define LEFT_HAND_IO CONFIG_LEFT_HAND   // LEFT_HAND IO端口
#define RIGHT_HAND_IO CONFIG_RIGHT_HAND // RIGHT_HAND IO端口

// 定义ADC相关宏
#define HUG1_CHANNEL CONFIG_HUG1_CHANNEL
#define HUG2_CHANNEL CONFIG_HUG2_CHANNEL
#define HEAD1_CHANNEL CONFIG_HEAD1_CHANNEL
#define HEAD2_CHANNEL CONFIG_HEAD2_CHANNEL
#define TOUCH_THRESHOLD (1000)
#define MAX_VALUE 4095
#define THRED_HOLD 2500
#define DEFAULT_VREF 1100 // 使用ESP32-S3的默认参考电压
#define NO_OF_SAMPLES 64  // ADC读取样本数量

class Sensor
{
public:
    static void Sensor_Init();
    static bool Consume_Queue(SensorMessage *msg); // 用于消费

private:
    static adc_oneshot_unit_handle_t adc1_handle; // 存储 ADC 单元句柄
    static adc_cali_handle_t adc1_cali_handles[10];
    static QueueHandle_t sensorQueue;
    static const char *TAG;
    static StaticTask_t xPressureTaskBuffer;
    static StaticTask_t xVibrationTaskBuffer;
    static StackType_t xPressureTaskStack[PRESSURE_TASK_STACK_SIZE];
    static StackType_t xVibrationTaskStack[VIBRATION_TASK_STACK_SIZE];
    static int64_t last_trigger_time;
    static TouchPadManager touchManager;
    static int normalize(int input_value, int min_output = 1, int max_output = 10);

    static int read_adc(int channel_index);
    static int read_adc_channel(adc_channel_t channel);
    static void handle_vibration_event();
    static void send_event_msg(SensorEventType type, int value);
    static void PressureSensor_Task(void *pvParameters);
    static void VibrationSensor_Task(void *pvParameters);
    static void init_vibration_sensor();
    static void init_adc();
    static void init_adc_channel(adc_channel_t channel);
    static void init_touch_pad();
    static void touchCallback(uint8_t pad_num);
    static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
};

#endif // SENSOR_H