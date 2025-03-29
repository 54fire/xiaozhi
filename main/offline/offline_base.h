#ifndef OFFLINE_BASE_H
#define OFFLINE_BASE_H

#include <array>
#include <string_view>
#include "assets/lang_config.h"
#include <vector>


class OfflineBase {
protected:
    size_t current_index_ = 0;

    // 子类必须实现此函数来提供自己的音频资源
     virtual const std::vector<std::string_view>& getSounds() const = 0;

public:
    virtual ~OfflineBase() = default;

    // 获取下一个音频资源
    std::string_view getNextSound();

    std::string_view getPrevSound();

    virtual const std::string_view getHelloSound() const = 0; 
    
    virtual const char *get_tag() const = 0;   

    // 重置播放索引
    virtual void reset();
};

#endif // OFFLINE_BASE_H