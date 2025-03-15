#include "offline.h"

// 离线音频文件列表（p3格式）
// const std::vector<std::string> OFFLINE_AUDIO_FILES = {
//     "xq.p3",
//     "xq1.p3",
//     "xq2.p3",
//     "xq3.p3"
// };

// 音频资源数组
const std::array<std::string_view, 12> OFFLINE_SOUNDS = {
    Lang::Sounds::P3_XHM,
    Lang::Sounds::P3_XQ,
    Lang::Sounds::P3_XQ1,
    Lang::Sounds::P3_XQ2,
    Lang::Sounds::P3_XQ3,
    Lang::Sounds::P3_NS,
    Lang::Sounds::P3_NS1,
    Lang::Sounds::P3_NS2,
    Lang::Sounds::P3_NS3,
    Lang::Sounds::P3_NS4,
    Lang::Sounds::P3_NS5,
    Lang::Sounds::P3_NS6,

    
};

// 离线音频播放器类实现
OfflineAudioPlayer::OfflineAudioPlayer() : current_index_(0) {
}

std::string_view OfflineAudioPlayer::getNextSound() {
    if (current_index_ >= OFFLINE_SOUNDS.size()) {
        current_index_ = 0;
    }
    std::string_view sound= OFFLINE_SOUNDS[current_index_];
    current_index_++;
    return sound;
}

// 获取单例实例
OfflineAudioPlayer& OfflineAudioPlayer::getInstance() {
    static OfflineAudioPlayer instance;
    return instance;
}