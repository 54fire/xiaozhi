#include "offline_scene_manager.h"
#include "ai_offline.h"
#include "esp_spiffs.h"
#include "online.h"
#include <iostream>
#include <esp_log.h>
#include <esp_err.h>
#include "application.h"
#include "mmap_generate_ai.h"
#define TAG "OfflineSceneManager"


// 单例实例
OfflineSceneManager* OfflineSceneManager::instance_ = nullptr;
std::mutex OfflineSceneManager::mutex_;

OfflineSceneManager::OfflineSceneManager() : current_player_(nullptr), current_asset_handle_(nullptr)
{
    // 初始化资源句柄为nullptr
    asset_online_audio = nullptr;
    asset_ai_audio = nullptr;
    
    // 注册文件分区
    MountFs();

    // 注册所有场景
    registered_scenes_["ai"] = std::make_unique<AiOfflinePlayer>();
    // registered_scenes_["online"] = std::make_unique<OnlineOfflinePlayer>();

    // 初始切换到online模式
    current_player_ = registered_scenes_["ai"].get();
    current_asset_handle_ = &asset_online_audio;
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
    const mmap_assets_config_t config_ai_audio = {
        .partition_label = "ai_audio",
        .max_files = MMAP_AI_FILES,
        .checksum = MMAP_AI_CHECKSUM,
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
    esp_err_t ret = mmap_assets_new(&config_ai_audio, &asset_ai_audio);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize AI audio assets: %s", esp_err_to_name(ret));
        asset_ai_audio = nullptr;
    } else {
        ESP_LOGI(TAG, "AI audio assets initialized successfully");
    }
    
    // 暂时注释掉online音频初始化，因为配置未完成
    // mmap_assets_new(&config_online_audio, &asset_online_audio);
#endif
}

void OfflineSceneManager::MountSceneAssets(const std::string& scene_name)
{
#if CONFIG_LAN_XIAOHONGMEI || CONFIG_LAN_XIAOHUOGUO
    mmap_assets_handle_t* target_handle = nullptr;
    
    if (scene_name == "ai") {
        target_handle = &asset_ai_audio;
    } 
    //  else if (scene_name == "online") {
    //     target_handle = &asset_online_audio;
    // }

    if (target_handle && *target_handle != nullptr) {
        ESP_LOGI(TAG, "Mounting assets for scene: %s", scene_name.c_str());
        ESP_LOGI(TAG, "Stored files: %d", mmap_assets_get_stored_files(*target_handle));
    } else {
        ESP_LOGW(TAG, "Warning: Asset handle for scene %s is not properly initialized", scene_name.c_str());
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
    if (next_scene_name == "ai") {
        current_asset_handle_ = &asset_ai_audio;
    } else if (next_scene_name == "online") {
        current_asset_handle_ = &asset_online_audio;
    }
    
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

int OfflineSceneManager::getPrevSound()
{
    return current_player_->getPrevSound();
}

void OfflineSceneManager::playNextSound()
{
    if (!current_player_ || !current_asset_handle_ || *current_asset_handle_ == nullptr) {
        ESP_LOGW(TAG, "Warning: Player or asset handle not initialized");
        return;
    }
    
    int offline_audio_enum = current_player_->getNextSound();
    ESP_LOGI(TAG, "当前播放索引: %d", offline_audio_enum);
    const uint8_t* data = mmap_assets_get_mem(*current_asset_handle_, offline_audio_enum);
    size_t size = mmap_assets_get_size(*current_asset_handle_, offline_audio_enum);
    const std::string_view sound(reinterpret_cast<const char*>(data), size);
    Play(sound);
}

void OfflineSceneManager::playPrevSound()
{
    if (!current_player_ || !current_asset_handle_ || *current_asset_handle_ == nullptr) {
        ESP_LOGW(TAG, "Warning: Player or asset handle not initialized");
        return;
    }
    
    int offline_audio_enum = current_player_->getPrevSound();
    ESP_LOGI(TAG, "当前播放索引: %d", offline_audio_enum);
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