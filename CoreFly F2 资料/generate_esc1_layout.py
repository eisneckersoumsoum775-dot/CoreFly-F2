#!/usr/bin/env python3
"""生成 ESC1 布局示意图 (80x80 板子左上角)"""
import os
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle)
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

try:
    pdfmetrics.registerFont(TTFont('YaHei', 'C:/Windows/Fonts/msyh.ttc', subfontIndex=0))
    CN = 'YaHei'
except:
    CN = 'Helvetica'

BASE = r"F:\Zhuomian\CoreFly F2\CoreFly F2 资料"

def P(text, style_name='td'):
    return Paragraph(text, styles[style_name])

styles = {
    'title': ParagraphStyle('t', fontName=CN, fontSize=18, leading=24, alignment=TA_CENTER, textColor=colors.HexColor('#1a1a2e')),
    'h2': ParagraphStyle('h2', fontName=CN, fontSize=12, leading=16, textColor=colors.HexColor('#0f3460')),
    'td': ParagraphStyle('td', fontName=CN, fontSize=7, leading=9),
    'th': ParagraphStyle('th', fontName=CN, fontSize=7.5, leading=10, alignment=TA_CENTER, textColor=colors.white),
    'note': ParagraphStyle('note', fontName=CN, fontSize=8, leading=12, textColor=colors.HexColor('#888')),
}

BLUE = colors.HexColor('#0f3460')
WHITE = colors.white
GRAY = colors.HexColor('#f0f0f0')
GRID = colors.HexColor('#ccc')

# ── ESC1 元件位置 (75x75 区域内) ──
# 板子 80x80, ESC1 占据左上约 55x30mm 区域

esc1_chart = [
    ["Y\\X", "5",  "10", "15", "20", "25", "30", "35", "40", "45", "50", "55", "60", "65"],
    ["75",   "","", "",   "",   "",   "",   "",   "",   "",   "",   "",   "",   ""],
    ["73",   "","", "",   "",   "",   "",   "",   "",   "",   "",   "",   "",   ""],
    ["71",   "",  "", "",  "","","","","","","","",""],
    ["69",   "P1", "","","","","","","","","","","",""],
    ["67",   "",  "", "",   "Q1\nUH","","","","","","","","",""],
    ["65",   "P2", "","","","","","","","","","","",""],
    ["63",   "",  "", "",   "Q2\nUL","","","","","","","","",""],
    ["61",   "P3", "","","","","","","","","","","",""],
    ["59",   "",  "", "",  "",  "","","","","","","","",""],
    ["57",   "",  "", "",  "R19\n2mΩ","","","","","","","","",""],
    ["55",   "",  "", "",  "",  "","","","","","","U4\nFD6288T\n(TSSOP-20)","",""],
    ["53",   "",  "", "",  "",  "","","","","","","","",""],
    ["51",   "",  "", "U9\nTPA1812","","","","","","","","",""],
    ["49",   "",  "", "",  "",  "","","","","","","","",""],
    ["47",   "",  "", "",  "",  "","","","","","","","",""],
]

# Alternate version: use a big ASCII diagram embedded in the PDF instead

def make_table():
    # Create a table representing the ESC1 layout
    ESC1_DATA = [
        ["元件", "编号", "封装", "X(mm)", "Y(mm)", "备注"],
        ["电机U焊盘", "P1", "ϕ3.5孔", "5", "70", "朝板边"],
        ["电机V焊盘", "P2", "ϕ3.5孔", "5", "64", "间距6mm"],
        ["电机W焊盘", "P3", "ϕ3.5孔", "5", "58", ""],
        ["上桥 U",  "Q1", "TO-252", "22", "67", "漏极↑→12V"],
        ["下桥 U",  "Q2", "TO-252", "22", "58", "源极↓→R19"],
        ["上桥 V",  "Q3", "TO-252", "32", "67", ""],
        ["下桥 V",  "Q4", "TO-252", "32", "58", ""],
        ["上桥 W",  "Q5", "TO-252", "42", "67", ""],
        ["下桥 W",  "Q6", "TO-252", "42", "58", ""],
        ["栅极驱动", "U4", "TSSOP-20", "52", "58", "HIN朝左,HO朝右"],
        ["自举电容U", "C26", "0805", "55", "70", "VB1→VS1"],
        ["自举电容V", "C25", "0805", "55", "67", "VB2→VS2"],
        ["自举电容W", "C24", "0805", "55", "64", "VB3→VS3"],
        ["采样电阻U", "R19", "2512", "22", "52", "Q2源极→GND"],
        ["采样电阻V", "R20", "2512", "32", "52", ""],
        ["采样电阻W", "R21", "2512", "42", "52", ""],
        ["电流运放U", "U9", "SOP-8", "50", "47", "TPA1812 U相"],
        ["电流运放V", "U7", "SOP-8", "62", "47", "TPA1812 V相"],
        ["VCC去耦", "C22/23", "0603", "54", "54", "100nF+10μF"],
        ["栅极电阻", "R13~18", "0603×6", "60", "55~72", "10Ω, 紧贴MOS栅极"],
        ["下拉电阻", "R22~27", "0603×6", "60", "52~72", "10kΩ, 栅→源"],
    ]
    return Table(
        [[P(r[0], 'th'), P(r[1], 'th'), P(r[2], 'th'), P(r[3], 'th'), P(r[4], 'th'), P(r[5], 'th')] if i == 0
         else [P(r[0]), P(r[1]), P(r[2]), P(r[3]), P(r[4]), P(r[5])] for i, r in enumerate(ESC1_DATA)],
        colWidths=[72, 48, 52, 48, 48, 110],
        repeatRows=1
    )

