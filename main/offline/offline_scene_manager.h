#pragma once

#include <string>
#include <map>
#include <memory>
#include <array>
#include "offline_base.h"
#include <mutex>
#include <vector>

#if CONFIG_LAN_XIAOHONGMEI
#include "mmap_generate_xiaohongmei.h"
#elif CONFIG_LAN_XIAOHUOGUO
#include "mmap_generate_xiaohuoguo.h"
#endif
#include "mmap_generate_xq.h"
#include "mmap_generate_ns.h"
#include "mmap_generate_ql.h"

class OfflineSceneManager
{
private:
    static OfflineSceneManager* instance_;
    static std::mutex mutex_;
    
    OfflineBase* current_player_;
    std::map<std::string, std::unique_ptr<OfflineBase>> registered_scenes_;
    std::vector<std::string> scene_names_ = {"fxq", "fns", "fql", "online"};
    size_t current_scene_index_ = 0;

    // 为每个场景添加独立的资源句柄
    mmap_assets_handle_t asset_fxq_audio;
    mmap_assets_handle_t asset_fns_audio;
    mmap_assets_handle_t asset_fql_audio;
    mmap_assets_handle_t asset_online_audio;

    // 当前活跃的资源句柄
    mmap_assets_handle_t* current_asset_handle_;

    // 私有构造函数和析构函数
    OfflineSceneManager();
    ~OfflineSceneManager();

    // 注册音频文件
    void MountFs();
    void MountSceneAssets(const std::string& scene_name);
    void UnmountSceneAssets(const std::string& scene_name);
    // 播放音频文件
    void Play(const std::string_view &sound);

public:
    // 禁止拷贝构造和赋值
    OfflineSceneManager(const OfflineSceneManager &) = delete;
    OfflineSceneManager &operator=(const OfflineSceneManager &) = delete;

    // 单例获取方法
    static OfflineSceneManager *getInstance();

    // 切换到下一个场景
    void switchToNextScene();

    // 获取当前活动的音频播放器
    OfflineBase *getCurrentPlayer();

    // 获取当前场景名称
    std::string getCurrentSceneName() const;

    // 获取下一个音频资源
    int getNextSound();
    // 播放下一个音频资源
    void playNextSound();
    // 播放上一个音频资源
    void playPrevSound();

    // 检查当前场景是否为在线场景
    bool isOnlineScene() const;

    // 重置当前播放器的索引
    void resetCurrentPlayer();
};