#include "offline_scene_manager.h"
#include "fns_offline.h"
#include "fql_offline.h"
#include "fxq_offline.h"
#include "online.h"
#include <iostream>
#include <esp_log.h>
#include "application.h"

#define TAG "OfflineSceneManager"

// 单例实例
OfflineSceneManager* OfflineSceneManager::instance_ = nullptr;
std::mutex OfflineSceneManager::mutex_;

OfflineSceneManager::OfflineSceneManager() : current_player_(nullptr)
{
    // 注册文件分区
    MountFs();

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

void OfflineSceneManager::MountFs()
{
    const mmap_assets_config_t config_offinline_audio = {
        .partition_label = "offline_audio",
        .max_files = MMAP_OFFLINE_AUDIO_FILES,
        .checksum = MMAP_OFFLINE_AUDIO_CHECKSUM,
        .flags = {
            .mmap_enable = true,
            .app_bin_check = true,
        },
    };

    mmap_assets_new(&config_offinline_audio, &asset_offline_audio);
    ESP_LOGI(TAG, "[%s]stored_files:%d", config_offinline_audio.partition_label, mmap_assets_get_stored_files(asset_offline_audio));
}

void OfflineSceneManager::switchToNextScene() {
    current_scene_index_ = (current_scene_index_ + 1) % scene_names_.size();
    std::string next_scene_name = scene_names_[current_scene_index_];
    current_player_ = registered_scenes_[next_scene_name].get();
    std::cout << "Switched to scene: " << next_scene_name << std::endl;
    auto &app = Application::GetInstance();
    app.StopPlaybackAndReset();
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

int OfflineSceneManager::getNextSound()
{
    return current_player_->getNextSound();
}

void OfflineSceneManager::playNextSound()
{
    int offline_audio_enum = current_player_->getNextSound();
    const uint8_t* data = mmap_assets_get_mem(asset_offline_audio, offline_audio_enum);
    size_t size = mmap_assets_get_size(asset_offline_audio, MMAP_OFFLINE_AUDIO_NS1_P3);
    const std::string_view sound(reinterpret_cast<const char*>(data), size);
    Play(sound);
}

void OfflineSceneManager::playPrevSound()
{
    int offline_audio_enum = current_player_->getPrevSound();
    const uint8_t* data = mmap_assets_get_mem(asset_offline_audio, offline_audio_enum);
    size_t size = mmap_assets_get_size(asset_offline_audio, MMAP_OFFLINE_AUDIO_NS1_P3);
    const std::string_view sound(reinterpret_cast<const char*>(data), size);
    Play(sound);
}

void OfflineSceneManager::Play(const std::string_view &sound) {
    auto &app = Application::GetInstance();
    app.PlaySound(sound);
}

bool OfflineSceneManager::isOnlineScene() const
{
    return getCurrentSceneName() == "online";
}