def build():
    out = os.path.join(BASE, 'ESC1_布局示意图.pdf')
    doc = SimpleDocTemplate(out, pagesize=A4,
        leftMargin=10*mm, rightMargin=10*mm, topMargin=14*mm, bottomMargin=14*mm,
        title='ESC1 布局示意图')

    story = []
    story.append(P('CoreFly F2 — ESC1 单路布局坐标', 'title'))
    story.append(P('板子 80×80mm, ESC1 占据左上角约 55×30mm', 'note'))
    story.append(Spacer(1, 6*mm))

    # ── Section 1: Component table ──
    story.append(P('一、元件位置坐标表', 'h2'))
    story.append(Spacer(1, 3*mm))
    t = make_table()
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), BLUE),
        ('TEXTCOLOR', (0, 0), (-1, 0), WHITE),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('GRID', (0, 0), (-1, -1), 0.4, GRID),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [WHITE, GRAY]),
        ('TOPPADDING', (0, 0), (-1, -1), 3),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 3),
        ('LEFTPADDING', (0, 0), (-1, -1), 3),
        ('RIGHTPADDING', (0, 0), (-1, -1), 3),
    ]))
    story.append(t)
    story.append(Spacer(1, 8*mm))

    # ── Section 2: ASCII layout ──
    story.append(P('二、相对位置简图 (Y轴朝上)', 'h2'))
    story.append(Spacer(1, 3*mm))

    layout_ascii = """
  mm  0    5    10   15   20   25   30   35   40   45   50   55   60   65   70   75
  ──  ┌────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┬────┐
  75  │    │    │        电机区域        │                    │      Q1 UH      │
      │    │    │  P1 ●── P2 ●── P3 ●     │                    │ ◎ TO-252       │
  70  │    │    │  U   V   W              │                    │      Y=67      │
      │    │    │  焊盘上锡               │                    │                │
  65  ├────┼────┤  ┌─────────────────┐    │       Q3 VH        │    Q5 WH        │
      │    │    │  │ 半桥中点铜皮   │    │    ◎ Y=67       │  ◎ Y=67      │
  60  │    │    │  │ SU-1 → P1      │    │                    │  FD6288T        │
      │    │    │  │ 约 15mm 宽     │    │  Q2 UL  Q4 VL  Q6 WL│◎U4 Y=58       │
  55  │    │    │  └─────────────────┘    │  ◎Y=58  ◎Y=58  ◎Y=58│TSSOP-20       │
      │    │    │                         │  │      │      │    │                │
  50  ├────┼────┤                         │  R19    R20    R21 │                │
      │    │    │   采样电阻 (2512)       │  2mΩ    2mΩ    2mΩ │                │
  45  │         │   下端铺铜→GND         │  Y=52   Y=52   Y=52 │  U9 TPA1812    │
      │    ≈2mm │                         │                    │◎SOP-8 Y=47    │
  40  │ GND铺铜│                         │                    │                │
      │         │                         │                    │  U7 TPA1812    │
  35  └─────────┘                         │                    │◎SOP-8 Y=47    │
  ──                                      └────────────────────┘                ──
"""

    mono_style = ParagraphStyle('mono', fontName='Helvetica', fontSize=6, leading=7)
    for line in layout_ascii.strip().split('\n'):
        story.append(Paragraph(line, mono_style))

    story.append(Spacer(1, 8*mm))

    # ── Section 3: 布线要点 ──
    story.append(P('三、关键布线顺序 (第4层 Bottom)', 'h2'))
    story.append(Spacer(1, 2*mm))

    routing = [
        "① 电机焊盘 ← 半桥中点铜皮 (Q2漏极+Q1源极) — 15mm宽, 15mm长",
        "② Q1/Q3/Q5 漏极 → 过孔阵 → 第3层12V平面 — 每个漏极打6~8个0.3mm过孔",
        "③ Q2/Q4/Q6 源极 → 采样电阻上端 — 短粗铜皮",
        "④ 采样电阻下端 → 过孔 → GND平面(第2层)",
        "⑤ U4(HO1) → 10Ω → Q1栅极  <10mm",
        "⑥ U4(LO1) → 10Ω → Q2栅极  <10mm",
        "⑦ U4(VS1) → 半桥中点 ← 同一铜皮",
        "⑧ 采样高端 → 100Ω → TPA1812 IN+ / GND → 100Ω → IN- — 差分对",
        "⑨ TPA1812 OUT → 100Ω → 过孔 → 第1层 → MCU ADC",
        "⑩ 最后走 PWM 信号: MCU → 过孔 → 第4层 → 100Ω → U4 HIN/LIN",
    ]
    for r in routing:
        story.append(Paragraph(r, ParagraphStyle('route', fontName=CN, fontSize=9, leading=14, leftIndent=8*mm, spaceAfter=1*mm)))

    story.append(Spacer(1, 5*mm))
    story.append(P('文档版本 V1.0 | ESC1 布局参考', 'note'))
    doc.build(story)
    print('Done:', out)

if __name__ == '__main__':
    build()
