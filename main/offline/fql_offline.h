#ifndef FQL_OFFLINE_PLAYER_H
#define FQL_OFFLINE_PLAYER_H

#include "offline_base.h"

class FqlOfflinePlayer : public OfflineBase {
private:
    const std::vector<std::string_view> FQL_SOUNDS;

public:
    FqlOfflinePlayer();
    const std::vector<std::string_view>& getSounds() const override;
    const std::string_view getHelloSound() const  override;
};

#endif // FQL_OFFLINE_PLAYER_H