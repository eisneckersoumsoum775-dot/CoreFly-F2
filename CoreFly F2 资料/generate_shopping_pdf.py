#!/usr/bin/env python3
"""生成购物清单 PDF"""
import os
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Table,
                                 TableStyle)
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
DARK = colors.HexColor('#1a1a2e')
BLUE = colors.HexColor('#0f3460')
RED  = colors.HexColor('#e74c3c')
GREEN = colors.HexColor('#27ae60')
WHITE = colors.white
GRAY = colors.HexColor('#f5f5f5')
GRID = colors.HexColor('#cccccc')

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
    'have':    S('have', fontSize=7.5, leading=10.5, textColor=GREEN),
    'td':      S('td', fontSize=7.5, leading=10.5),
    'tdc':     S('tdc', fontSize=7.5, leading=10.5, alignment=TA_CENTER),
    'th':      S('th', fontSize=7.5, leading=11, alignment=TA_CENTER, textColor=WHITE),
}

def T(rows, col_widths, header_bg=BLUE):
    data = []
    for i, row in enumerate(rows):
        style = STYLES['th'] if i == 0 else STYLES['tdc']
        data.append([Paragraph(str(c), style) for c in row])
    t = Table(data, colWidths=col_widths, repeatRows=1)
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), header_bg),
        ('TEXTCOLOR', (0, 0), (-1, 0), WHITE),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('GRID', (0, 0), (-1, -1), 0.4, GRID),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [WHITE, GRAY]),
        ('TOPPADDING', (0, 0), (-1, -1), 3),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
        ('LEFTPADDING', (0, 0), (-1, -1), 3),
        ('RIGHTPADDING', (0, 0), (-1, -1), 3),
    ]))
    return t

def H1(text): return Paragraph(f'<b>{text}</b>', STYLES['h1'])
def B(text):  return Paragraph(text, STYLES['body'])
def P(text, style='body'): return Paragraph(text, STYLES[style])
def SP(mm_val=3): return Spacer(1, mm_val*mm)

chips = [
    ["元器件", "数量", "LCSC 编号", "单价", "小计"],
    ["FD6288T 栅极驱动", "4", "C5804616", "~¥4", "¥16"],
    ["TPA1812-SO1R 电流运放", "8", "C45073132", "~¥1.5", "¥12"],
    ["LMV321-TR VREF跟随", "1", "C362273", "~¥0.5", "¥0.5"],
    ["SY8120B1ABC 12V→5V", "1", "C88474", "~¥1.5", "¥1.5"],
    ["NCE3080K MOSFET", "24", "C108901", "~¥1.5", "¥36"],
    ["S8050 蜂鸣器驱动", "1", "C3014350", "~¥0.2", "¥0.2"],
    ["", "", "", "芯片小计", "¥66"],
]

modules = [
    ["元器件", "数量", "来源", "单价", "小计"],
    ["MPU6050 贴片模块", "1", "淘宝", "~¥9", "¥9"],
    ["SPL06-001 气压计", "1", "淘宝", "~¥5", "¥5"],
    ["", "", "", "模块小计", "¥14"],
]

connectors = [
    ["元器件", "数量", "LCSC 编号", "单价", "小计"],
    ["2.54-1×1P 排针 (电机/电源)", "14", "C81276", "¥0.1", "¥1.4"],
    ["2.54-1×3P 排针 (接收机)", "1", "C5116482", "¥0.3", "¥0.3"],
    ["2.54-1×4P 排针 (SWD)", "1", "C2691448", "¥0.3", "¥0.3"],
    ["蜂鸣器 MLT-8530", "1", "C94599", "~¥2", "¥2"],
    ["轻触开关 TS-1101-C-W", "1", "C318938", "¥0.2", "¥0.2"],
    ["", "", "", "连接器小计", "¥4"],
]

