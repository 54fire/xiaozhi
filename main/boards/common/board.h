

#ifndef BOARD_H
#define BOARD_H

#include <http.h>
#include <web_socket.h>
#include <mqtt.h>
#include <udp.h>
#include <string>

#include "led/led.h"
#include "backlight.h"

// #define CONFIG_WEBSOCKET_URL "ws://192.168.10.20:8000/xiaozhi/v1/"
// #define CONFIG_WEBSOCKET_URL "ws://8.137.102.252:8788/xiaozhi/v1/"
// #define CONFIG_WEBSOCKET_URL "wss://api.tenclass.net/xiaozhi/v1/"
// #define CONFIG_WEBSOCKET_URL "ws://47.96.93.102:8788/xiaozhi/v1/"
// #define CONFIG_WEBSOCKET_URL "ws://ai.prod.zxzyn.com/xiaozhi/v1/"
#define CONFIG_WEBSOCKET_URL "ws://192.168.10.116:8000/xiaozhi/v1/"

void *create_board();
class AudioCodec;
class Display;
class Board
{
private:
    Board(const Board &) = delete;            // 禁用拷贝构造函数
    Board &operator=(const Board &) = delete; // 禁用赋值操作
    virtual std::string GetBoardJson() = 0;

protected:
    Board();
    std::string GenerateUuid();

    // 软件生成的设备唯一标识
    std::string uuid_;

public:
    static Board& GetInstance() {
        static Board* instance = static_cast<Board*>(create_board());
        return *instance;
    }

    virtual ~Board() = default;
    virtual std::string GetBoardType() = 0;
    virtual std::string GetUuid() { return uuid_; }
    virtual Backlight* GetBacklight() { return nullptr; }
    virtual Led* GetLed();
    virtual AudioCodec* GetAudioCodec() = 0;
    virtual Display* GetDisplay();
    virtual Http* CreateHttp() = 0;
    virtual WebSocket* CreateWebSocket() = 0;
    virtual Mqtt* CreateMqtt() = 0;
    virtual Udp* CreateUdp() = 0;
    virtual void StartNetwork() = 0;
    virtual void EndNetwork() = 0;
    virtual bool IsConnected() = 0;
    virtual const char* GetNetworkStateIcon() = 0;
    virtual bool GetBatteryLevel(int &level, bool& charging, bool& discharging);
    virtual std::string GetJson();
    virtual void SetPowerSaveMode(bool enabled) = 0;
};

#define DECLARE_BOARD(BOARD_CLASS_NAME) \
    void *create_board()                \
    {                                   \
        return new BOARD_CLASS_NAME();  \
    }

#endif // BOARD_H