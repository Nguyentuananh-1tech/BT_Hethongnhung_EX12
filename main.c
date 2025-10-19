#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_PIN     GPIO_Pin_12
#define LED_PORT    GPIOB

typedef struct {
    uint32_t on_time;   
    uint32_t off_time;  
} BlinkPattern;

QueueHandle_t xQueue;

void GPIO_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = LED_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_PORT, &gpio);
}

void BlinkTask(void *pvParameters)
{
    BlinkPattern pattern = {500, 500}; 
    BlinkPattern recvData;

    while (1)
    {
        if (xQueueReceive(xQueue, &recvData, 0) == pdPASS)
            pattern = recvData;

        GPIO_SetBits(LED_PORT, LED_PIN);
        vTaskDelay(pattern.on_time / portTICK_PERIOD_MS);

        GPIO_ResetBits(LED_PORT, LED_PIN);
        vTaskDelay(pattern.off_time / portTICK_PERIOD_MS);
    }
}

void ControlTask(void *pvParameters)
{
    BlinkPattern pattern;
    int index = 0;

    while (1)
    {
        switch (index)
        {
            case 0: pattern.on_time = 1000; pattern.off_time = 1000; break; 
            case 1: pattern.on_time = 200;  pattern.off_time = 200;  break; 
            case 2: pattern.on_time = 1500; pattern.off_time = 500;  break; 
        }

        xQueueSend(xQueue, &pattern, 0);
        index = (index + 1) % 3;

        vTaskDelay(5000 / portTICK_PERIOD_MS); 
    }
}

int main(void)
{
    SystemInit();
    GPIO_Config();

    xQueue = xQueueCreate(5, sizeof(BlinkPattern));

    xTaskCreate(BlinkTask, "Blink", 128, NULL, 1, NULL);
    xTaskCreate(ControlTask, "Control", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}