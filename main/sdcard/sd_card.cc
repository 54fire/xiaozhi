

#include "sd_card.h"
#include "esp_log.h"

static const char *TAG = "SDCard";

esp_err_t SDCard::initialize()
{
    if (isInitialized)
    {
        ESP_LOGW(TAG, "SDCard is already initialized.");
        return ESP_OK;
    }

    esp_err_t ret = mountSDCard();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SDCard.");
        return ret;
    }

    isInitialized = true;
    ESP_LOGI(TAG, "SDCard initialized successfully.");
    return ESP_OK;
}

esp_err_t SDCard::mountSDCard()
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.clk = BSP_SD_CLK;
    slot_config.cmd = BSP_SD_CMD;
    slot_config.d0 = BSP_SD_D0;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem...");
    auto ret = esp_vfs_fat_sdmmc_mount(mountPoint, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount filesystem. Error: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Filesystem mounted.");
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

esp_err_t SDCard::unmountSDCard()
{
    if (!isInitialized)
    {
        ESP_LOGW(TAG, "SDCard is not initialized.");
        return ESP_OK;
    }

    esp_err_t ret = esp_vfs_fat_sdcard_unmount(mountPoint, card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to unmount SDCard. Error: %s", esp_err_to_name(ret));
        return ret;
    }

    isInitialized = false;
    ESP_LOGI(TAG, "SDCard unmounted.");
    return ESP_OK;
}

esp_err_t SDCard::writeToFile(const char *path, const char *data)
{
    if (!isInitialized)
    {
        ESP_LOGE(TAG, "SDCard is not initialized.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing.");
        return ESP_FAIL;
    }

    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "File written successfully.");
    return ESP_OK;
}

esp_err_t SDCard::readFromFile(const char *path, char *buffer, size_t bufferSize)
{
    if (!isInitialized)
    {
        ESP_LOGE(TAG, "SDCard is not initialized.");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading.");
        return ESP_FAIL;
    }

    if (fgets(buffer, bufferSize, f) == NULL)
    {
        ESP_LOGE(TAG, "Failed to read from file.");
        fclose(f);
        return ESP_FAIL;
    }

    fclose(f);
    ESP_LOGI(TAG, "File read successfully.");
    return ESP_OK;
}