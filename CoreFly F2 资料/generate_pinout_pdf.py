#!/usr/bin/env python3
"""生成 CoreFly F2 引脚分配表 PDF (含网络标签)"""
import os
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Table,
                                 TableStyle, PageBreak)
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

try:
    pdfmetrics.registerFont(TTFont('YaHei', 'C:/Windows/Fonts/msyh.ttc', subfontIndex=0))
    CN = 'YaHei'
except:
    try:
        pdfmetrics.registerFont(TTFont('SimHei', 'C:/Windows/Fonts/simhei.ttf'))
        CN = 'SimHei'
    except:
        CN = 'Helvetica'

BASE = r"F:\Zhuomian\CoreFly F2\CoreFly F2 资料"

DARK  = colors.HexColor('#1a1a2e')
BLUE  = colors.HexColor('#0f3460')
WHITE = colors.white
GRAY  = colors.HexColor('#f5f5f5')
GRID  = colors.HexColor('#cccccc')
TAG_BG = colors.HexColor('#e8f0fe')

def S(name, **kw):
    defaults = dict(fontName=CN, fontSize=9, leading=14, textColor=colors.HexColor('#333'))
    defaults.update(kw)
    return ParagraphStyle(name, **defaults)

STYLES = {
    'title':    S('t', fontSize=20, leading=28, alignment=TA_CENTER, textColor=DARK),
    'subtitle': S('st', fontSize=11, leading=16, alignment=TA_CENTER, textColor=colors.HexColor('#666')),
    'h1':       S('h1', fontSize=13, leading=20, textColor=BLUE),
    'body':     S('body', fontSize=9, leading=14),
    'note':     S('note', fontSize=8, leading=12, textColor=colors.HexColor('#888')),
    'td':       S('td', fontSize=7.2, leading=10.5),
    'tdc':      S('tdc', fontSize=7.2, leading=10.5, alignment=TA_CENTER),
    'tdt':      S('tdt', fontSize=7.2, leading=10.5, alignment=TA_CENTER, textColor=colors.HexColor('#1a56db')),
    'th':       S('th', fontSize=7.5, leading=11, alignment=TA_CENTER, textColor=WHITE),
}

def C(text, net=False):
    """根据列类型选样式"""
    if net:
        return Paragraph(f'<b>{text}</b>', STYLES['tdt'])
    return Paragraph(str(text), STYLES['tdc'])

def T(rows, col_widths, net_cols=None, header_bg=BLUE):
    """
    建表格。
    net_cols: set of column indices that should use bold blue net-label style.
    """
    if net_cols is None:
        net_cols = set()
    data = []
    for i, row in enumerate(rows):
        r = []
        for j, cell in enumerate(row):
            if i == 0:
                r.append(Paragraph(str(cell), STYLES['th']))
            elif j in net_cols:
                r.append(Paragraph(f'<b>{cell}</b>', STYLES['tdt']))
            elif j == 0:
                r.append(Paragraph(str(cell), STYLES['td']))
            else:
                r.append(Paragraph(str(cell), STYLES['tdc']))
        data.append(r)
    t = Table(data, colWidths=col_widths, repeatRows=1)
    style = [
        ('BACKGROUND', (0, 0), (-1, 0), header_bg),
        ('TEXTCOLOR', (0, 0), (-1, 0), WHITE),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('GRID', (0, 0), (-1, -1), 0.4, GRID),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [WHITE, GRAY]),
        ('TOPPADDING', (0, 0), (-1, -1), 3.5),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3.5),
        ('LEFTPADDING', (0, 0), (-1, -1), 3),
        ('RIGHTPADDING', (0, 0), (-1, -1), 3),
    ]
    t.setStyle(TableStyle(style))
    return t

def H1(text): return Paragraph(f'<b>{text}</b>', STYLES['h1'])
def B(text):  return Paragraph(text, STYLES['body'])
def SP(mm_val=3): return Spacer(1, mm_val*mm)

# ═══════════════════════════════════════════
# 数据
# ═══════════════════════════════════════════

