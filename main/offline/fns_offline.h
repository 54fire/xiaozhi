#ifndef FNS_OFFLINE_PLAYER_H
#define FNS_OFFLINE_PLAYER_H

#include "offline_base.h"

class FnsOfflinePlayer : public OfflineBase {
private:
    const std::vector<int> FNS_SOUNDS;

public:
    FnsOfflinePlayer();
    const std::vector<int>& getSounds() const override;
    const std::string_view getHelloSound() const  override;
    const char* get_tag() const  override;
};

#endif // FNS_OFFLINE_PLAYER_H