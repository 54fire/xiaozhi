#ifndef OFFLINE_H
#define OFFLINE_H

#include <array>
#include <string_view>
#include "assets/lang_config.h"

class OfflineAudioPlayer {
private:
    size_t current_index_;

    // 私有构造函数，确保单例模式
    OfflineAudioPlayer();

    // 禁止拷贝构造和赋值
    OfflineAudioPlayer(const OfflineAudioPlayer&) = delete;
    OfflineAudioPlayer& operator=(const OfflineAudioPlayer&) = delete;

public:
    // 获取单例实例
    static OfflineAudioPlayer& getInstance();

    // 获取下一个音频资源
    std::string_view getNextSound();
};

#endif // OFFLINE_H