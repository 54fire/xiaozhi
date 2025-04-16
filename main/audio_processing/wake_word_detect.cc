#include "wake_word_detect.h"
#include "application.h"
#include "esp_mn_speech_commands.h"
#include "esp_mn_models.h"

#include <esp_log.h>
#include <model_path.h>
#include <string.h> 
#include <arpa/inet.h>
#include <sstream>
#include <stdio.h>
#include <ctype.h>  // 用于isspace函数

#define DETECTION_RUNNING_EVENT 1

static const char *TAG = "WakeWordDetect";

static std::string trim_st(const std::string& str) {
    // 去除前导空格
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) {
        return ""; // 字符串全是空格
    }

    // 去除尾部空格
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

WakeWordDetect::WakeWordDetect()
    : afe_data_(nullptr),
      wake_word_pcm_(),
      wake_word_opus_()
{

    event_group_ = xEventGroupCreate();
}

WakeWordDetect::~WakeWordDetect()
{
    if (afe_data_ != nullptr)
    {
        afe_iface_->destroy(afe_data_);
    }

    if (wake_word_encode_task_stack_ != nullptr)
    {
        heap_caps_free(wake_word_encode_task_stack_);
    }

    vEventGroupDelete(event_group_);
}

void WakeWordDetect::Initialize(AudioCodec* codec)
{
    codec_ = codec;
    channels_ = codec_->input_channels();
    reference_ = codec_->input_reference();
    int ref_num = reference_ ? 1 : 0;

    srmodel_list_t *models = esp_srmodel_init("model");
    for (int i = 0; i < models->num; i++) {
        ESP_LOGI(TAG, "Model %d: %s", i, models->model_name[i]);
        if (strstr(models->model_name[i], ESP_WN_PREFIX) != NULL) {
            wakenet_model_ = models->model_name[i];
            auto words = esp_srmodel_get_wake_words(models, wakenet_model_);
            // split by ";" to get all wake words
            std::stringstream ss(words);
            std::string word;
            while (std::getline(ss, word, ';')) {
                wake_words_.push_back(word);
            }
        }
        else if (strstr(models->model_name[i], ESP_MN_PREFIX) != NULL)
        {
            multinet_model_name_ = models->model_name[i];
        }
    }

    std::string input_format;
    for (int i = 0; i < channels_ - ref_num; i++) {
        input_format.push_back('M');
    }
    for (int i = 0; i < ref_num; i++) {
        input_format.push_back('R');
    }
    afe_config_t* afe_config = afe_config_init(input_format.c_str(), models, AFE_TYPE_SR, AFE_MODE_HIGH_PERF);
    afe_config->aec_init = reference_;
    afe_config->aec_mode = AEC_MODE_SR_HIGH_PERF;
    afe_config->afe_perferred_core = 1;
    afe_config->afe_perferred_priority = 1;
    afe_config->memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM;
    
    afe_iface_ = esp_afe_handle_from_config(afe_config);
    afe_data_ = afe_iface_->create_from_config(afe_config);

#if USE_COMMAND_WAKE
    if (!multinet_model_name_.empty())
    {
        multinet_ = esp_mn_handle_from_name(const_cast<char *>(multinet_model_name_.c_str()));
        model_data_ = multinet_->create(const_cast<char *>(multinet_model_name_.c_str()), 16000);
        wake_words_.clear();
        esp_mn_commands_clear();

        // command wake word
        if (CONFIG_COMMAND_WAKE_WORD != nullptr && strlen(CONFIG_COMMAND_WAKE_WORD) != 0) {
            std::stringstream ss(CONFIG_COMMAND_WAKE_WORD);
            std::string word;
            while (std::getline(ss, word, ';')) {
                wake_words_.push_back(trim_st(word));
            }
            for (size_t i = 0; i < wake_words_.size(); ++i) {
                ESP_LOGI(TAG, "i: %d data: %s", i, wake_words_[i].data());
                esp_mn_commands_add(i + 1, wake_words_[i].data());
            }        
            esp_mn_commands_update();
        }
        multinet_->print_active_speech_commands(model_data_);
    } 
    else {
        ESP_LOGE(TAG, "No valid MultiNet model found!");
    }
#endif

    xTaskCreate([](void *arg) {
        auto this_ = (WakeWordDetect*)arg;
        this_->AudioDetectionTask();
        vTaskDelete(NULL);
    }, "audio_detection", 4096, this, 3, nullptr);
}

void WakeWordDetect::OnWakeWordDetected(std::function<void(const std::string &wake_word)> callback)
{
    wake_word_detected_callback_ = callback;
}

void WakeWordDetect::StartDetection()
{
    ESP_LOGI(TAG, "StartDetection");
    xEventGroupSetBits(event_group_, DETECTION_RUNNING_EVENT);
}

void WakeWordDetect::StopDetection()
{
    xEventGroupClearBits(event_group_, DETECTION_RUNNING_EVENT);
    if (afe_data_ != nullptr) {
        afe_iface_->reset_buffer(afe_data_);
    }
}

bool WakeWordDetect::IsDetectionRunning()
{
    return xEventGroupGetBits(event_group_) & DETECTION_RUNNING_EVENT;
}

void WakeWordDetect::Feed(const std::vector<int16_t>& data) {
    if (afe_data_ == nullptr) {
        return;
    }
    afe_iface_->feed(afe_data_, data.data());
}

