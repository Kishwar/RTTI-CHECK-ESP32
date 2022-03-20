
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_task_wdt.h"
#include "soc/rtc_wdt.h"

#include <iostream>
#include <memory>

#define PRINT_USE_COUNT(p) std::cout << "Use count: " << p.use_count() << std::endl;

struct Base {
    Base() {
        std::cout << "Base constructor" << std::endl;
    }
    virtual ~Base() {
        std::cout << "Base destructor" << std::endl;
    }
    virtual int size(void) = 0;
};

struct Wi : public Base {
    int a{40};
    char b[1024];
    Wi() {
        std::cout << "Wi constructor" << std::endl;
    }
    ~Wi() {
        std::cout << "Wi destructor" << std::endl;
    }
    int size(void) {
        return sizeof(*this);
    }
    int geta(void) {
        return a;
    }
};

static QueueHandle_t xQueue = NULL;
static TimerHandle_t xTimer = NULL;

void _task_timer(void *ptr)
{
    Base *w = new Wi();
    xQueueSendToBack(xQueue, &w, 0);
}

void _task_xtask(void *ptr)
{
    while (true)
    {
        Base *pc;
        xQueueReceive(xQueue, &pc, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
        auto *v = dynamic_cast<Wi *>(pc);
        std::cout << v->geta() << std::endl; 
        delete v;
        v = nullptr;
    }
}

extern "C" void app_main(void)
{
    esp_task_wdt_init(portMAX_DELAY, pdFALSE);

    xQueue = xQueueCreate(1, sizeof(Base));
    xTimer = xTimerCreate("_task_timer", pdMS_TO_TICKS(2000), pdTRUE, NULL, _task_timer);
    xTaskCreate(_task_xtask, "_task_xtask", 4096, NULL, 2, NULL);

    if( xTimer != NULL )
    {
        xTimerStart( xTimer, 0 );
    }

    while(true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}