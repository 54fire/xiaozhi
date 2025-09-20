#include "offline_base.h"

int OfflineBase::getNextSound() {
  static bool _init = false;
  if (!_init) {
    _init = true;
    return 0;
  }
  current_index_++;
  if (current_index_ >= getSounds().size()) {
    current_index_ = 0;
  } 
  return current_index_;
}

int OfflineBase::getPrevSound() {
  current_index_--;
  if (current_index_ < 0) {
    current_index_ = getSounds().size() - 1;
  } 
  return current_index_;
}

void OfflineBase::reset() { current_index_ = 0; }