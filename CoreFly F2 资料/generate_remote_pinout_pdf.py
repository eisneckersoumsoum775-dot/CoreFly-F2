#!/usr/bin/env python3
"""生成遥控器引脚分配表 PDF"""
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

def S(name, **kw):
    d = dict(fontName=CN, fontSize=9, leading=14, textColor=colors.HexColor('#333'))
    d.update(kw)
    return ParagraphStyle(name, **d)

STYLES = {
    'title':   S('t', fontSize=20, leading=28, alignment=TA_CENTER, textColor=DARK),
    'subtitle':S('st', fontSize=11, leading=16, alignment=TA_CENTER, textColor=colors.HexColor('#666')),
    'h1':      S('h1', fontSize=13, leading=20, textColor=BLUE),
    'body':    S('body', fontSize=9, leading=14),
    'note':    S('note', fontSize=8, leading=12, textColor=colors.HexColor('#888')),
    'td':      S('td', fontSize=7.2, leading=10.5),
    'tdc':     S('tdc', fontSize=7.2, leading=10.5, alignment=TA_CENTER),
    'th':      S('th', fontSize=7.5, leading=11, alignment=TA_CENTER, textColor=WHITE),
}

def T(rows, col_widths, header_bg=BLUE):
    data = []
    for i, row in enumerate(rows):
        style = 'th' if i == 0 else 'tdc'
        data.append([Paragraph(str(c), STYLES[style]) for c in row])
    t = Table(data, colWidths=col_widths, repeatRows=1)
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), header_bg),
        ('TEXTCOLOR', (0, 0), (-1, 0), WHITE),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('GRID', (0, 0), (-1, -1), 0.4, GRID),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [WHITE, GRAY]),
        ('TOPPADDING', (0, 0), (-1, -1), 3.5),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3.5),
        ('LEFTPADDING', (0, 0), (-1, -1), 3),
        ('RIGHTPADDING', (0, 0), (-1, -1), 3),
    ]))
    return t

def H1(text): return Paragraph(f'<b>{text}</b>', STYLES['h1'])
def B(text):  return Paragraph(text, STYLES['body'])
def SP(mm_val=3): return Spacer(1, mm_val*mm)

# ── 数据 ──

lcd = [
    ["LCD 转接板", "MCU 引脚", "功能", "标签"],
    ["VCC (3.3V)", "+3.3V", "供电", "LCD_VCC"],
    ["GND", "GND", "地", "LCD_GND"],
    ["SCK", "PA5", "SPI1_SCK", "LCD_SCK"],
    ["SDI (MOSI)", "PA7", "SPI1_MOSI", "LCD_MOSI"],
    ["SDO (MISO)", "PA6", "SPI1_MISO", "LCD_MISO"],
    ["CS", "PA4", "GPIO_Output", "LCD_CS"],
    ["DC (A0)", "PD2", "GPIO_Output", "LCD_DC"],
    ["RESET", "PD3", "GPIO_Output", "LCD_RST"],
    ["BL (背光)", "PD4", "TIM2_CH3 PWM", "LCD_BL"],
]

joystick = [
    ["摇杆", "引脚", "MCU", "功能", "标签"],
    ["摇杆1左", "VRx", "PA0", "ADC1_IN0", "JOY1_X"],
    ["", "VRy", "PA1", "ADC1_IN1", "JOY1_Y"],
    ["", "SW", "PE0", "GPIO_Input", "JOY1_SW"],
    ["", "备用", "PE1", "GPIO", "JOY1_AUX"],
    ["摇杆2右", "VRx", "PA2", "ADC1_IN2", "JOY2_X"],
    ["", "VRy", "PA3", "ADC1_IN3", "JOY2_Y"],
    ["", "SW", "PE2", "GPIO_Input", "JOY2_SW"],
    ["", "备用", "PE3", "GPIO", "JOY2_AUX"],
]

