#include "offline_base.h"

std::string_view OfflineBase::getNextSound() {
    if (current_index_ >= getSounds().size()) {
        current_index_ = 0;
    }
    std::string_view sound = getSounds()[current_index_];
    current_index_++;
    return sound;
}

std::string_view OfflineBase::getPrevSound() {
    if (current_index_ <= 0) {
        current_index_ = 0;
    }
    std::string_view sound = getSounds()[current_index_];
    current_index_--;
    return sound;
}
void OfflineBase::reset() {
    current_index_ = 0;
}