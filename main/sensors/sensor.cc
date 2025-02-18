

#include "sensor.h"

// 在这里定义所有的static成员变量
QueueHandle_t Sensor::sensorQueue = nullptr;
const char *Sensor::TAG = "Sensor";
StaticTask_t Sensor::xPressureTaskBuffer;
StaticTask_t Sensor::xVibrationTaskBuffer;
StackType_t Sensor::xPressureTaskStack[PRESSURE_TASK_STACK_SIZE];
StackType_t Sensor::xVibrationTaskStack[VIBRATION_TASK_STACK_SIZE];
int64_t Sensor::last_trigger_time = 0;
adc_oneshot_unit_handle_t Sensor::adc1_handle = NULL;
adc_cali_handle_t Sensor::adc1_cali_handles[10];
TouchPadManager Sensor::touchManager;

// 在这里定义所有的static成员函数
int Sensor::normalize(int input_value, int min_output, int max_output)
{
    return static_cast<int>((input_value - THRED_HOLD) * (max_output - min_output) / (MAX_VALUE - THRED_HOLD) + min_output);
}

bool Sensor::Consume_Queue(SensorMessage *msg)
{
    if (sensorQueue != NULL && msg != NULL)
    {
        if (xQueueReceive(sensorQueue, msg, portMAX_DELAY) == pdPASS)
        {

            // 消息处理成功
            return true;
        }
    }
    return false;
}

void Sensor::send_event_msg(SensorEventType type, int value)
{
    SensorMessage msg;
    msg.eventType = type;
    msg.value = value;
    if (xQueueSend(sensorQueue, &msg, portMAX_DELAY) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to send sensor msg");
    }
}

void Sensor::PressureSensor_Task(void *pvParameters)
{
    int pressurePins[4] = {-1, -1, -1, -1};
    pressurePins[0] = HUG1_CHANNEL;
    pressurePins[1] = HUG2_CHANNEL;
    pressurePins[2] = HEAD1_CHANNEL;
    pressurePins[3] = HEAD2_CHANNEL;
    SensorEventType pressureEvents[4] = {HUG1, HUG2, HEAD1, HEAD2};
    bool is_event_triggered = false;
    int64_t current_time;
    while (true)
    {
        is_event_triggered = false;
        for (int i = 0; i < 4; i++)
        {
            if (pressurePins[i] != -1)
            {
                int adc_value = MAX_VALUE - read_adc(i);
                if (adc_value > THRED_HOLD)
                {
                    ESP_LOGI(TAG, "Pressure Sensor %d ADC value: %d", i + 1, adc_value);
                    is_event_triggered = true;
                    SensorLED::set_led_brightness(adc_value);
                    current_time = esp_timer_get_time();
                    if ((current_time - last_trigger_time) / 1000 >= 500)
                    {
                        send_event_msg(pressureEvents[i], normalize(adc_value));
                        last_trigger_time = current_time;
                    }
                }
            }
        }
        if (!is_event_triggered)
        {
            SensorLED::turn_off_led();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool Sensor::example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}
void Sensor::init_adc()
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    if (HUG1_CHANNEL != -1)
    {

        init_adc_channel(HUG1_CHANNEL);
    }
    if (HUG2_CHANNEL != -1)
    {

        init_adc_channel(HUG2_CHANNEL);
    }
    if (HEAD1_CHANNEL != -1)
    {

        init_adc_channel(HEAD1_CHANNEL);
    }
    if (HEAD2_CHANNEL != -1)
    {

        init_adc_channel(HEAD2_CHANNEL);
    }
}

void Sensor::init_adc_channel(adc_channel_t channel)
{

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,         // 如果atten在bitwidth之前，则先初始化atten
        .bitwidth = ADC_BITWIDTH_DEFAULT, // 然后是bitwidth
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));
    // 校准 ADC
    bool do_calibration = example_adc_calibration_init(ADC_UNIT_1, channel, ADC_ATTEN_DB_12, &adc1_cali_handles[channel]);
    // 存储校准句柄
    adc1_cali_handles[channel] = do_calibration ? adc1_cali_handles[channel] : NULL;
}
int Sensor::read_adc(int channel_index)
{
    if (channel_index == 0)
    {
        return read_adc_channel(HUG1_CHANNEL);
    }
    else if (channel_index == 1)
    {
        return read_adc_channel(HUG2_CHANNEL);
    }
    else if (channel_index == 2)
    {
        return read_adc_channel(HEAD1_CHANNEL);
    }
    else if (channel_index == 3)
    {
        return read_adc_channel(HEAD2_CHANNEL);
    }

    return MAX_VALUE;
}

