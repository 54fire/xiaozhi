#include "fql_offline.h"

FqlOfflinePlayer::FqlOfflinePlayer()
    : FQL_SOUNDS({
                  Lang::Sounds::P3_QL1,
                  Lang::Sounds::P3_QL2,
                  Lang::Sounds::P3_QL3,
                  Lang::Sounds::P3_QL4,
                  Lang::Sounds::P3_QL5,
                  Lang::Sounds::P3_QL6,
                  Lang::Sounds::P3_QL7,
                  Lang::Sounds::P3_QL8,
                  Lang::Sounds::P3_QL9,
                  Lang::Sounds::P3_QL10,
                  Lang::Sounds::P3_QL11,
                  Lang::Sounds::P3_QL12,
                  Lang::Sounds::P3_QL13,
                  Lang::Sounds::P3_QL14,
                  Lang::Sounds::P3_QL15,
                  Lang::Sounds::P3_QL16,
                  Lang::Sounds::P3_QL17,
                  Lang::Sounds::P3_QL18,
                  Lang::Sounds::P3_QL19,
                  Lang::Sounds::P3_QL20,
                  Lang::Sounds::P3_QL21,
                  Lang::Sounds::P3_QL22,
                  Lang::Sounds::P3_QL23,
                  Lang::Sounds::P3_QL24,
                  Lang::Sounds::P3_QL25,
                  Lang::Sounds::P3_QL26,
                  Lang::Sounds::P3_QL27,
                  Lang::Sounds::P3_QL28,
                  Lang::Sounds::P3_QL29,
                  Lang::Sounds::P3_QL30,
                  Lang::Sounds::P3_QL31,
                  Lang::Sounds::P3_QL32,
                  Lang::Sounds::P3_QL33,
                  Lang::Sounds::P3_QL34,
                  Lang::Sounds::P3_QL35,
                  Lang::Sounds::P3_QL36,
                  Lang::Sounds::P3_QL37,
                  Lang::Sounds::P3_QL38,
                  Lang::Sounds::P3_QL39,
                  Lang::Sounds::P3_QL40,
                  Lang::Sounds::P3_QL41,
                  Lang::Sounds::P3_QL42,
                  Lang::Sounds::P3_QL43,
                  Lang::Sounds::P3_QL44,
                  Lang::Sounds::P3_QL45,
                  Lang::Sounds::P3_QL46,
                  Lang::Sounds::P3_QL47,
                  Lang::Sounds::P3_QL48,
                  Lang::Sounds::P3_QL49,
                  Lang::Sounds::P3_QL50,
                  Lang::Sounds::P3_QL51,
                  Lang::Sounds::P3_QL52,
                  Lang::Sounds::P3_QL53}) {}

const std::vector<std::string_view> &FqlOfflinePlayer::getSounds() const
{
    return FQL_SOUNDS;
}

const std::string_view  FqlOfflinePlayer::getHelloSound() const
{
    return Lang::Sounds::P3_FQL;
}

const char* FqlOfflinePlayer::get_tag() const 
{
    return "fql";
}