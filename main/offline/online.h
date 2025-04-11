#ifndef ONLINE_OFFLINE_PLAYER_H
#define ONLINE_OFFLINE_PLAYER_H

#include "offline_base.h"

class OnlineOfflinePlayer : public OfflineBase {
private:
    const std::vector<int> ONLINE_SOUNDS;

public:
    OnlineOfflinePlayer();
    const std::vector<int>& getSounds() const override;
    const std::string_view getHelloSound() const  override;
    const char* get_tag() const  override;
};

#endif // ONLINE_OFFLINE_PLAYER_H