esc_pwm = [
    ["ESC", "FD6288T 引脚", "MCU 引脚", "定时器", "MCU侧标签", "FD侧标签"],
    # ESC1
    ["ESC1\n(U4)", "1 HIN1",  "PA8",  "TIM1_CH1",  "M1_HIN1", "HIN1-1"],
    ["",          "2 HIN2",  "PA9",  "TIM1_CH2",  "M1_HIN2", "HIN2-1"],
    ["",          "3 HIN3",  "PA10", "TIM1_CH3",  "M1_HIN3", "HIN3-1"],
    ["",          "4 LIN1",  "PB0",  "TIM1_CH2N", "M1_LIN1", "LIN1-1"],
    ["",          "5 LIN2",  "PB1",  "TIM1_CH3N", "M1_LIN2", "LIN2-1"],
    ["",          "6 LIN3",  "PE8",  "TIM1_CH1N", "M1_LIN3", "LIN3-1"],
    # ESC2
    ["ESC2\n(U3)", "1 HIN1",  "PC6",  "TIM8_CH1",  "M2_HIN1", "HIN1-2"],
    ["",          "2 HIN2",  "PC7",  "TIM8_CH2",  "M2_HIN2", "HIN2-2"],
    ["",          "3 HIN3",  "PC8",  "TIM8_CH3",  "M2_HIN3", "HIN3-2"],
    ["",          "4 LIN1",  "PB14", "TIM8_CH2N", "M2_LIN1", "LIN1-2"],
    ["",          "5 LIN2",  "PB15", "TIM8_CH3N", "M2_LIN2", "LIN2-2"],
    ["",          "6 LIN3",  "PC9",  "TIM8_CH4",  "M2_LIN3", "LIN3-2"],
    # ESC3
    ["ESC3\n(U6)", "1 HIN1",  "PA15", "TIM2_CH1",  "M3_HIN1", "HIN1-3"],
    ["",          "2 HIN2",  "PB3",  "TIM2_CH2",  "M3_HIN2", "HIN2-3"],
    ["",          "3 HIN3",  "PB10", "TIM2_CH3",  "M3_HIN3", "HIN3-3"],
    ["",          "4 LIN1",  "PB11", "TIM2_CH4",  "M3_LIN1", "LIN1-3"],
    ["",          "5 LIN2",  "PE5",  "TIM9_CH1",  "M3_LIN2", "LIN2-3"],
    ["",          "6 LIN3",  "PE6",  "TIM9_CH2",  "M3_LIN3", "LIN3-3"],
    # ESC4
    ["ESC4\n(U5)", "1 HIN1",  "PD12", "TIM4_CH1",  "M4_HIN1", "HIN1-4"],
    ["",          "2 HIN2",  "PD13", "TIM4_CH2",  "M4_HIN2", "HIN2-4"],
    ["",          "3 HIN3",  "PD14", "TIM4_CH3",  "M4_HIN3", "HIN3-4"],
    ["",          "4 LIN1",  "PD15", "TIM4_CH4",  "M4_LIN1", "LIN1-4"],
    ["",          "5 LIN2",  "PB8",  "TIM10_CH1", "M4_LIN2", "LIN2-4"],
    ["",          "6 LIN3",  "PB9",  "TIM11_CH1", "M4_LIN3", "LIN3-4"],
]

