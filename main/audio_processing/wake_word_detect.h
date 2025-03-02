#ifndef WAKE_WORD_DETECT_H
#define WAKE_WORD_DETECT_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_afe_sr_models.h>
#include <esp_nsn_models.h>

#include <list>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "command_recognition.h"
#include "esp_mn_iface.h"

#define TIAN_XIAO_MEI "tian xiao mei"
#define TIAN_XIAO_MEI_CODE 1
#define TIAN_XIAO_HU "tian xiao hu"
#define TIAN_XIAO_HU_CODE 2
#define XIAO_YU_XIAO_YU "xiao yu xiao yu"
#define XIAO_YU_XIAO_YU_CODE 3
#define USE_COMMAND_WAKE true
#define WAKE_COMMAND XIAO_YU_XIAO_YU
#define WAKE_NAME "你好，小语"

class WakeWordDetect
{
public:
    WakeWordDetect();
    ~WakeWordDetect();

    void Initialize(int channels, bool reference);
    void Feed(const std::vector<int16_t> &data);
    void OnWakeWordDetected(std::function<void(const std::string &wake_word)> callback);
    void OnVadStateChange(std::function<void(bool speaking)> callback);
    void StartDetection();
    void StopDetection();
    bool IsDetectionRunning();
    void EncodeWakeWordData();
    bool GetWakeWordOpus(std::vector<uint8_t> &opus);
    const std::string &GetLastDetectedWakeWord() const { return last_detected_wake_word_; }

private:
    esp_afe_sr_data_t *afe_detection_data_ = nullptr;
    char *wakenet_model_ = NULL;
    std::string multinet_model_name_;
    std::vector<std::string> wake_words_;
    std::vector<int16_t> input_buffer_;
    EventGroupHandle_t event_group_;
    std::function<void(const std::string &wake_word)> wake_word_detected_callback_;
    std::function<void(bool speaking)> vad_state_change_callback_;
    std::function<void(const char *command)> command_detected_callback_;
    bool is_speaking_ = false;
    int channels_;
    bool reference_;
    std::string last_detected_wake_word_;

    TaskHandle_t wake_word_encode_task_ = nullptr;
    StaticTask_t wake_word_encode_task_buffer_;
    StackType_t *wake_word_encode_task_stack_ = nullptr;
    std::list<std::vector<int16_t>> wake_word_pcm_;
    std::list<std::vector<uint8_t>> wake_word_opus_;
    std::mutex wake_word_mutex_;
    std::condition_variable wake_word_cv_;

    esp_mn_iface_t *multinet_;
    model_iface_data_t *model_data_;
    void trim(char* str);
    void StoreWakeWordData(uint16_t *data, size_t size);
    void AudioDetectionTask();
};

#endif