int Sensor::read_adc_channel(adc_channel_t channel)
{

    if (Sensor::adc1_handle && Sensor::adc1_cali_handles[channel])
    {
        int adc_raw_value;
        ESP_ERROR_CHECK(adc_oneshot_read(Sensor::adc1_handle, channel, &adc_raw_value));
        // ESP_LOGI(TAG, "ADC1 Channel[7] Raw Data: %d", adc_raw_value);

        int voltage_value;
        if (Sensor::adc1_cali_handles[channel])
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(Sensor::adc1_cali_handles[channel], adc_raw_value, &voltage_value));
            // ESP_LOGI(TAG, "ADC1 Channel[7] Cali Voltage: %d mV", voltage_value);
            //  将ADC值用于其他目的，例如设置LED亮度
            return voltage_value;
        }
    }
    return MAX_VALUE;
}

void Sensor::VibrationSensor_Task(void *pvParameters)
{
    int vibrationPins[2] = {LEFT_HAND_IO, RIGHT_HAND_IO};
    SensorEventType vibrationEvents[2] = {LEFT_HAND, RIGHT_HAND};
    bool is_event_triggered = false;
    int64_t current_time;
    while (true)
    {
        is_event_triggered = false;
        for (int i = 0; i < 2; i++)
        {
            if (vibrationPins[i] != -1)
            {
                int vibration_value = gpio_get_level((gpio_num_t)vibrationPins[i]);
                if (vibration_value != 1)
                {
                    SensorLED::turn_on_led();
                    is_event_triggered = true;
                    current_time = esp_timer_get_time();
                    if ((current_time - last_trigger_time) / 1000 >= 500)
                    {
                        ESP_LOGI(TAG, "Vibration Sensor %d value: %d", i + 1, vibration_value);
                        send_event_msg(vibrationEvents[i], vibration_value);
                        last_trigger_time = current_time;
                    }
                }
            }
        }
        if (!is_event_triggered)
        {
            SensorLED::turn_off_led();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void Sensor::touchCallback(uint8_t touch_num)
{
    if (touch_num == 1)
    {
        send_event_msg(HEAD1, 1);
    }
    else
    {
        send_event_msg(HEAD2, 1);
    }

    printf("Pad %d activated\n", touch_num);
}

void Sensor::init_touch_pad()
{
    if (!ENABLE_SENSOR_TOUCH)
    {
        return;
    }
    touchManager.init();
    touchManager.createTask();
    touchManager.setCallback(touchCallback);
    ESP_LOGI(TAG, "TouchPad task created");
}
void Sensor::init_vibration_sensor()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    int vibrationPins[2] = {LEFT_HAND_IO, RIGHT_HAND_IO};
    for (int i = 0; i < 2; i++)
    {
        if (vibrationPins[i] != -1)
        {
            io_conf.pin_bit_mask = (1ULL << vibrationPins[i]);
            gpio_config(&io_conf);
        }
    }
}
void Sensor::Sensor_Init()
{
    if (!ENABLE_SENSOR)
    {
        return;
    }
    sensorQueue = xQueueCreate(2, sizeof(SensorMessage));
    if (sensorQueue == NULL)
    {
        ESP_LOGE(TAG, "sensorQueue init failed!!");
    }

    init_vibration_sensor();
    SensorLED::init_led_pwm();
    init_adc(); // 初始化ADC
    init_touch_pad();
    xTaskCreateStatic(PressureSensor_Task, "PressureTask", PRESSURE_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, xPressureTaskStack, &xPressureTaskBuffer);
    ESP_LOGI(TAG, "Pressure task created");
    xTaskCreateStatic(VibrationSensor_Task, "VibrationTask", VIBRATION_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, xVibrationTaskStack, &xVibrationTaskBuffer);
    ESP_LOGI(TAG, "Vibration task created");
}
