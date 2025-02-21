// #include "command_recognition.h"
// #include "esp_log.h"
// #include "esp_mn_iface.h"
// #include "esp_mn_models.h"
// #include "model_path.h"

// static const char *TAG = "CommandRecognition";

// CommandRecognition::CommandRecognition()
//     : afe_data_(nullptr),
//       multinet_(nullptr),
//       model_data_(nullptr) {}

// CommandRecognition::~CommandRecognition()
// {
//     if (afe_data_)
//     {
//         esp_afe_sr_v1.destroy(afe_data_);
//         afe_data_ = nullptr;
//     }

//     if (model_data_)
//     {
//         multinet_->destroy(model_data_);
//         model_data_ = nullptr;
//     }
// }

// void CommandRecognition::Initialize(int channels)
// {
//     srmodel_list_t *models = esp_srmodel_init("model");
//     for (int i = 0; i < models->num; i++)
//     {
//         ESP_LOGI(TAG, "Model %d: %s", i, models->model_name[i]);
//         if (strstr(models->model_name[i], ESP_MN_PREFIX) != nullptr)
//         {
//             multinet_model_name_ = models->model_name[i];
//             break;
//         }
//     }

//     if (multinet_model_name_.empty())
//     {
//         ESP_LOGE(TAG, "No valid multinet model found!");
//         return;
//     }

//     afe_config_t afe_config = {
//         .aec_init = false,
//         .se_init = false,
//         .vad_init = true,
//         .wakenet_init = false,
//         .voice_communication_init = false,
//         .voice_communication_agc_init = false,
//         .vad_mode = VAD_MODE_3,
//         .afe_mode = SR_MODE_HIGH_PERF,
//         .memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM,
//         .pcm_config = {
//             .total_ch_num = channels,
//             .mic_num = channels,
//             .ref_num = 0,
//             .sample_rate = 16000}};

//     afe_data_ = esp_afe_sr_v1.create_from_config(&afe_config);

//     multinet_ = esp_mn_handle_from_name(multinet_model_name_.c_str());
//     model_data_ = multinet_->create(multinet_model_name_.c_str(), 16000);

//     // Add custom commands here
//     esp_mn_commands_clear();
//     esp_mn_commands_add(1, "da kai dian deng");
//     esp_mn_commands_add(2, "guan bi dian deng");
//     esp_mn_commands_update();

//     multinet_->print_active_speech_commands(model_data_);
// }

// void CommandRecognition::Feed(const std::vector<int16_t> &data)
// {
//     if (!afe_data_ || !model_data_)
//     {
//         ESP_LOGE(TAG, "AFE or Model data not initialized!");
//         return;
//     }

//     int feed_size = esp_afe_sr_v1.get_feed_chunksize(afe_data_);
//     while (data.size() >= feed_size)
//     {
//         esp_afe_sr_v1.feed(afe_data_, data.data());
//         data.erase(data.begin(), data.begin() + feed_size);
//     }
// }

// void CommandRecognition::Recognize()
// {
//     if (!afe_data_ || !model_data_)
//     {
//         ESP_LOGE(TAG, "AFE or Model data not initialized!");
//         return;
//     }

//     int fetch_size = esp_afe_sr_v1.get_fetch_chunksize(afe_data_);
//     auto res = esp_afe_sr_v1.fetch(afe_data_);
//     if (!res || res->ret_value != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Fetch error: %d", res->ret_value);
//         return;
//     }

//     esp_mn_state_t state = multinet_->detect(model_data_, res->data);
//     if (state == ESP_MN_STATE_DETECTED)
//     {
//         esp_mn_results_t *results = multinet_->get_results(model_data_);
//         for (int i = 0; i < results->num; i++)
//         {
//             ESP_LOGI(TAG, "Detected command: %s, Probability: %.2f",
//                      results->string, results->prob[i]);
//             // Call callback if registered
//             if (command_detected_callback_)
//             {
//                 command_detected_callback_(results->string);
//             }
//         }
//     }
// }

// void CommandRecognition::OnCommandDetected(std::function<void(const char *command)> callback)
// {
//     command_detected_callback_ = callback;
// }