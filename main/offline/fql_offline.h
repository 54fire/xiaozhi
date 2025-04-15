#ifndef FQL_OFFLINE_PLAYER_H
#define FQL_OFFLINE_PLAYER_H

#include "offline_base.h"

class FqlOfflinePlayer : public OfflineBase {
private:
    const std::vector<int> FQL_SOUNDS;

public:
    FqlOfflinePlayer();
    const std::vector<int>& getSounds() const override;
    const std::string_view getHelloSound() const  override;
    const char* get_tag() const  override;
};

#endif // FQL_OFFLINE_PLAYER_H