# CoreFly F2

基于 STM32F407 的四轴无人机飞控系统，采用无感 FOC 驱动方案。

## 硬件平台

| 模块 | 型号 |
|------|------|
| 主控 | STM32F407VET6 (Cortex-M4F, 168MHz) |
| 栅极驱动 | FD6288Q |
| MOSFET | AON7418 × 24 |
| IMU | BMI160 / BMI270 |
| 气压计 | SPL06-001 |
| 无线通信 | Ra-01SC (LoRa) |
| 电机 | 4× BLDC 无感 FOC |

## 软件架构

- **IDE**: Keil MDK-ARM V5.32
- **RTOS**: FreeRTOS (tick 1000Hz, heap_4)
- **电机控制**: SVPWM + 无感 FOC (SMO + PLL)
- **传感器**: IMU 姿态解算、气压计高度估计
- **通信**: LoRa 遥控 + 数传

## 目录结构

```
├── Core/                  # STM32CubeMX 生成的 HAL 外设代码
│   ├── Inc/               # 头文件 (ADC, I2C, SPI, TIM, USART...)
│   └── Src/               # 源文件
├── Drivers/               # HAL 库驱动
├── MDK-ARM/               # Keil 工程
│   ├── Algorithms/        # FOC 算法 (Clarke/Park/SVPWM)
│   ├── Apps/              # 应用层 (FreeRTOS 任务, 电机控制)
│   ├── BSP/               # 板级支持包 (IMU, LoRa, 气压计)
│   ├── FreeRTOS/          # FreeRTOS 内核
│   └── 软件设计计划书.txt   # 开发手册
├── CoreFly F2 资料/        # 设计资料
│   ├── 原理图/             # PCB/SCH 工程文件
│   ├── 数据手册/           # 芯片数据手册
│   ├── 板子/               # BOM 清单
│   └── FOC/               # FOC 参考资料
├── 图/                    # 硬件效果图
├── 遥控器/                 # 遥控器 UI 原型
└── CoreFly F2.ioc         # CubeMX 配置文件
```

## 开发阶段

参考 `MDK-ARM/软件设计计划书.txt` 获取完整开发手册，包含：

1. FOC 理论基础 (Clarke/Park/SVPWM 推导)
2. 无感 FOC 角度估计 (SMO + PLL)
3. 单路电机 FOC 闭环
4. 传感器与姿态解算
5. 四电机扩展与飞控核心
6. 通信与安全监控
7. 系统集成与飞行测试

## 快速开始

1. 安装 Keil MDK-ARM V5.32+
2. 安装 STM32F4xx 器件包
3. 打开 `MDK-ARM/CoreFly F2.uvprojx`
4. 编译下载

## License

待定
