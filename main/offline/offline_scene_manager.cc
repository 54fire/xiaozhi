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

OfflineSceneManager::OfflineSceneManager() : current_player_(nullptr), current_asset_handle_(nullptr)
{
    // 注册文件分区
    MountFs();

    // 注册所有场景
    registered_scenes_["fxq"] = std::make_unique<FxqOfflinePlayer>();
    registered_scenes_["fns"] = std::make_unique<FnsOfflinePlayer>();
    registered_scenes_["fql"] = std::make_unique<FqlOfflinePlayer>();
    registered_scenes_["online"] = std::make_unique<OnlineOfflinePlayer>();

    // 初始切换到fxq模式
    current_player_ = registered_scenes_["fxq"].get();
    current_asset_handle_ = &asset_fxq_audio;
    MountSceneAssets("fxq");
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
#if CONFIG_LAN_XIAOHONGMEI || CONFIG_LAN_XIAOHUOGUO
    // 配置每个场景的资源分区
    const mmap_assets_config_t config_fxq_audio = {
        .partition_label = "fxq_audio",
        .max_files = MMAP_XQ_FILES,
        .checksum = MMAP_XQ_CHECKSUM,
        .flags = {
            .mmap_enable = true,
            .app_bin_check = true,
        },
    };

    const mmap_assets_config_t config_fns_audio = {
        .partition_label = "fns_audio",
        .max_files = MMAP_NS_FILES,
        .checksum = MMAP_NS_CHECKSUM,
        .flags = {
            .mmap_enable = true,
            .app_bin_check = true,
        },
    };

    const mmap_assets_config_t config_fql_audio = {
        .partition_label = "fql_audio",
        .max_files = MMAP_QL_FILES,
        .checksum = MMAP_QL_CHECKSUM,
        .flags = {
            .mmap_enable = true,
            .app_bin_check = true,
        },
    };

    // const mmap_assets_config_t config_online_audio = {
    //     .partition_label = "online_audio",
    //     .max_files = MMAP_XIAOHUOGUO_FILES,
    //     .checksum = MMAP_XIAOHUOGUO_CHECKSUM,
    //     .flags = {
    //         .mmap_enable = true,
    //         .app_bin_check = true,
    //     },
    // };

    // 初始化所有资源句柄
    mmap_assets_new(&config_fxq_audio, &asset_fxq_audio);
    mmap_assets_new(&config_fns_audio, &asset_fns_audio);
    mmap_assets_new(&config_fql_audio, &asset_fql_audio);
    // mmap_assets_new(&config_online_audio, &asset_online_audio);
#endif
}

void OfflineSceneManager::MountSceneAssets(const std::string& scene_name)
{
#if CONFIG_LAN_XIAOHONGMEI || CONFIG_LAN_XIAOHUOGUO
    mmap_assets_handle_t* target_handle = nullptr;
    
    if (scene_name == "fxq") {
        target_handle = &asset_fxq_audio;
    } else if (scene_name == "fns") {
        target_handle = &asset_fns_audio;
    } else if (scene_name == "fql") {
        target_handle = &asset_fql_audio;
    }
    //  else if (scene_name == "online") {
    //     target_handle = &asset_online_audio;
    // }

    if (target_handle) {
        ESP_LOGI(TAG, "Mounting assets for scene: %s", scene_name.c_str());
        ESP_LOGI(TAG, "Stored files: %d", mmap_assets_get_stored_files(*target_handle));
    }
#endif
}

void OfflineSceneManager::UnmountSceneAssets(const std::string& scene_name)
{
#if CONFIG_LAN_XIAOHONGMEI || CONFIG_LAN_XIAOHUOGUO
    // 目前不需要实际卸载，因为我们使用的是mmap，资源会一直保持在内存中
    ESP_LOGI(TAG, "Unmounting assets for scene: %s", scene_name.c_str());
#endif
}

void OfflineSceneManager::switchToNextScene() {
    // 卸载当前场景资源
    UnmountSceneAssets(scene_names_[current_scene_index_]);
    
    // 切换到下一个场景
    current_scene_index_ = (current_scene_index_ + 1) % scene_names_.size();
    std::string next_scene_name = scene_names_[current_scene_index_];
    
    // 挂载新场景资源
    MountSceneAssets(next_scene_name);
    
    // 更新当前播放器和资源句柄
    current_player_ = registered_scenes_[next_scene_name].get();
    if (next_scene_name == "fxq") {
        current_asset_handle_ = &asset_fxq_audio;
    } else if (next_scene_name == "fns") {
        current_asset_handle_ = &asset_fns_audio;
    } else if (next_scene_name == "fql") {
        current_asset_handle_ = &asset_fql_audio;
    } 
    // else if (next_scene_name == "online") {
    //     current_asset_handle_ = &asset_online_audio;
    // }
    
    std::cout << "Switched to scene: " << next_scene_name << std::endl;
    current_player_->reset();
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
    ESP_LOGI(TAG, "当前播放索引: %d", offline_audio_enum);
    const uint8_t* data = mmap_assets_get_mem(*current_asset_handle_, offline_audio_enum);
    size_t size = mmap_assets_get_size(*current_asset_handle_, offline_audio_enum);
    const std::string_view sound(reinterpret_cast<const char*>(data), size);
    Play(sound);
}

void OfflineSceneManager::playPrevSound()
{
    int offline_audio_enum = current_player_->getPrevSound();
    const uint8_t* data = mmap_assets_get_mem(*current_asset_handle_, offline_audio_enum);
    size_t size = mmap_assets_get_size(*current_asset_handle_, offline_audio_enum);
    const std::string_view sound(reinterpret_cast<const char*>(data), size);
    Play(sound);
}

void OfflineSceneManager::Play(const std::string_view &sound) {
    auto &app = Application::GetInstance();
    app.Narrate(sound);
}

bool OfflineSceneManager::isOnlineScene() const
{
    return getCurrentSceneName() == "online";
}