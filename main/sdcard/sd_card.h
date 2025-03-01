#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdio.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#ifdef CONFIG_BOARD_TYPE_BOX_LLM_WIFI
#define BSP_SD_CLK GPIO_NUM_47
#define BSP_SD_CMD GPIO_NUM_48
#define BSP_SD_D0 GPIO_NUM_21
#elif CONFIG_BOARD_TYPE_HUG_BEAT_WIFI
#define BSP_SD_CLK GPIO_NUM_9
#define BSP_SD_CMD GPIO_NUM_8
#define BSP_SD_D0 GPIO_NUM_10

#elif CONFIG_BOARD_TYPE_HUG_BEAT_4G
#define BSP_SD_CLK GPIO_NUM_9
#define BSP_SD_CMD GPIO_NUM_8
#define BSP_SD_D0 GPIO_NUM_10

#elif CONFIG_BOARD_TYPE_PPYUE_1_0_WIFI
#define BSP_SD_CLK GPIO_NUM_47
#define BSP_SD_CMD GPIO_NUM_14
#define BSP_SD_D0 GPIO_NUM_21
#endif

class SDCard
{
public:
    static SDCard &getInstance()
    {
        static SDCard instance;
        return instance;
    }

    esp_err_t initialize();
    esp_err_t mountSDCard();
    esp_err_t unmountSDCard();
    esp_err_t writeToFile(const char *path, const char *data);
    esp_err_t readFromFile(const char *path, char *buffer, size_t bufferSize);

private:
    SDCard() = default;
    ~SDCard() = default;
    SDCard(const SDCard &) = delete;
    SDCard &operator=(const SDCard &) = delete;

    bool isInitialized = false;
    sdmmc_card_t *card = nullptr;
    const char *mountPoint = "/sdcard";
};

#endif // SD_CARD_H