esc_gate = [
    ["ESC", "FD6288T 引脚", "目标 MOSFET", "栅极标签", "下拉标签"],
    ["ESC1\n(U4)", "19 HO1", "Q1 UH", "HO1-1", "M1_UH_Gate"],
    ["",          "16 HO2", "Q3 VH", "HO2-1", "M1_VH_Gate"],
    ["",          "13 HO3", "Q5 WH", "HO3-1", "M1_WH_Gate"],
    ["",          "11 LO1", "Q2 UL", "LO1-1", "M1_UL_Gate"],
    ["",          "10 LO2", "Q4 VL", "LO2-1", "M1_VL_Gate"],
    ["",          "9  LO3", "Q6 WL", "LO3-1", "M1_WL_Gate"],
    ["",          "18 VS1", "Q1.3+Q2.2 → P1", "SU-1", "(电机U相)"],
    ["",          "15 VS2", "Q3.3+Q4.2 → P2", "SV-1", "(电机V相)"],
    ["",          "12 VS3", "Q5.3+Q6.2 → P3", "SW-1", "(电机W相)"],
    ["ESC2\n(U3)", "19 HO1", "Q7 UH", "HO1-2", "M2_UH_Gate"],
    ["",          "16 HO2", "Q9 VH", "HO2-2", "M2_VH_Gate"],
    ["",          "13 HO3", "Q11 WH", "HO3-2", "M2_WH_Gate"],
    ["",          "11 LO1", "Q8 UL", "LO1-2", "M2_UL_Gate"],
    ["",          "10 LO2", "Q10 VL", "LO2-2", "M2_VL_Gate"],
    ["",          "9  LO3", "Q12 WL", "LO3-2", "M2_WL_Gate"],
    ["",          "18 VS1", "Q7.3+Q8.2 → P4", "SU-2", "(电机U相)"],
    ["",          "15 VS2", "Q9.3+Q10.2 → P5", "SV-2", "(电机V相)"],
    ["",          "12 VS3", "Q11.3+Q12.2 → P6", "SW-2", "(电机W相)"],
    ["ESC3\n(U6)", "19 HO1", "Q13 UH", "HO1-3", "M3_UH_Gate"],
    ["",          "16 HO2", "Q15 VH", "HO2-3", "M3_VH_Gate"],
    ["",          "13 HO3", "Q17 WH", "HO3-3", "M3_WH_Gate"],
    ["",          "11 LO1", "Q14 UL", "LO1-3", "M3_UL_Gate"],
    ["",          "10 LO2", "Q16 VL", "LO2-3", "M3_VL_Gate"],
    ["",          "9  LO3", "Q18 WL", "LO3-3", "M3_WL_Gate"],
    ["",          "18 VS1", "Q13.3+Q14.2 → P7", "SU-3", "(电机U相)"],
    ["",          "15 VS2", "Q15.3+Q16.2 → P8", "SV-3", "(电机V相)"],
    ["",          "12 VS3", "Q17.3+Q18.2 → P9", "SW-3", "(电机W相)"],
    ["ESC4\n(U5)", "19 HO1", "Q19 UH", "HO1-4", "M4_UH_Gate"],
    ["",          "16 HO2", "Q21 VH", "HO2-4", "M4_VH_Gate"],
    ["",          "13 HO3", "Q23 WH", "HO3-4", "M4_WH_Gate"],
    ["",          "11 LO1", "Q20 UL", "LO1-4", "M4_UL_Gate"],
    ["",          "10 LO2", "Q22 VL", "LO2-4", "M4_VL_Gate"],
    ["",          "9  LO3", "Q24 WL", "LO3-4", "M4_WL_Gate"],
    ["",          "18 VS1", "Q19.3+Q20.2 → P10", "SU-4", "(电机U相)"],
    ["",          "15 VS2", "Q21.3+Q22.2 → P11", "SV-4", "(电机V相)"],
    ["",          "12 VS3", "Q23.3+Q24.2 → P12", "SW-4", "(电机W相)"],
]

esc_bootstrap = [
    ["ESC", "VB 引脚", "VS 引脚", "电容", "自举标签 VB", "自举标签 VS"],
    ["ESC1 (U4)", "20 VB1", "18 VS1", "C26 2.2uF", "VB1-1", "SU-1"],
    ["",         "17 VB2", "15 VS2", "C25 2.2uF", "VB2-1", "SV-1"],
    ["",         "14 VB3", "12 VS3", "C24 2.2uF", "VB3-1", "SW-1"],
    ["ESC2 (U3)", "20 VB1", "18 VS1", "C29 2.2uF", "VB1-2", "SU-2"],
    ["",         "17 VB2", "15 VS2", "C28 2.2uF", "VB2-2", "SV-2"],
    ["",         "14 VB3", "12 VS3", "C27 2.2uF", "VB3-2", "SW-2"],
    ["ESC3 (U6)", "20 VB1", "18 VS1", "C32 2.2uF", "VB1-3", "SU-3"],
    ["",         "17 VB2", "15 VS2", "C31 2.2uF", "VB2-3", "SV-3"],
    ["",         "14 VB3", "12 VS3", "C30 2.2uF", "VB3-3", "SW-3"],
    ["ESC4 (U5)", "20 VB1", "18 VS1", "C35 2.2uF", "VB1-4", "SU-4"],
    ["",         "17 VB2", "15 VS2", "C34 2.2uF", "VB2-4", "SV-4"],
    ["",         "14 VB3", "12 VS3", "C33 2.2uF", "VB3-4", "SW-4"],
]

