#ifndef BLUFI_H_
#include "esp_event.h"
#include "esp_log.h"

class Blufi
{
private:
    Blufi();
    ~Blufi();

    esp_err_t ret;
    EventGroupHandle_t event_group_;

    void InitWifi();

public:
    static Blufi& GetInstance() {
        static Blufi instance;
        return instance;
    }

    // 删除拷贝构造函数和赋值运算符
    Blufi(const Blufi&) = delete;
    Blufi& operator=(const Blufi&) = delete;

    void Start();
};

#endif // BLUFI_H_