size_t WakeWordDetect::GetFeedSize() {
    if (afe_data_ == nullptr) {
        return 0;
    }
    return afe_iface_->get_feed_chunksize(afe_data_) * codec_->input_channels();
}

// 函数：去除字符串前后的空格
void WakeWordDetect::trim(char* str) {
    char* start = str;  // 指向字符串的起始位置
    char* end = str + strlen(str) - 1;  // 指向字符串的末尾位置

    // 去除前面的空格
    while (isspace(*start)) {
        start++;
    }

    // 去除后面的空格
    while (end > start && isspace(*end)) {
        end--;
    }

    // 在字符串末尾添加空字符，截断多余的空格
    *(end + 1) = '\0';

    // 如果去除空格后字符串变短了，需要将剩余部分前移
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

void WakeWordDetect::AudioDetectionTask() {
    auto fetch_size = afe_iface_->get_fetch_chunksize(afe_data_);
    auto feed_size = afe_iface_->get_feed_chunksize(afe_data_);
    ESP_LOGI(TAG, "Audio detection task started, feed size: %d fetch size: %d", feed_size, fetch_size);
    while (true) {
        xEventGroupWaitBits(event_group_, DETECTION_RUNNING_EVENT, pdFALSE, pdTRUE, portMAX_DELAY);

        auto res = afe_iface_->fetch_with_delay(afe_data_, portMAX_DELAY);
        if (res == nullptr || res->ret_value == ESP_FAIL) {
            continue;
        }

        // Store the wake word data for voice recognition, like who is speaking
        StoreWakeWordData((uint16_t*)res->data, res->data_size / sizeof(uint16_t));

#if !USE_COMMAND_WAKE
        if (res->wakeup_state == WAKENET_DETECTED) {
            StopDetection();
            last_detected_wake_word_ = wake_words_[res->wake_word_index - 1];

            if (wake_word_detected_callback_) {
                wake_word_detected_callback_(last_detected_wake_word_);
            }
        }
#else
        esp_mn_state_t mn_state = multinet_->detect(model_data_, res->data);
        if (mn_state == ESP_MN_STATE_DETECTED)
        {
            esp_mn_results_t *mn_result = multinet_->get_results(model_data_);
            for (int i = 0; i < mn_result->num; i++)
            {
                ESP_LOGI(TAG, "Detected command: '%s', Probability: %.2f,num:%d ", mn_result->string, mn_result->prob[i],mn_result->num);
                StopDetection();
#if CONFIG_COMMAND_WAKE_NAME
                last_detected_wake_word_ = CONFIG_COMMAND_WAKE_NAME;
#else
                last_detected_wake_word_ = mn_result->string;
#endif
                if (wake_word_detected_callback_)
                {
                    wake_word_detected_callback_(last_detected_wake_word_);
                }
            }
        }
#endif
    }
}

void WakeWordDetect::StoreWakeWordData(uint16_t *data, size_t samples)
{
    // store audio data to wake_word_pcm_
    wake_word_pcm_.emplace_back(std::vector<int16_t>(data, data + samples));
    // keep about 2 seconds of data, detect duration is 32ms (sample_rate == 16000, chunksize == 512)
    while (wake_word_pcm_.size() > 2000 / 32)
    {
        wake_word_pcm_.pop_front();
    }
}

void WakeWordDetect::EncodeWakeWordData()
{
    wake_word_opus_.clear();
    if (wake_word_encode_task_stack_ == nullptr)
    {
        wake_word_encode_task_stack_ = (StackType_t *)heap_caps_malloc(4096 * 8, MALLOC_CAP_SPIRAM);
    }
    wake_word_encode_task_ = xTaskCreateStatic([](void *arg) {
        auto this_ = (WakeWordDetect*)arg;
        {
            auto start_time = esp_timer_get_time();
            auto encoder = std::make_unique<OpusEncoderWrapper>(16000, 1, OPUS_FRAME_DURATION_MS);
            encoder->SetComplexity(0); // 0 is the fastest

            for (auto& pcm: this_->wake_word_pcm_) {
                encoder->Encode(std::move(pcm), [this_](std::vector<uint8_t>&& opus) {
                    std::lock_guard<std::mutex> lock(this_->wake_word_mutex_);
                    this_->wake_word_opus_.emplace_back(std::move(opus));
                    this_->wake_word_cv_.notify_all();
                });
            }
            this_->wake_word_pcm_.clear();

            auto end_time = esp_timer_get_time();
            ESP_LOGI(TAG, "Encode wake word opus %zu packets in %lld ms",
                this_->wake_word_opus_.size(), (end_time - start_time) / 1000);

            std::lock_guard<std::mutex> lock(this_->wake_word_mutex_);
            this_->wake_word_opus_.push_back(std::vector<uint8_t>());
            this_->wake_word_cv_.notify_all();
        }
        vTaskDelete(NULL); 
    }, "encode_detect_packets", 4096 * 8, this, 1, wake_word_encode_task_stack_, &wake_word_encode_task_buffer_);
}

bool WakeWordDetect::GetWakeWordOpus(std::vector<uint8_t> &opus)
{
    std::unique_lock<std::mutex> lock(wake_word_mutex_);
    wake_word_cv_.wait(lock, [this]()
                       { return !wake_word_opus_.empty(); });
    opus.swap(wake_word_opus_.front());
    wake_word_opus_.pop_front();
    return !opus.empty();
}