esc_power = [
    ["ESC", "VCC 引脚", "COM 引脚", "去耦电容", "VCC 标签"],
    ["ESC1 (U4)", "7 VCC",  "8 COM",  "C22 100nF + C23 10uF → GND", "FD_VCC1"],
    ["ESC2 (U3)", "7 VCC",  "8 COM",  "复接 +12V 总线",               "FD_VCC2"],
    ["ESC3 (U6)", "7 VCC",  "8 COM",  "复接 +12V 总线",               "FD_VCC3"],
    ["ESC4 (U5)", "7 VCC",  "8 COM",  "复接 +12V 总线",               "FD_VCC4"],
]

esc_sense = [
    ["ESC", "采样电阻", "MOS 源极", "运放 (需添加)", "增益", "ADC 标签"],
    ["ESC1", "R19 2mΩ", "Q2.3 (UL)", "TPA181A2 x2", "50V/V", "IADC1_U / IADC1_V"],
    ["ESC2", "R40 2mΩ", "Q8.3 (UL)", "TPA181A2 x2", "50V/V", "IADC2_U / IADC2_V"],
    ["ESC3", "R61 2mΩ", "Q14.3 (UL)", "TPA181A2 x2", "50V/V", "IADC3_U / IADC3_V"],
    ["ESC4", "R82 2mΩ", "Q20.3 (UL)", "TPA181A2 x2", "50V/V", "IADC4_U / IADC4_V"],
    ["",     "R20 2mΩ", "Q4.3 (VL)",  "",           "",      ""],
    ["",     "R41 2mΩ", "Q10.3 (VL)", "",           "",      ""],
    ["",     "R62 2mΩ", "Q16.3 (VL)", "",           "",      ""],
    ["",     "R83 2mΩ", "Q22.3 (VL)", "",           "",      ""],
]

sensor_comm = [
    ["功能", "MCU 引脚", "网络标签", "说明"],
    ["BMI270 NSS / CS",  "PA4",  "IMU_NSS",  "SPI1 片选"],
    ["BMI270 SCK",       "PA5",  "IMU_SCK",  "SPI1 时钟"],
    ["BMI270 MISO / SDO", "PA6",  "IMU_MISO", "SPI1 主入从出"],
    ["BMI270 MOSI / SDI", "PA7",  "IMU_MOSI", "SPI1 主出从入"],
    ["BMI270 INT1",      "PD1",  "IMU_INT",  "外部中断"],
    ["SPL06 SCL",        "PB6",  "I2C1_SCL", "I2C1 时钟 (共用总线)"],
    ["SPL06 SDA",        "PB7",  "I2C1_SDA", "I2C1 数据 (共用总线)"],
    ["LoRa SCK",         "PC10", "LORA_SCK",  "SPI3 时钟"],
    ["LoRa MISO",        "PC11", "LORA_MISO", "SPI3 主入从出"],
    ["LoRa MOSI",        "PC12", "LORA_MOSI", "SPI3 主出从入"],
    ["LoRa NSS",         "PD6",  "LORA_NSS",  "SPI3 片选"],
    ["LoRa DIO0",        "PD7",  "LORA_DIO0", "外部中断"],
    ["LoRa NRESET",      "PD4",  "LORA_NRST", "模块复位"],
    ["Flash CS",         "PB12", "FLASH_CS",  "SPI2 片选"],
    ["Flash SCK",        "PB13", "FLASH_SCK", "SPI2 时钟"],
    ["Flash MISO / SO",  "PC2",  "FLASH_MISO","SPI2 主入从出"],
    ["Flash MOSI / SI",  "PC3",  "FLASH_MOSI","SPI2 主出从入"],
    ["接收机 RX",        "PD9",  "RC_RX",    "USART3_RX"],
    ["接收机 TX",        "PD8",  "RC_TX",    "USART3_TX"],
]

