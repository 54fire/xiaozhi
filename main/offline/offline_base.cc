#include "offline_base.h"

int OfflineBase::getNextSound() {
  if (current_index_ >= getSounds().size()) {
    current_index_ = 0;
  }
  return current_index_++;
}

int OfflineBase::getPrevSound() {
  if (current_index_ == 0) {
    current_index_ = getSounds().size() - 1;
  }
  return current_index_--;
}

void OfflineBase::reset() { current_index_ = 0; }