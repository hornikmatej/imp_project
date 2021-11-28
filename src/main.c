/**
 * @author Matej Hornik (xhorni20@stud.fit.vutbr.cz)
 * @date 28.11.2021
 * @brief Projekt IMP - ESP32: Přístupový terminál
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define INCLUDE_vTaskSuspend 1
#define PASSWORD_LENGHT 4
#define GPIO_LED_RED    12 
#define GPIO_LED_GREEN  13

#define GPIO_ROW2_7     19 // 7 pin - ROW2
#define GPIO_ROW3_6     18 // 6 pin - ROW3
#define GPIO_COL3_5     26 // 5 pin - COL3
#define GPIO_ROW4_4     25 // 4 pin - ROW4
#define GPIO_COL1_3     17 // 3 pin - COL1
#define GPIO_ROW1_2     16 // 2 pin - ROW1
#define GPIO_COL2_1     27 // 1 pin - COL2

#define OUTPUT_PIN_MASK ((1ULL<<GPIO_ROW1_2) | (1ULL<<GPIO_ROW2_7) | (1ULL<<GPIO_ROW3_6) | (1ULL<<GPIO_ROW4_4))
#define INPUT_PIN_MASK ((1ULL<<GPIO_COL1_3) | (1ULL<<GPIO_COL2_1) | (1ULL<<GPIO_COL3_5))

// globalne premeny
static xQueueHandle gpio_evt_queue = NULL;
uint32_t row;
SemaphoreHandle_t sem_handle = NULL;
char pw[PASSWORD_LENGHT] = "1234";
bool pressed = false;
TaskHandle_t red_blink;

/**
 * Funkcia ktora vypise do terminalu kolko sekund bezi ESP32
 */