adc_periph = [
    ["信号", "MCU 引脚", "网络标签", "ADC 通道"],
    ["ESC1 U 相电流", "PC0", "IADC1_U",    "ADC123_IN10"],
    ["ESC1 V 相电流", "PC1", "IADC1_V",    "ADC123_IN11"],
    ["ESC2 U 相电流", "PC4", "IADC2_U",    "ADC123_IN14"],
    ["ESC2 V 相电流", "PC5", "IADC2_V",    "ADC123_IN15"],
    ["ESC3 U 相电流", "PA2", "IADC3_U",    "ADC123_IN2"],
    ["ESC3 V 相电流", "PA3", "IADC3_V",    "ADC123_IN3"],
    ["ESC4 U 相电流", "PA0", "IADC4_U",    "ADC123_IN0"],
    ["ESC4 V 相电流", "PA1", "IADC4_V",    "ADC123_IN1"],
    ["VBAT 电压采样", "PC0", "VBAT_SENSE", "与 ESC1_U 分时复用"],
    ["蜂鸣器控制",   "PE0", "BEEP",       "GPIO, 低电平导通"],
    ["LED 红",       "PD2", "LED_RED",    "GPIO, 低电平亮"],
    ["LED 蓝",       "PD3", "LED_BLUE",   "GPIO, 低电平亮"],
    ["SWDIO",        "PA13","SWDIO",      "调试接口"],
    ["SWCLK",        "PA14","SWCLK",      "调试接口"],
    ["HSE IN",       "PH0", "OSC_IN",     "8MHz 晶振"],
    ["HSE OUT",      "PH1", "OSC_OUT",    "8MHz 晶振"],
    ["LSE IN",       "PC14","OSC32_IN",   "32.768kHz (可选)"],
    ["LSE OUT",      "PC15","OSC32_OUT",  "32.768kHz (可选)"],
]

change_data = [
    ["位置", "原接法", "改为", "新标签", "原因"],
    ["气压计 SCL", "PB10 (I2C2)", "PB6  (I2C1)", "I2C1_SCL",  "释放 TIM2 + 统一 I2C1 总线"],
    ["气压计 SDA", "PB11 (I2C2)", "PB7  (I2C1)", "I2C1_SDA",  "释放 TIM2 + 统一 I2C1 总线"],
    ["LoRa SCK",   "PB3 (SPI3)",  "PC10 (SPI3)", "LORA_SCK",  "释放 PB3 → 给 ESC3 HIN2"],
    ["LoRa MISO",  "PB4 (SPI3)",  "PC11 (SPI3)", "LORA_MISO", "释放 PB4"],
    ["LoRa MOSI",  "PB5 (SPI3)",  "PC12 (SPI3)", "LORA_MOSI", "释放 PB5"],
    ["LoRa NSS",   "PC4",         "PD6",         "LORA_NSS",  "释放 ADC 通道 PC4"],
    ["LoRa DIO0",  "PC5",         "PD7",         "LORA_DIO0", "释放 ADC 通道 PC5"],
    ["Flash MISO", "PB14",        "PC2",         "FLASH_MISO","释放 TIM8_CH2N"],
    ["Flash MOSI", "PB15",        "PC3",         "FLASH_MOSI","释放 TIM8_CH3N"],
    ["接收机 RX",  "—",           "PD9",         "RC_RX",     "新增 USART3_RX"],
    ["接收机 TX",  "—",           "PD8",         "RC_TX",     "新增 USART3_TX"],
    ["蜂鸣器",     "—",           "PE0",         "BEEP",      "新增 GPIO"],
    ["LED 红",     "—",           "PD2",         "LED_RED",   "新增 GPIO"],
    ["LED 蓝",     "—",           "PD3",         "LED_BLUE",  "新增 GPIO"],
    ["VBAT_SENSE", "—",           "PC0",         "VBAT_SENSE","新增 ADC"],
]