specials = [
    ["元器件", "规格", "数量", "说明", "小计"],
    ["采样电阻 2mΩ", "2512 2W", "12", "30A 电流检测", "¥12"],
    ["自举电容 2.2µF", "0805 25V X7R", "12", "VBx→VSx", "¥2"],
    ["10µF 0805 25V", "0805 25V", "1", "C6 +12V去耦", "¥0.3"],
    ["22µF 0805 10V", "0805 10V", "1", "C21 5V去耦", "¥0.3"],
    ["", "", "", "特殊件小计", "¥15"],
]

resistors = [
    ["阻值", "封装", "BOM需", "已有约", "买", "小计"],
    ["100Ω", "0603", "48", "~10", "50个", "¥1"],
    ["10Ω", "0603", "24", "~10", "30个", "¥0.5"],
    ["10K", "0603", "29", "~10", "30个", "¥0.5"],
    ["1K", "0603", "4", "~5", "0 (够用)", "-"],
    ["100K", "0603", "1", "~5", "0 (够用)", "-"],
    ["", "", "", "", "电阻小计", "¥2"],
]

have_list = [
    ["已有元器件", "数量", "备注"],
    ["STM32F407VET6", "1", "MCU ✓"],
    ["Ra-01SC (LoRa)", "1", "通信 ✓"],
    ["8MHz 晶振 X49SM", "1", "HSE ✓"],
    ["RT9193-3.3 (LDO)", "1", "已有 ✓"],
    ["CKCS4030-4.7uH", "1", "功率电感 ✓"],
    ["BAT54S (SOT-23)", "1", "双肖特基 ✓"],
    ["LED 红 0805", "1", "✓"],
    ["LED 蓝 0603", "1", "✓"],
    ["100nF/22pF/1µF 0603", "各若干", "电容 ✓"],
    ["1M/15K/110K 0603", "各若干", "电阻 ✓"],
]

total = [
    ["类别", "金额"],
    ["芯片 6 类", "¥66"],
    ["模块 2 类", "¥14"],
    ["连接器/杂件", "¥4"],
    ["采样电阻 + 电容", "¥15"],
    ["电阻补量", "¥2"],
    ["总计", "¥101"],
]

def build():
    out = os.path.join(BASE, 'CoreFly_F2_购物清单.pdf')
    doc = SimpleDocTemplate(out, pagesize=A4,
        leftMargin=14*mm, rightMargin=14*mm, topMargin=14*mm, bottomMargin=14*mm,
        title='CoreFly F2 购物清单')
    story = []
    story.append(P('<b>CoreFly F2 飞控电调一体板</b>', 'title'))
    story.append(P('元器件采购清单 — BOM vs 已有库存对比', 'subtitle'))
    story.append(SP(6))

    story.append(H1('一、需要购买 — 芯片'))
    story.append(T(chips, [85, 40, 65, 40, 40]))
    story.append(SP(5))
    story.append(H1('二、需要购买 — 模块'))
    story.append(T(modules, [85, 40, 65, 40, 40]))
    story.append(SP(5))
    story.append(H1('三、需要购买 — 连接器 / 杂件'))
    story.append(T(connectors, [85, 40, 65, 40, 40]))
    story.append(SP(5))
    story.append(H1('四、需要购买 — 功率电阻 / 特殊电容'))
    story.append(T(specials, [80, 65, 35, 65, 35]))
    story.append(SP(2))
    story.append(P('注意: 你已有 2.2µF / 10µF / 22µF 是 0603 封装，此处需要 0805 耐压 25V。'))
    story.append(SP(5))
    story.append(H1('五、需要购买 — 常用电阻补量'))
    story.append(T(resistors, [42, 42, 42, 42, 52, 42]))
    story.append(SP(8))

    story.append(H1('六、已有库存 — 不用买'))
    story.append(T(have_list, [100, 45, 70]))
    story.append(SP(8))

    story.append(H1('七、总价汇总'))
    story.append(T(total, [140, 60]))

    story.append(SP(6))
    story.append(P('文档版本 V1.0 | 2026-05-20', 'note'))
    doc.build(story)
    print('Done:', out)

if __name__ == '__main__':
    build()
