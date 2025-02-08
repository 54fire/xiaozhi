// touch.cc
#include "touch.h"
#include <stdio.h>

TaskHandle_t TouchPadManager::taskHandle = NULL;
TouchCallback TouchPadManager::touchCallback = NULL;

TouchPadManager::TouchPadManager() : pinTouch1(HEAD_1_TOUCH), pinTouch2(HEAD_2_TOUCH) {}

void TouchPadManager::init()
{
    touch_pad_init();
    touch_pad_config(pinTouch1);
    touch_pad_config(pinTouch2);

    touch_pad_denoise_t denoisecnf = {
        .grade = TOUCH_PAD_DENOISE_BIT4,
        .cap_level = TOUCH_PAD_DENOISE_CAP_L2,
    };
    touch_pad_denoise_set_config(&denoisecnf);
    touch_pad_denoise_enable();

    touch_filter_config_t filter_info = {
        .mode = TOUCH_PAD_FILTER_IIR_8,
        .debounce_cnt = 1,
        .noise_thr = 0,
        .jitter_step = 4,
        .smh_lvl = TOUCH_PAD_SMOOTH_IIR_2,
    };
    touch_pad_filter_set_config(&filter_info);
    touch_pad_filter_enable();

    touch_pad_isr_register(ISR_touch_cb, NULL, TOUCH_PAD_INTR_MASK_ACTIVE);
    touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE);

    touch_pad_set_idle_channel_connect(TOUCH_PAD_CONN_GND);
    touch_pad_set_channel_mask(1UL << pinTouch1);
    touch_pad_set_channel_mask(1UL << pinTouch2);

    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
    vTaskDelay(40 / portTICK_PERIOD_MS);

    uint32_t denoisedata;
    touch_pad_denoise_read_data(&denoisedata);
    printf("denosise value is:%ld\n", denoisedata);

    uint32_t bchmk;
    touch_pad_read_benchmark(pinTouch1, &bchmk);
    printf("benchmark2 is:%ld\n", bchmk);
    touch_pad_set_thresh(pinTouch1, bchmk * 0.2);

    touch_pad_read_benchmark(pinTouch2, &bchmk);
    printf("benchmark11 is:%ld\n", bchmk);
    touch_pad_set_thresh(pinTouch2, bchmk * 0.2);
}

void TouchPadManager::createTask()
{
    xTaskCreate(touchReadTask, "touchReadTask", 1024 * 2, NULL, 2, &taskHandle);
}

void TouchPadManager::setCallback(TouchCallback callback)
{
    touchCallback = callback;
}

void TouchPadManager::ISR_touch_cb(void *arg)
{
    int pad_num = touch_pad_get_current_meas_channel();
    if (taskHandle != NULL)
    {
        xTaskNotifyFromISR(taskHandle, pad_num, eSetValueWithOverwrite, NULL);
    }
}

void TouchPadManager::touchReadTask(void *arg)
{
    uint32_t pad_num;
    uint8_t touch_num;
    while (1)
    {
        if (0 != xTaskNotifyWait(0xffffffff, 0, &pad_num, portMAX_DELAY))
        {
            touch_num = pad_num == HEAD_1_TOUCH ? 1 : 2;
            if (touchCallback)
            {
                touchCallback(touch_num);
            }
            SensorLED::turn_on_led();
            printf("pad_num is:%ld,touch is %d\n", pad_num, touch_num);
        }
        else
        {
            SensorLED::turn_off_led();
        }
    }
}