unused = "PA11 PA12 PB2 PB4 PB5 PC13 PD0 PD5 PD10 PD11 PE1 PE2 PE3 PE4 PE7 PE9 PE10 PE11 PE12 PE13 PE14 PE15"

# ═══════════════════════════════════════════
# 生成
# ═══════════════════════════════════════════
def build():
    out = os.path.join(BASE, 'CoreFly_F2_引脚分配表.pdf')
    doc = SimpleDocTemplate(out, pagesize=A4,
        leftMargin=12*mm, rightMargin=12*mm,
        topMargin=14*mm, bottomMargin=14*mm,
        title='CoreFly F2 MCU 引脚分配表')

    story = []
    story.append(Paragraph('<b>CoreFly F2 无人机飞控电调一体板</b>', STYLES['title']))
    story.append(Paragraph('MCU 引脚分配总表 (含网络标签) — STM32F407VET6 LQFP-100', STYLES['subtitle']))
    story.append(SP(6))

    # ═══ 一、ESC PWM ═══
    story.append(H1('一、4路 ESC PWM 输入 (MCU → 100Ω → FD6288T)'))
    story.append(T(esc_pwm, [36, 48, 40, 52, 50, 48], net_cols={4, 5}))
    story.append(SP(4))
    story.append(B('<b>注：</b>MCU侧标签(M1_HIN1...)放在100Ω电阻的 MCU 端；FD侧标签(HIN1-1...)放在电阻到 FD6288T 引脚端。每条PWM线上串一个100Ω电阻。'))

    story.append(PageBreak())

    # ═══ 二、ESC 栅极驱动 ═══
    story.append(H1('二、FD6288T 栅极驱动输出 & 半桥中点'))
    story.append(T(esc_gate, [36, 50, 82, 48, 48], net_cols={3, 4}))
    story.append(SP(4))
    story.append(B('<b>注：</b>每个 MOSFET 栅极串 10Ω 电阻，栅-源之间并 10kΩ 下拉电阻。半桥中点 = 上桥源极 + 下桥漏极 + VS 引脚 + 电机端子，同一网络。'))

    story.append(SP(5))

    # ═══ 三、自举电容 ═══
    story.append(H1('三、自举电容 (2.2µF 25V X7R ×12)'))
    story.append(T(esc_bootstrap, [36, 48, 48, 68, 48, 48], net_cols={4, 5}))

    story.append(PageBreak())

    # ═══ 四、供电 ═══
    story.append(H1('四、FD6288T 供电 & 采样电阻'))
    story.append(T(esc_power, [36, 48, 48, 90, 48], net_cols={4}))
    story.append(SP(5))
    story.append(T(esc_sense, [36, 62, 60, 62, 32, 68], net_cols={5}))

    story.append(SP(5))

    # ═══ 五、传感器 & 通信 ═══
    story.append(H1('五、传感器 & 通信模块'))
    story.append(T(sensor_comm, [80, 48, 66, 90], net_cols={2}))

    story.append(SP(6))

    # ═══ 六、ADC & 外设 ═══
    story.append(H1('六、ADC 采样 & 外设'))
    story.append(T(adc_periph, [76, 44, 68, 86], net_cols={2}))

    story.append(PageBreak())

    story.append(H1('七、未用引脚 (预留)'))
    story.append(B(unused))
    story.append(SP(6))

    # ═══ 八、修改对照 ═══
    story.append(H1('八、原理图修改对照表'))
    story.append(T(change_data, [58, 64, 64, 60, 80], net_cols={3}))

    story.append(SP(8))
    story.append(Paragraph('文档版本 V2.0 | 2026-05-18 | 含网络标签', STYLES['note']))

    doc.build(story)
    print('Done:', out)

if __name__ == '__main__':
    build()
