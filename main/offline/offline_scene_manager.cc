#include "offline_scene_manager.h"
#include "fns_offline.h"
#include "fql_offline.h"
#include "fxq_offline.h"
#include "online.h"
#include <iostream>
#include "application.h"
// 单例实例
OfflineSceneManager* OfflineSceneManager::instance_ = nullptr;
std::mutex OfflineSceneManager::mutex_;

OfflineSceneManager::OfflineSceneManager() : current_player_(nullptr)
{
    // 注册所有场景
    registered_scenes_["fxq"] = std::make_unique<FxqOfflinePlayer>();
    registered_scenes_["fns"] = std::make_unique<FnsOfflinePlayer>();
    registered_scenes_["fql"] = std::make_unique<FqlOfflinePlayer>();
    registered_scenes_["online"] = std::make_unique<OnlineOfflinePlayer>();

    // 初始切换到Fxq模式
    current_player_ = registered_scenes_["online"].get();
}

OfflineSceneManager::~OfflineSceneManager()
{
    current_player_ = nullptr;
}

// 单例获取方法
OfflineSceneManager* OfflineSceneManager::getInstance()
{
    if (instance_ == nullptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr)
        {
            instance_ = new OfflineSceneManager();
        }
    }
    return instance_;
}

void OfflineSceneManager::switchToNextScene() {
    current_scene_index_ = (current_scene_index_ + 1) % scene_names_.size();
    std::string next_scene_name = scene_names_[current_scene_index_];
    current_player_ = registered_scenes_[next_scene_name].get();
    std::cout << "Switched to scene: " << next_scene_name << std::endl;
     auto &app = Application::GetInstance();
     app.PlaySound(current_player_->getHelloSound());
}

OfflineBase *OfflineSceneManager::getCurrentPlayer()
{
    return current_player_;
}

std::string OfflineSceneManager::getCurrentSceneName() const
{
    return scene_names_[current_scene_index_];
}

void OfflineSceneManager::resetCurrentPlayer()
{
    if (current_player_)
    {
        current_player_->reset();
    }
}

std::string_view OfflineSceneManager::getNextSound()
{
    return current_player_->getNextSound();
}

void OfflineSceneManager::playNextSound()
{
    auto &app = Application::GetInstance();
     app.PlaySound(current_player_->getNextSound());
}

void OfflineSceneManager::playPrevSound()
{
    auto &app = Application::GetInstance();
     app.PlaySound(current_player_->getPrevSound());
}


bool OfflineSceneManager::isOnlineScene() const
{
    return scene_names_[current_scene_index_] == "online";
}