buttons = [
    ["按键/开关", "MCU", "配置", "标签", "接法"],
    ["解锁 ARM", "PE4", "GPIO_Input", "BTN_ARM", "→GND, 内上拉"],
    ["模式 MODE", "PE5", "GPIO_Input", "BTN_MODE", "→GND, 内上拉"],
    ["上 UP", "PE6", "GPIO_Input", "BTN_UP", "→GND, 内上拉"],
    ["下 DOWN", "PB10", "GPIO_Input", "BTN_DOWN", "→GND, 内上拉"],
    ["确认 OK", "PB11", "GPIO_Input", "BTN_OK", "→GND, 内上拉"],
    ["拨动 SWA", "PB12", "GPIO_Input", "SW_A", "10kΩ↑, ON→GND"],
    ["拨动 SWB", "PB13", "GPIO_Input", "SW_B", "10kΩ↑, ON→GND"],
    ["拨动 SWC", "PB14", "GPIO_Input", "SW_C", "10kΩ↑, ON→GND"],
    ["拨动 SWD", "PB15", "GPIO_Input", "SW_D", "10kΩ↑, ON→GND"],
]

lora = [
    ["Ra-01SC", "MCU", "功能", "标签"],
    ["SCK", "PC10", "SPI3_SCK", "LORA_SCK"],
    ["MISO", "PC11", "SPI3_MISO", "LORA_MISO"],
    ["MOSI", "PC12", "SPI3_MOSI", "LORA_MOSI"],
    ["NSS", "PD6", "GPIO_Output", "LORA_NSS"],
    ["DIO0", "PD7", "GPIO_EXTI7", "LORA_DIO0"],
    ["NRESET", "PD5", "GPIO_Output", "LORA_NRST"],
    ["TXEN", "PD10", "GPIO_Output", "LORA_TXEN"],
    ["RXEN", "PD0", "GPIO_Output", "LORA_RXEN"],
    ["BUSY", "PD1", "GPIO_Input", "LORA_BUSY"],
]

adc_periph = [
    ["功能", "MCU", "配置", "标签"],
    ["VBAT 采样", "PC0", "ADC1_IN10", "VBAT_SENSE"],
    ["蜂鸣器", "PE8", "GPIO_Output", "BEEP"],
    ["LED 红", "PD12", "GPIO_Output", "LED_RED"],
    ["LED 绿", "PD13", "GPIO_Output", "LED_GREEN"],
    ["LED 蓝", "PD14", "GPIO_Output", "LED_BLUE"],
    ["电池低压灯", "PD15", "GPIO_Output", "LED_BAT"],
    ["调试 TX", "PD8", "USART3_TX", "DBG_TX"],
    ["调试 RX", "PD9", "USART3_RX", "DBG_RX"],
]

sys_pins = [
    ["功能", "MCU", "配置"],
    ["SWDIO", "PA13", "SYS_SWDIO"],
    ["SWCLK", "PA14", "SYS_SWCLK"],
    ["HSE IN", "PH0", "RCC_OSC_IN"],
    ["HSE OUT", "PH1", "RCC_OSC_OUT"],
]

