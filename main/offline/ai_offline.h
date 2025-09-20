#ifndef AI_OFFLINE_H
#define AI_OFFLINE_H

#include "offline_base.h"

class AiOfflinePlayer : public OfflineBase {
private:
    const std::vector<int> AI_SOUNDS;

public:
    AiOfflinePlayer();
    const std::vector<int>& getSounds() const override;
    const std::string_view getHelloSound() const override;
    const char* get_tag() const override;
};
#endif