void hello_task(void *pvParameter)
{
    printf("Hello world!\n");
    for (int i = 1; 1 ; i++) {
        printf("--------- Running %d seconds... ---------\n", i);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

/**
 * Funkcia je vyvolana v pripade zmeny logickej hodnoty na GPIO vstupe `GPIO_COL1_3` = 17
 */
static void IRAM_ATTR gpio_isr_handler1(void* arg)
{
    char znak;
    static uint32_t pressTimestamp = 0;
    // ulozenie akutalneho casu, kvoli efektu sumu pri stisknuti tlacitka
    uint32_t ts = esp_log_timestamp();
    uint32_t dif = ts - pressTimestamp;
    pressTimestamp = ts;
    // debounce time musi byt vacsi ako 20 milisekund
    if (dif > 20){
        if (pressed){ 
            // pri pustani tlacitka funkcia uvolni semafor aby mohla pokracovat v testovani stisknutia tlacitok
            pressed = false;
            xSemaphoreGiveFromISR(sem_handle, NULL);
        }
        else {
            pressed = true;
        }
        // do fronty sa ulozi ktore tlacitko bolo stisknute
        if (row == 1){
            znak = '1';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 2){
            znak = '4';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 3){
            znak = '7';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 4){
            znak = '*';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        return;
    }
}

/**
 * Funkcia je vyvolana v pripade zmeny logickej hodnoty na GPIO vstupe `GPIO_COL2_1` = 27
 */
static void IRAM_ATTR gpio_isr_handler2(void* arg)
{
    char znak;
    static uint32_t pressTimestamp = 0;
    // ulozenie akutalneho casu, kvoli efektu sumu pri stisknuti tlacitka
    uint32_t ts = esp_log_timestamp();
    uint32_t dif = ts - pressTimestamp;
    pressTimestamp = ts;
    // debounce time musi byt vacsi ako 20 milisekund
    if (dif > 20){
        if (pressed){ 
            // pri pustani tlacitka funkcia uvolni semafor aby mohla pokracovat v testovani stisknutia tlacitok
            pressed = false;
            xSemaphoreGiveFromISR(sem_handle, NULL);
        }
        else {
            pressed = true;
        }
        // do fronty sa ulozi ktore tlacitko bolo stisknute
        if (row == 1){
            znak = '2';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 2){
            znak = '5';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 3){
            znak = '8';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 4){
            znak = '0';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        return;
    }
}

/**
 * Funkcia je vyvolana v pripade zmeny logickej hodnoty na GPIO vstupe `GPIO_COL3_5` = 26
 */
static void IRAM_ATTR gpio_isr_handler3(void* arg)
{
    char znak;
    static uint32_t pressTimestamp = 0;
    // ulozenie akutalneho casu, kvoli efektu sumu pri stisknuti tlacitka
    uint32_t ts = esp_log_timestamp();
    uint32_t dif = ts - pressTimestamp;
    pressTimestamp = ts;
    // debounce time musi byt vacsi ako 20 milisekund
    if (dif > 20){
        if (pressed){ 
            // pri pustani tlacitka funkcia uvolni semafor aby mohla pokracovat v testovani stisknutia tlacitok
            pressed = false;
            xSemaphoreGiveFromISR(sem_handle, NULL);
        }
        else {
            pressed = true;
        }
        // do fronty sa ulozi ktore tlacitko bolo stisknute
        if (row == 1){
            znak = '3';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 2){
            znak = '6';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 3){
            znak = '9';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        if (row == 4){
            znak = '#';
            xQueueSendFromISR(gpio_evt_queue, &znak, NULL);
        }
        return;
    }
}

/**
 * Funkcia prijima znaky z fronty a stara sa logiku programu
 */
static void main_logic(void* arg)
{
    char znak;
    char buff[PASSWORD_LENGHT];
    char buff8[PASSWORD_LENGHT];
    int num_puts = 0;
    bool change_mod = false;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &znak, portMAX_DELAY)) {
            if (pressed) printf("Pressed  -> [%c]\n", znak);
            else printf("Released -> [%c]\n", znak);

            if (pressed){
                // pri stisknutom tlacitku sa vykonava logika programu
                if (change_mod){
                    //mod zmenenia hesla
                    if (znak == '#'){
                        // vypnutie modu zmeny hesla
                        change_mod = false;
                        vTaskSuspend(red_blink);
                        gpio_set_level(GPIO_LED_RED, 1);
                        num_puts = 0;
                        continue;
                    }
                    if (znak == '*'){
                        continue;
                    }
                    num_puts += 1;
                    // ukladanie znakov v mode zmeny hesla
                    buff8[num_puts - 1] = znak;
                    if (num_puts == (PASSWORD_LENGHT + PASSWORD_LENGHT)){
                        if (strncmp(pw, buff8, PASSWORD_LENGHT) == 0){
                            //potvrdenie spravneho hesla 3x blikne green
                            for (int i = 0; i < 2; i++){
                                gpio_set_level(GPIO_LED_GREEN, 1);
                                vTaskDelay(200 / portTICK_RATE_MS);
                                gpio_set_level(GPIO_LED_GREEN, 0);
                                vTaskDelay(200 / portTICK_RATE_MS);
                            }
                            // ulozenie noveho hesla
                            strncpy(pw, buff8 + 4, PASSWORD_LENGHT);
                        }
                        // ukoncenie modu zmeny hesla
                        change_mod = false;
                        vTaskSuspend(red_blink);
                        gpio_set_level(GPIO_LED_RED, 1);
                        num_puts = 0;
                        continue;
                    }
                    continue;
                }
                if (znak == '#'){
                    //mod zmenenia hesla
                    change_mod = true;
                    num_puts = 0;
                    vTaskResume(red_blink);
                    continue;
                }
                if (znak == '*'){
                    // vymaze sa buffer znakov pri zadavani hesla a blikne cervene ledka
                    num_puts = 0;
                    gpio_set_level(GPIO_LED_RED, 0);
                    vTaskDelay(100 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_LED_RED, 1);
                    continue;
                }
                // ukladanie znakov z fronty
                num_puts += 1;
                buff[num_puts - 1] = znak;
                if (num_puts == PASSWORD_LENGHT){
                    num_puts = 0;
                    // v pripade zadanych 4 znakov a spravneho hesla zasvieti zelena na 1 sekundu
                    if (strncmp(pw, buff, PASSWORD_LENGHT) == 0){
                        gpio_set_level(GPIO_LED_GREEN, 1);
                        gpio_set_level(GPIO_LED_RED, 0);
                        vTaskDelay(1000 / portTICK_RATE_MS);
                        gpio_set_level(GPIO_LED_GREEN, 0);
                        gpio_set_level(GPIO_LED_RED, 1);
                    }
                }
            }
        }
    }
}

/**
 * Funkcia sa stara o blikanie cervenej ledky v pripade zmeny hesla
 */
void red_blink_task(void* arg){
    int cnt = 0;
    while(1) {
        // blinking every 250 ms
        gpio_set_level(GPIO_LED_RED, cnt & 0x01);
        vTaskDelay(250 / portTICK_RATE_MS);
        cnt++;
    }
}

/**
 * Funkcia kontroluje stisk tlacitok na keypade 
 * na 10 milisekund zapne jeden riadok a potom dalsi
 * ak bolo tlacitko stisknute tak caka na signal pustenia tlacitka aby mohla testovat dalej
 */
void check_task(void* arg){
     while(1){
        row = 1;
        gpio_set_level(GPIO_ROW1_2   , 1);
        vTaskDelay(10 / portTICK_RATE_MS);
        if (pressed){
            if( xSemaphoreTake(sem_handle, portMAX_DELAY) == pdTRUE ){
            }
        }
        gpio_set_level(GPIO_ROW1_2   , 0);

        row = 2;
        gpio_set_level(GPIO_ROW2_7   , 1);
        vTaskDelay(10 / portTICK_RATE_MS);
        if (pressed){
            if( xSemaphoreTake(sem_handle, portMAX_DELAY) == pdTRUE ){
            }
        }
        gpio_set_level(GPIO_ROW2_7   , 0);

        row = 3;
        gpio_set_level(GPIO_ROW3_6   , 1);
        vTaskDelay(10 / portTICK_RATE_MS);
        if (pressed){
            if( xSemaphoreTake(sem_handle, portMAX_DELAY) == pdTRUE ){
            }
        }
        gpio_set_level(GPIO_ROW3_6   , 0);

        row = 4;
        gpio_set_level(GPIO_ROW4_4 , 1);
        vTaskDelay(10 / portTICK_RATE_MS);
        if (pressed){
            if( xSemaphoreTake(sem_handle, portMAX_DELAY) == pdTRUE ){
            }
        }
        gpio_set_level(GPIO_ROW4_4   , 0);
    }
}

void app_main() {
    // konfiguracia vystupnych pinov
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = OUTPUT_PIN_MASK, // set as output mode...
        .mode = GPIO_MODE_OUTPUT,            // ...using an appropriate bit mask
        .pull_up_en = 0,                     // disable pull-down mode
        .pull_down_en = 1,                   // disable pull-up mode
        .intr_type = GPIO_INTR_DISABLE       // configure GPIO with the given settings
    };
    gpio_config(&gpio_cfg);  // set the configuration

    // konfiguracia vstupnych pinov
    gpio_cfg.intr_type = GPIO_INTR_ANYEDGE;  // interrupt of rising edge
    gpio_cfg.pin_bit_mask = INPUT_PIN_MASK;  // 
    gpio_cfg.mode = GPIO_MODE_INPUT;         // set GPIOs as inputs
    gpio_cfg.pull_up_en = 0;
    gpio_cfg.pull_down_en = 1;                  
    gpio_config(&gpio_cfg);

    gpio_install_isr_service(0); //DEFAULT FLAG

    //  nastavenie interupt funkcii pre jednotlive vstupne piny
    gpio_isr_handler_add(GPIO_COL1_3, gpio_isr_handler1, (void*) GPIO_COL1_3);
    gpio_isr_handler_add(GPIO_COL2_1, gpio_isr_handler2, (void*) GPIO_COL2_1);
    gpio_isr_handler_add(GPIO_COL3_5, gpio_isr_handler3, (void*) GPIO_COL3_5);
    // vytvorenie fronty pre stisknute znaky
    gpio_evt_queue = xQueueCreate(10, sizeof(char));
    // vytvorenie semaforu
    sem_handle = xSemaphoreCreateBinary();
    if (sem_handle == NULL){
        fprintf(stderr,"ERROR while creating semahore");
        exit(1);
    }

    // nastavenie lediek
    gpio_pad_select_gpio(GPIO_LED_GREEN);
    gpio_pad_select_gpio(GPIO_LED_RED);
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_GREEN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT));
    // vytvorenie taskov
    xTaskCreate(&hello_task, "hello_task", 2048, NULL, 5, NULL);
    xTaskCreate(&main_logic, "main_logic", 16384, NULL, 5, NULL);
    xTaskCreate(&check_task, "check_task", 2048, NULL, 5, NULL);
    xTaskCreate(&red_blink_task, "red_blink_task", 1024, NULL, 5, &red_blink);

    vTaskSuspend(red_blink);
    gpio_set_level(GPIO_LED_RED, 1);
}