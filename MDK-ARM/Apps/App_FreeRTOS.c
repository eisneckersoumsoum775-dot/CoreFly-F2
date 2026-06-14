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
// 2. FOC 开环控制任务 — 电机1 (优先级：3, 周期 2ms/500Hz)
//    TIM1 PWM 引脚:
//      HIN1=PA8(TIM1_CH1)   LIN1=PE8(TIM1_CH1N)
//      HIN2=PA9(TIM1_CH2)   LIN2=PB0(TIM1_CH2N)
//      HIN3=PA10(TIM1_CH3)  LIN3=PB1(TIM1_CH3N)
//    ARR=8400, 中心对齐, 等效 20kHz
// ==========================================
void Motor1_Ctrl_Task(void *pvParameters);
#define Motor1_Ctrl_TASK_STACK_SIZE 512
#define Motor1_Ctrl_TASK_PRIORITY 3
TaskHandle_t Motor1_Ctrl_Task_handle;
#define Motor1_Ctrl_TASK_PERIOD 2            // 2ms = 500Hz


// ==========================================
// 3. 蜂鸣器测试任务 (优先级：3)
// ==========================================
void Buzzer_task(void *pvParameters);
// ==========================================
// 4. 串口测试任务 (优先级：3)
// ==========================================
void UartTest_task(void *pvParameters);
#define UartTest_TASK_STACK_SIZE 512
#define UartTest_TASK_PRIORITY 3
TaskHandle_t UartTest_task_handle;
#define UartTest_TASK_PERIOD 100         // 10Hz 发送频率


// ==========================================
// 5. MPU6050 测试任务 (浼樺厛绾э細3)
// ==========================================
void MPU6050_Test_Task(void *pvParameters);
#define MPU6050_Test_TASK_STACK_SIZE 512
#define MPU6050_Test_TASK_PRIORITY 3
TaskHandle_t MPU6050_Test_Task_handle;
#define MPU6050_Test_TASK_PERIOD 10
// ==========================================
// 6. SPL06 测试任务
// ==========================================
void SPL06_Test_Task(void *pvParameters);
#define SPL06_Test_TASK_STACK_SIZE 256
#define SPL06_Test_TASK_PRIORITY 3
TaskHandle_t SPL06_Test_Task_handle;
#define SPL06_Test_TASK_PERIOD 100


// 蜂鸣器 PWM 驱动 (TIM6 中断翻转 PE0, ~2.7kHz)
static TIM_HandleTypeDef htim6;

void Buzzer_PWM_Init(void)
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
void Buzzer_SetFreq(uint16_t freq_hz)
{
    uint32_t arr = (1000000UL / ((uint32_t)freq_hz * 2)) - 1;
    if (arr > 65535) arr = 65535;
    __HAL_TIM_SET_AUTORELOAD(&htim6, arr);
    __HAL_TIM_SET_COUNTER(&htim6, 0);
}

void Buzzer_Beep(uint16_t freq_hz, uint16_t duration_ms, uint16_t gap_ms)
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
    xTaskCreate(UartTest_task, "UartTest_task", UartTest_TASK_STACK_SIZE, NULL, UartTest_TASK_PRIORITY, &UartTest_task_handle);
    // 启动 FOC 开环控制任务 (电机1) —— 已注释
    // xTaskCreate(Motor1_Ctrl_Task, "Motor1_Ctrl_Task", Motor1_Ctrl_TASK_STACK_SIZE, NULL, Motor1_Ctrl_TASK_PRIORITY, &Motor1_Ctrl_Task_handle);
        xTaskCreate(SPL06_Test_Task, "SPL06_Test", SPL06_Test_TASK_STACK_SIZE, NULL, SPL06_Test_TASK_PRIORITY, &SPL06_Test_Task_handle);
    xTaskCreate(MPU6050_Test_Task, "MPU6050_Test", MPU6050_Test_TASK_STACK_SIZE, NULL, MPU6050_Test_TASK_PRIORITY, &MPU6050_Test_Task_handle);

    
    // 启动任务调度器
    vTaskStartScheduler();
}


// ==========================================
// 1. 测试任务 (优先级：3)
// ==========================================
void Ceshi_task(void *pvParameters)
{
    // 点亮红灯和蓝灯 (高电平有效)
    // HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




void UartTest_task(void *pvParameters)
{
    // uint32_t seq = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        // seq++;
        // uint32_t base = seq * 2;
        // printf(":%d,%d,%d,%d\r\n",
        //        (int)(base % 1000),
        //        (int)((base + 250) % 1000),
        //        (int)((base + 500) % 1000),
        //        (int)((base + 750) % 1000));

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(UartTest_TASK_PERIOD));
    }
}



