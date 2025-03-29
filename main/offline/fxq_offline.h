#ifndef FXQ_OFFLINE_PLAYER_H
#define FXQ_OFFLINE_PLAYER_H

#include "offline_base.h"

class FxqOfflinePlayer : public OfflineBase {
private:
    const std::vector<std::string_view> FXQ_SOUNDS;

public:
    FxqOfflinePlayer();
    const std::vector<std::string_view>& getSounds() const override;
    const std::string_view getHelloSound() const  override;
    const char* get_tag() const  override;
};

#endif // FXQ_OFFLINE_PLAYER_H