#include "online.h"

OnlineOfflinePlayer::OnlineOfflinePlayer()
    : ONLINE_SOUNDS({}) {} // 提供一个空的音频资源数组

const std::vector<std::string_view>& OnlineOfflinePlayer::getSounds() const {
    return ONLINE_SOUNDS;
}

const std::string_view  OnlineOfflinePlayer::getHelloSound() const
{
    return Lang::Sounds::P3_WIFICONFIG;
}