// ==========================================
// 2. FOC 开环控制任务 — 电机1 (优先级：3, 周期 2ms/500Hz)
//    TIM1 PWM 引脚:
//      HIN1=PA8(TIM1_CH1)   LIN1=PE8(TIM1_CH1N)
//      HIN2=PA9(TIM1_CH2)   LIN2=PB0(TIM1_CH2N)
//      HIN3=PA10(TIM1_CH3)  LIN3=PB1(TIM1_CH3N)
//    ARR=8400, PSC=0, 中心对齐, 等效 PWM=20kHz
// ==========================================
// void Motor1_Ctrl_Task(void *pvParameters)
// {
//     /* 1. 启动 TIM1 三路互补 PWM */
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
//     HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
//     HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

//     /* 强行写入 50% 占空比 (ARR=8400的一半) */
//     TIM1->CCR1 = 4200;
//     TIM1->CCR2 = 4200;
//     TIM1->CCR3 = 4200;

//     while (1)
//     {
//         // 里面什么都不写，把 FOC 任务全部注释掉！
//         // App_Motor_Run_Task(&MyMotor);  // <-- 注释掉
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }




// ==========================================
// 5. MPU6050 测试任务
// ==========================================
void MPU6050_Test_Task(void *pvParameters)
{
    MPU6050_Attitude_t att;
    uint8_t calib_done = 0;

    /* 初始化蜂鸣器 */
    Buzzer_PWM_Init();

    /* 红灯亮, 校准开始提示音 */
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(200));
    Buzzer_Beep(1600, 100, 80);
    Buzzer_Beep(2000, 100, 0);

    /* 初始化 MPU6050 */
    if (MPU6050_Init() == 0)
    {
        printf("[MPU6050] Init FAIL\r\n");
        while (1) { HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin); vTaskDelay(pdMS_TO_TICKS(200)); }
    }

    Alg_MPU6050_Init(&att, MPU6050_Test_TASK_PERIOD * 0.001f);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* 校准中: 红灯闪烁 */
    while (!calib_done)
    {
        MPU6050_Read_Data();
        calib_done = Alg_MPU6050_Calibrate(&att,
            mpu_data.acc_x, mpu_data.acc_y, mpu_data.acc_z,
            mpu_data.gyro_x, mpu_data.gyro_y, mpu_data.gyro_z);
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU6050_Test_TASK_PERIOD));
    }

    /* 校准完成: 红灯灭, 蓝灯亮, 提示音 */
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
    Buzzer_Beep(2000, 100, 80);
    Buzzer_Beep(2800, 200, 0);

    while (1)
    {
        MPU6050_Read_Data();
        Alg_MPU6050_Update(&att,
            mpu_data.acc_x, mpu_data.acc_y, mpu_data.acc_z,
            mpu_data.gyro_x, mpu_data.gyro_y, mpu_data.gyro_z);

        if (xSemaphoreTake(USART_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            // printf("%.1f,%.1f,%.1f\r\n", att.roll, att.pitch, att.yaw);
            xSemaphoreGive(USART_Mutex);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU6050_Test_TASK_PERIOD));
    }
}

// ==========================================
// 6. SPL06 测试任务
// ==========================================
void SPL06_Test_Task(void *pvParameters)
{
    SPL06_Altitude_t alt;

    vTaskDelay(pdMS_TO_TICKS(300));

    if (xSemaphoreTake(USART_Mutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        printf("[SPL06] Init...\r\n");
        xSemaphoreGive(USART_Mutex);
    }

    if (!SPL06_Init())
    {
        if (xSemaphoreTake(USART_Mutex, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            printf("[SPL06] Init FAIL\r\n");
            xSemaphoreGive(USART_Mutex);
        }
        vTaskDelete(NULL);
    }

    Alg_SPL06_Init(&alt, 101325.0f);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        SPL06_Read_Data();
        Alg_SPL06_Update(&alt, spl06_data.pressure_pa, spl06_data.temperature_c);

        if (xSemaphoreTake(USART_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            printf("ALT:%.2f m | T:%.1f C | P:%.0f Pa\r\n", alt.altitude, alt.temperature, alt.pressure);
            xSemaphoreGive(USART_Mutex);
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SPL06_Test_TASK_PERIOD));
    }
}