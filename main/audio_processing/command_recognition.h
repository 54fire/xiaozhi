// #ifndef COMMAND_RECOGNITION_H
// #define COMMAND_RECOGNITION_H

// #include <vector>
// #include <string>
// #include <functional>

// class CommandRecognition
// {
// public:
//     CommandRecognition();
//     ~CommandRecognition();

//     void Initialize(int channels);
//     void Feed(const std::vector<int16_t> &data);
//     void Recognize();

//     void OnCommandDetected(std::function<void(const char *command)> callback);

// private:
//     esp_afe_sr_v1_data_t *afe_data_;
//     esp_mn_iface_t *multinet_;
//     model_iface_data_t *model_data_;
//     std::string multinet_model_name_;
//     std::function<void(const char *command)> command_detected_callback_;
// };

// #endif // COMMAND_RECOGNITION_H