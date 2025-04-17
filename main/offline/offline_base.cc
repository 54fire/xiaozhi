#include "offline_base.h"
#include "esp_log.h"
int OfflineBase::getNextSound() {
    if (current_index_ >= getSounds().size()) {
        current_index_ = 0;
    }
    int sound = getSounds()[current_index_];
    ESP_LOGI(get_tag(), "current sound: %d", current_index_);
    current_index_++;
    return sound;
}

int OfflineBase::getPrevSound() {
    if (current_index_ == 0) {
        current_index_ = 0;
    }
    int sound = getSounds()[current_index_];
    ESP_LOGI(get_tag(), "current sound: %d", current_index_);
    if (current_index_ > 0) current_index_--;
    return sound;
}
void OfflineBase::reset() {
    current_index_ = 0;
}