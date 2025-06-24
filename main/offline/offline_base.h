#pragma once

#include <array>
#include <string_view>
#include "assets/lang_config.h"
#include <vector>


class OfflineBase {
protected:
    size_t current_index_ = 0;

    // 子类必须实现此函数来提供自己的音频资源
    virtual const std::vector<int>& getSounds() const = 0;

public:
    virtual ~OfflineBase() = default;

    // 获取下一个音频资源
    int getNextSound();
    // 获取上一个音频资源
    int getPrevSound();

    virtual const std::string_view getHelloSound() const = 0; 
    
    virtual const char *get_tag() const = 0;   

    // 重置播放索引
    virtual void reset();
};