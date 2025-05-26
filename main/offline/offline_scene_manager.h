#ifndef OFFLINE_SCENE_MANAGER_H
#define OFFLINE_SCENE_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <array>
#include "offline_base.h"
#include <mutex>

#if CONFIG_LAN_XIAOHONGMEI
#include "mmap_generate_xiaohongmei.h"
#elif CONFIG_LAN_XIAOHUOGUO
#include "mmap_generate_xiaohuoguo.h"
#endif

class OfflineSceneManager
{
private:
    std::map<std::string, std::unique_ptr<OfflineBase>> registered_scenes_;
    std::array<std::string, 4> scene_names_ = {"fxq", "fns", "fql", "online"};
    size_t current_scene_index_ = 3;
    OfflineBase *current_player_;
#if CONFIG_LAN_XIAOHONGMEI || CONFIG_LAN_XIAOHUOGUO
    mmap_assets_handle_t asset_offline_audio;
#endif
    // 单例相关成员
    static OfflineSceneManager *instance_;
    static std::mutex mutex_;

    // 私有构造函数和析构函数
    OfflineSceneManager();
    ~OfflineSceneManager();

    // 注册音频文件
    void MountFs();
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

#endif // OFFLINE_SCENE_MANAGER_H