quick = [
    ["MCU 引脚", "标签", "功能"],
    ["PA0", "JOY1_X", "ADC1_IN0"],
    ["PA1", "JOY1_Y", "ADC1_IN1"],
    ["PA2", "JOY2_X", "ADC1_IN2"],
    ["PA3", "JOY2_Y", "ADC1_IN3"],
    ["PA4", "LCD_CS", "GPIO_Output"],
    ["PA5", "LCD_SCK", "SPI1_SCK"],
    ["PA6", "LCD_MISO", "SPI1_MISO"],
    ["PA7", "LCD_MOSI", "SPI1_MOSI"],
    ["PA13", "SYS_SWDIO", ""],
    ["PA14", "SYS_SWCLK", ""],
    ["PB10", "BTN_DOWN", "GPIO_Input"],
    ["PB11", "BTN_OK", "GPIO_Input"],
    ["PB12", "SW_A", "GPIO_Input"],
    ["PB13", "SW_B", "GPIO_Input"],
    ["PB14", "SW_C", "GPIO_Input"],
    ["PB15", "SW_D", "GPIO_Input"],
    ["PC0", "VBAT_SENSE", "ADC1_IN10"],
    ["PC10", "LORA_SCK", "SPI3_SCK"],
    ["PC11", "LORA_MISO", "SPI3_MISO"],
    ["PC12", "LORA_MOSI", "SPI3_MOSI"],
    ["PD0", "LORA_RXEN", "GPIO_Output"],
    ["PD1", "LORA_BUSY", "GPIO_Input"],
    ["PD2", "LCD_DC", "GPIO_Output"],
    ["PD3", "LCD_RST", "GPIO_Output"],
    ["PD4", "LCD_BL", "TIM2_CH3"],
    ["PD5", "LORA_NRST", "GPIO_Output"],
    ["PD6", "LORA_NSS", "GPIO_Output"],
    ["PD7", "LORA_DIO0", "GPIO_EXTI7"],
    ["PD8", "DBG_TX", "USART3_TX"],
    ["PD9", "DBG_RX", "USART3_RX"],
    ["PD10", "LORA_TXEN", "GPIO_Output"],
    ["PD12", "LED_RED", "GPIO_Output"],
    ["PD13", "LED_GREEN", "GPIO_Output"],
    ["PD14", "LED_BLUE", "GPIO_Output"],
    ["PD15", "LED_BAT", "GPIO_Output"],
    ["PE0", "JOY1_SW", "GPIO_Input"],
    ["PE1", "JOY1_AUX", "预留"],
    ["PE2", "JOY2_SW", "GPIO_Input"],
    ["PE3", "JOY2_AUX", "预留"],
    ["PE4", "BTN_ARM", "GPIO_Input"],
    ["PE5", "BTN_MODE", "GPIO_Input"],
    ["PE6", "BTN_UP", "GPIO_Input"],
    ["PE8", "BEEP", "GPIO_Output"],
    ["PH0", "RCC_OSC_IN", ""],
    ["PH1", "RCC_OSC_OUT", ""],
]

def build():
    out = os.path.join(BASE, '遥控器_引脚分配.pdf')
    doc = SimpleDocTemplate(out, pagesize=A4,
        leftMargin=14*mm, rightMargin=14*mm, topMargin=14*mm, bottomMargin=14*mm,
        title='遥控器 MCU 引脚分配表')

    story = []
    story.append(Paragraph('<b>遥控器 MCU 引脚分配表</b>', STYLES['title']))
    story.append(Paragraph('STM32F407VET6 (LQFP-100) — 摇杆×2 + LCD + LoRa + 按键', STYLES['subtitle']))
    story.append(SP(6))

    story.append(H1('一、LCD (2.8寸 18Pin→SPI) — SPI1'))
    story.append(T(lcd, [90, 55, 65, 55]))
    story.append(SP(5))

    story.append(H1('二、摇杆 ×2'))
    story.append(T(joystick, [48, 38, 40, 68, 55]))
    story.append(SP(5))

    story.append(H1('三、按键 & 拨动开关'))
    story.append(T(buttons, [72, 40, 60, 55, 75]))
    story.append(SP(5))

    story.append(PageBreak())

    story.append(H1('四、LoRa — SPI3'))
    story.append(T(lora, [65, 55, 72, 60]))
    story.append(SP(5))

    story.append(H1('五、ADC / 外设 / 调试'))
    story.append(T(adc_periph, [72, 52, 68, 60]))
    story.append(SP(5))

    story.append(H1('六、系统'))
    story.append(T(sys_pins, [65, 55, 60]))
    story.append(SP(8))

    story.append(H1('七、CubeMX 快速录入 (完整47脚)'))
    story.append(T(quick, [48, 72, 100]))
    story.append(SP(6))
    story.append(Paragraph('文档版本 V1.0 | 2026-05-20', STYLES['note']))

    doc.build(story)
    print('Done:', out)

if __name__ == '__main__':
    build()
