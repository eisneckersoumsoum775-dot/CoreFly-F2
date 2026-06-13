#include "App_FreeRTOS.h"
#include "tim.h"
#include "App_config.h"

Motor_Ctrl_t MyMotor;

/* 定义 I2C 互斥量 */
SemaphoreHandle_t IIC_Mutex = NULL;
/* 定义 SPI 互斥量 */
SemaphoreHandle_t SPI2_Mutex = NULL;
SemaphoreHandle_t SPI3_Mutex = NULL;
/* 定义 UART 互斥量，保护 printf 线程安全 */
SemaphoreHandle_t USART_Mutex = NULL;



// ==========================================
// 1. 测试任务 (优先级：3)
// ==========================================
void Ceshi_task(void *pvParameters);
#define Ceshi_TASK_STACK_SIZE 256
#define Ceshi_TASK_PRIORITY 3
TaskHandle_t Ceshi_task_handle;
#define Ceshi_TASK_PERIOD 100


// ==========================================
// 2. 转换测试任务 (优先级：3)
// ==========================================
// void Zhuanhuan_task(void *pvParameters);
// #define Zhuanhuan_TASK_STACK_SIZE 512
// #define Zhuanhuan_TASK_PRIORITY 3
// TaskHandle_t Zhuanhuan_task_handle;
// #define Zhuanhuan_TASK_PERIOD 2            // 2ms 控制周期


// ==========================================
// 3. 蜂鸣器测试任务 (优先级：3)
// ==========================================
void Buzzer_task(void *pvParameters);
#define Buzzer_TASK_STACK_SIZE 256
#define Buzzer_TASK_PRIORITY 3
TaskHandle_t Buzzer_task_handle;


// ==========================================
// 4. 串口测试任务 (优先级：3)
// ==========================================
void UartTest_task(void *pvParameters);
#define UartTest_TASK_STACK_SIZE 512
#define UartTest_TASK_PRIORITY 3
TaskHandle_t UartTest_task_handle;
#define UartTest_TASK_PERIOD 100         // 10Hz 发送频率


// 蜂鸣器 PWM 驱动 (TIM6 中断翻转 PE0, ~2.7kHz)
static TIM_HandleTypeDef htim6;

static void Buzzer_PWM_Init(void)
{
    __HAL_RCC_TIM6_CLK_ENABLE();
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 83;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 184;
    htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim6);
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        HAL_GPIO_TogglePin(BEEP_GPIO_Port, BEEP_Pin);
    }
}


// 蜂鸣器音调控制
static void Buzzer_SetFreq(uint16_t freq_hz)
{
    uint32_t arr = (1000000UL / ((uint32_t)freq_hz * 2)) - 1;
    if (arr > 65535) arr = 65535;
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr);
    __HAL_TIM_SET_COUNTER(&htim6, 0);
}

static void Buzzer_Beep(uint16_t freq_hz, uint16_t duration_ms, uint16_t gap_ms)
{
    Buzzer_SetFreq(freq_hz);
    HAL_TIM_Base_Start_IT(&htim6);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    HAL_TIM_Base_Stop_IT(&htim6);
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    if (gap_ms) vTaskDelay(pdMS_TO_TICKS(gap_ms));
}




/**
 * @brief 启动FreeRTOS任务
 */
void App_FreeRTOS_start(void)
{
    /* 创建互斥量 */
    IIC_Mutex = xSemaphoreCreateMutex();
    SPI2_Mutex = xSemaphoreCreateMutex();
    SPI3_Mutex = xSemaphoreCreateMutex();
    USART_Mutex = xSemaphoreCreateMutex();

    // 创建测试任务
    xTaskCreate(Ceshi_task, "Ceshi_task", Ceshi_TASK_STACK_SIZE, NULL, Ceshi_TASK_PRIORITY, &Ceshi_task_handle);
    xTaskCreate(Buzzer_task, "Buzzer_task", Buzzer_TASK_STACK_SIZE, NULL, Buzzer_TASK_PRIORITY, &Buzzer_task_handle);
    xTaskCreate(UartTest_task, "UartTest_task", UartTest_TASK_STACK_SIZE, NULL, UartTest_TASK_PRIORITY, &UartTest_task_handle);
    // 启动 FOC 开环控制任务
    // xTaskCreate(Zhuanhuan_task, "Zhuanhuan_task", Zhuanhuan_TASK_STACK_SIZE, NULL, Zhuanhuan_TASK_PRIORITY, &Zhuanhuan_task_handle);
    
    // 启动任务调度器
    vTaskStartScheduler();
}


// ==========================================
// 1. 测试任务 (优先级：3)
// ==========================================
void Ceshi_task(void *pvParameters)
{
    // 点亮红灯和蓝灯 (低电平有效)
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void Buzzer_task(void *pvParameters)
{
    Buzzer_PWM_Init();

    // 未连接 —— 低沉三连降
    Buzzer_Beep(2800, 150, 60);
    Buzzer_Beep(2200, 150, 60);
    Buzzer_Beep(1600, 250, 500);

    // 已连接 —— 轻快三连升
    Buzzer_Beep(1600, 80, 60);
    Buzzer_Beep(2200, 80, 60);
    Buzzer_Beep(2800, 150, 0);

    vTaskDelete(NULL);
}


void UartTest_task(void *pvParameters)
{
    uint32_t seq = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        seq++;
        uint32_t base = seq * 2;
        printf("%d,%d,%d,%d\n",
               (int)(base % 1000),
               (int)((base + 250) % 1000),
               (int)((base + 500) % 1000),
               (int)((base + 750) % 1000));

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(UartTest_TASK_PERIOD));
    }
}


// // ==========================================
// // 2. FOC 开环控制任务 (优先级：3, 周期 2ms)
// // ==========================================
// void Zhuanhuan_task(void *pvParameters)
// {
//     /* ── 1. 启动 TIM1 三路互补 PWM ── */
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

//     /* ── 2. 初始化 FOC 算法 ──
//        注意: PWM_ARR = 3500，必须与 TIM1->Period 一致！ */
//     App_Motor_Init(&MyMotor, 12.0f, 3500);
//     MyMotor.MathParam.V_q_target = 2.0f;   // 开环电压幅值 (从 4.0V 降到 2.0V 更安全)
//     App_Motor_Set_Speed(&MyMotor, 0.02f);  // 目标电角速度 (rad/s)

//     /* ── 3. 运行状态打印 ── */
//     printf("FOC OpenLoop Started, Vq=2.0V, Speed=0.02rad/s\n");

//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     uint32_t print_cnt = 0;

//     while (1)
//     {
//         /* 运行 FOC 计算: InvPark → SVPWM → 更新 CCR_a/b/c */
//         App_Motor_Run_Task(&MyMotor);

//         /* 更新硬件 PWM 比较值 */
//         TIM1->CCR1 = MyMotor.MathParam.CCR_a;
//         TIM1->CCR2 = MyMotor.MathParam.CCR_b;
//         TIM1->CCR3 = MyMotor.MathParam.CCR_c;

//         /* 每 25 次循环 (~50ms) 打印一次，避免串口溢出 */
//         print_cnt++;
//         if (print_cnt >= 25) {
//             print_cnt = 0;
//             printf(":%d,%d,%d,%.1f\n",
//                    (int)MyMotor.MathParam.CCR_a,
//                    (int)MyMotor.MathParam.CCR_b,
//                    (int)MyMotor.MathParam.CCR_c,
//                    MyMotor.MathParam.Theta * 1000.0f);
//         }

//         /* 绝对精准延时 2ms */
//         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(Zhuanhuan_TASK_PERIOD));
//     }
// }
