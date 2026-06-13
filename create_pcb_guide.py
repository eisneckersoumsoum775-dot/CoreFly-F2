# -*- coding: utf-8 -*-
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor
from reportlab.lib.units import mm
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle
from reportlab.platypus.flowables import HRFlowable
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

# ── Fonts ──
pdfmetrics.registerFont(TTFont('SimHei', 'C:/Windows/Fonts/simhei.ttf'))
pdfmetrics.registerFont(TTFont('SimFang', 'C:/Windows/Fonts/simfang.ttf'))
FONT = 'SimHei'
FONT_BODY = 'SimFang'

output_path = r"F:\Zhuomian\CoreFly F2\CoreFly F2 资料\CoreFly_F2_PCB布局布线设计指南.pdf"

doc = SimpleDocTemplate(output_path, pagesize=A4,
    topMargin=20*mm, bottomMargin=20*mm, leftMargin=18*mm, rightMargin=18*mm,
    title="CoreFly F2 PCB 布局布线设计指南")

# ── Styles ──
title_style = ParagraphStyle('Title', fontName=FONT, fontSize=22, leading=30, spaceAfter=6*mm)
h1_style = ParagraphStyle('H1', fontName=FONT, fontSize=15, leading=22, spaceBefore=10*mm, spaceAfter=4*mm, textColor=HexColor('#1a1a2e'))
h2_style = ParagraphStyle('H2', fontName=FONT, fontSize=12, leading=18, spaceBefore=6*mm, spaceAfter=2*mm, textColor=HexColor('#2d3436'))
body_style = ParagraphStyle('Body', fontName=FONT_BODY, fontSize=10, leading=16, spaceAfter=3*mm)
bul_style = ParagraphStyle('Bullet', fontName=FONT_BODY, fontSize=10, leading=16, leftIndent=12, bulletIndent=6, spaceBefore=1*mm, spaceAfter=1*mm)
small_style = ParagraphStyle('Small', fontName=FONT_BODY, fontSize=8, leading=12, textColor=HexColor('#888888'))
cell_style = ParagraphStyle('Cell', fontName=FONT_BODY, fontSize=9, leading=13)
cell_hdr = ParagraphStyle('CellHdr', fontName=FONT, fontSize=9, leading=13, textColor=HexColor('#ffffff'))

# ── Helpers ──
HDR_BG = HexColor('#2c3e50')
GRID = HexColor('#cccccc')
EVEN = HexColor('#f8f9fa')
ODD = HexColor('#ffffff')

def P(text):
    """Wrap in Paragraph so <b> and <br/> are parsed."""
    return Paragraph(text, cell_style)

def PH(text):
    """Header cell paragraph."""
    return Paragraph(text, cell_hdr)

def tbl(data, col_widths, center=False):
    """Build a Table from a list of lists. First row = header.
    All data cells are automatically wrapped in P() for HTML parsing."""
    rows = []
    # Header row
    rows.append([PH(x) for x in data[0]])
    # Data rows
    for row in data[1:]:
        rows.append([P(x) for x in row])

    t = Table(rows, colWidths=col_widths)
    style_cmds = [
        ('BACKGROUND', (0, 0), (-1, 0), HDR_BG),
        ('FONTNAME', (0, 0), (-1, -1), FONT_BODY),
        ('FONTNAME', (0, 0), (-1, 0), FONT),
        ('FONTSIZE', (0, 0), (-1, -1), 9),
        ('VALIGN', (0, 0), (-1, -1), 'MIDDLE'),
        ('GRID', (0, 0), (-1, -1), 0.5, GRID),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [EVEN, ODD]),
        ('TOPPADDING', (0, 0), (-1, -1), 5),
        ('BOTTOMPADDING', (0, 0), (-1, -1), 5),
        ('LEFTPADDING', (0, 0), (-1, -1), 5),
        ('RIGHTPADDING', (0, 0), (-1, -1), 5),
    ]
    if center:
        style_cmds.append(('ALIGN', (0, 0), (-1, -1), 'CENTER'))
    else:
        style_cmds.append(('ALIGN', (0, 0), (-1, 0), 'CENTER'))
    t.setStyle(TableStyle(style_cmds))
    return t

story = []

# ── Header ──
story.append(Paragraph("CoreFly F2 飞控电调一体板", title_style))
story.append(Paragraph("PCB 布局布线设计指南",
    ParagraphStyle('Sub', fontName=FONT, fontSize=16, leading=20, textColor=HexColor('#e74c3c'))))
story.append(Spacer(1, 3*mm))
story.append(Paragraph("版本 V1.0 | 2026-05-16 | STM32F407VET6 + FD6288Q x 4 + AON7418 x 24", small_style))
story.append(HRFlowable(width="100%", thickness=1, color=GRID))
story.append(Spacer(1, 8*mm))

# ═════════════════════════════ 1 ═════════════════════════════
story.append(Paragraph("1. 层叠方案", h1_style))
story.append(Paragraph('<b>4 层板，总厚 1.6mm，铜厚外层 1oz / 内层 1oz（或外层 2oz 用于大电流）。</b>', body_style))

story.append(tbl([
    ["层号", "名称", "内容", "关键约束"],
    ["第1层 (Top)", "信号层",
     "MCU 引脚扇出、SPI/I2C/UART、晶振、SWD<br/>3.3V/5V 短距离供电、PWM 过孔起点",
     "不走 12V 大电流、不走电机相线"],
    ["第2层 (Inner1)", "完整 GND 平面",
     "<b>全板铺铜，不开槽不切割</b><br/>模拟地/数字地在此层汇合",
     "<b>最重要！禁止走任何信号线</b>"],
    ["第3层 (Inner2)", "12V 电源平面",
     "全板铺铜，电池输入到各电调供电<br/>层间电容效应充当去耦",
     "与第2层间距建议 0.2-0.3mm"],
    ["第4层 (Bottom)", "功率 + 驱动层",
     "FD6288Q到MOSFET栅极驱动<br/>MOSFET漏/源极、电机相线输出<br/>电流采样电阻 + 运放、散热铜皮",
     "不走传感器信号、不走晶振"],
], [30*mm, 32*mm, 55*mm, 45*mm]))
story.append(Spacer(1, 4*mm))
story.append(Paragraph('<b>原理：</b>第 2 层 GND 是正面敏感电路和背面噪声功率电路之间的电磁屏蔽层。第 2/3 层之间 0.2-0.3mm 薄介质形成 PCB 层间电容，等效于整板自带高频去耦电容。', body_style))

# ═════════════════════════════ 2 ═════════════════════════════
story.append(Paragraph("2. 12V 电源走线要求", h1_style))

story.append(Paragraph("2.1 走线宽度与铜厚", h2_style))
story.append(tbl([
    ["电流等级", "1oz (35um)", "2oz (70um)"],
    ["单路 ESC 持续 30A", ">= 15mm", ">= 7mm"],
    ["单路 ESC 峰值 45A (10s)", ">= 20mm (短时可稍缩)", ">= 10mm"],
    ["整板电池入口 120A+", "大面积铺铜 + 过孔阵", "大面积铺铜 + 过孔阵"],
], [55*mm, 52*mm, 52*mm], center=True))
story.append(Spacer(1, 4*mm))

story.append(Paragraph("2.2 关键规则", h2_style))
rules = [
    "<b>回路面积最小化：</b>12V 和 GND 路径紧贴走，围起来的环路面积越小越好。4 层板第 2 层 GND + 第 3 层 12V 紧贴，回路面积几乎为零。",
    "<b>不能有缩颈：</b>整条电源路径上任何位置宽度都不能低于目标值。两个大面积铜皮之间如果只有一个细连接，电流全部挤过去会发热烧断。",
    "<b>输入电解电容：</b>离电池焊盘 <=5mm。100-470uF/25V 电解 + 22uF x 2 MLCC。",
    "<b>各电调独立退耦：</b>每个 FD6288Q 旁边放 100uF + 22uF MLCC，紧靠芯片 VCC 脚。",
    "<b>电机三相线：</b>MOSFET 到电机焊盘，在第 4 层走，越短越粗越好 (>=10mm 宽)，三相间距 >=1mm。",
]
for r in rules:
    story.append(Paragraph(f"<bullet>&bull;</bullet> {r}", bul_style))

# ═════════════════════════════ 3 ═════════════════════════════
story.append(Paragraph("3. 填充铜皮", h1_style))
story.append(Paragraph(
    '12V 电压级别使用直角填充完全没问题，不需要倒圆角。直角尖端放电和爬电问题在 12V 下可忽略。SPI (10MHz) 等信号也够用，直角不影响阻抗。',
    body_style))

story.append(tbl([
    ["做法", "结论"],
    ["直角填充 12V 铜皮", "完全可行"],
    ["EDA 铺铜功能 vs 手动画矩形", "优先用铺铜功能"],
    ["内角蚀刻残留", "只影响 0.1mm 以下细间距引脚，大铜皮不用管"],
    ["圆角", "可选，好看但对性能几乎无影响"],
    ["高速信号 (GHz级) 拐角", "你这个板子不涉及"],
], [65*mm, 95*mm]))

# ═════════════════════════════ 4 ═════════════════════════════
story.append(Paragraph("4. 过孔设计", h1_style))
story.append(Paragraph(
    '<b>全部使用通孔</b>（免费打板要求）。通孔穿过所有层时，EDA 自动在不连接的层上留出隔离环 (anti-pad)，不会短路。',
    body_style))

story.append(Paragraph("4.1 电流能力估算", h2_style))
story.append(Paragraph(
    '一个 0.3mm 内径通孔（1oz 铜，长度仅一层介质约 0.2mm）保守可通过约 <b>2A</b>。实际因过孔极短，承载能力更高，但按保守值计算。',
    body_style))

story.append(Paragraph("4.2 各处过孔数量要求", h2_style))
story.append(tbl([
    ["位置", "电流", "0.3mm 过孔数", "0.5mm 过孔数"],
    ["每个高侧 MOS 漏极到第3层", "约15A", "6-8 个", "4-5 个"],
    ["电池+ 到第3层 12V 入口", "约120A", "50-60 个", "25-30 个"],
    ["电池- 到第2层 GND", "约120A", "50-60 个", "25-30 个"],
    ["第3层到第1层 SY8120 VIN", "约2A", "2 个", "1 个"],
    ["第4层 GND 到第2层 GND", "-", "每 5-10mm 打 1 个", "同左"],
], [50*mm, 25*mm, 35*mm, 35*mm], center=True))
story.append(Spacer(1, 4*mm))
story.append(Paragraph(
    '<b>关键做法：</b>大电流过孔不要单个打，<b>成阵打</b>。MOSFET 漏极焊盘旁大面积铺铜，在铺铜区域打满过孔阵列到第 3 层。电池入口处打 5x10 或 6x10 过孔方阵。过孔多打不花钱。',
    body_style))

# ═════════════════════════════ 5 ═════════════════════════════
story.append(Paragraph("5. 布线顺序", h1_style))
story.append(Paragraph('<b>顺序反了就麻烦。先大电流后小信号，先关键后次要。</b>', body_style))

story.append(tbl([
    ["步骤", "内容", "优先级"],
    ["第1步", "器件摆放 -- 先定电机焊盘，再摆电调，再放 MCU/IMU<br/><b>布线前最重要的一步，截图检查后再动线</b>", "最高"],
    ["第2步", "第4层功率走线 -- MOSFET 漏/源极、电机相线", "最高"],
    ["第3步", "打通 12V 过孔 -- 第3层 12V 连通到第4层 MOS 漏极", "最高"],
    ["第4步", "栅极驱动线 -- FD6288Q 到 MOSFET Gate (短、粗、直 &lt;10mm)", "高"],
    ["第5步", "电流采样 -- 采样电阻到 INA180 到过孔到第1层 MCU ADC", "高"],
    ["第6步", "PWM 信号 -- MCU 到过孔到第4层 FD6288Q 输入", "中"],
    ["第7步", "传感器信号 -- SPI/I2C 在第1层走，不过孔换层", "中"],
    ["第8步", "GND 过孔阵 -- 第4层 GND 连通第2层 GND，尤其 MOSFET 旁边", "中"],
    ["第9步", "跑 DRC -- 间距/隔离环/第2层完整性检查", "收尾必须"],
], [13*mm, 117*mm, 18*mm]))

# ═════════════════════════════ 6 ═════════════════════════════
story.append(Paragraph("6. 器件摆放原则", h1_style))

placement_rules = [
    "<b>先定四个角的电机焊盘</b> -- 不可移动的物理约束，所有布局以此为基础。",
    "<b>四个电调各自靠近电机焊盘</b> -- MOSFET 到焊盘的相线最短。每个电调的三相 MOSFET 桥和 FD6288Q 放在一起。",
    "<b>MCU 放第1层中心</b> -- 到四个电调的 PWM 信号走线距离大致相等。",
    "<b>BMI270 IMU 放第1层正中心</b> -- 软件姿态解算要求。第4层对应位置下方不要放 MOSFET，保持干净。",
    "<b>SPL06-001 气压计下方 PCB 开孔</b> -- 周围用地线围一圈隔离，避开螺旋桨下洗气流。",
    "<b>电源入口放一侧</b> -- 电池焊盘 -> 电解电容 -> MLCC -> 第3层过孔阵入口。",
    "<b>LoRa 模块 (Ra-01SC) 放板边</b> -- 天线引出方向避开电机走线区域。",
    "<b>第4层 MOSFET 散热</b> -- 漏极焊盘旁大面积铺铜，铜皮上打满过孔到第3层辅助散热。",
]
for r in placement_rules:
    story.append(Paragraph(f"<bullet>&bull;</bullet> {r}", bul_style))

# ═════════════════════════════ 7 ═════════════════════════════
story.append(Paragraph("7. 关键器件 LCSC 编号", h1_style))
story.append(Paragraph(
    '嘉立创编辑器搜索时优先用 LCSC 编号（C 开头），比型号名更可靠。', body_style))

story.append(tbl([
    ["器件", "型号", "封装", "LCSC 编号", "备注"],
    ["主控 MCU", "STM32F407VET6", "LQFP-100", "C8315", ""],
    ["三相栅极驱动", "FD6288Q", "QFN-24", "C112517", "Fortior Tech"],
    ["MOSFET", "AON7418", "DFN 3x3", "自行查询", "或替代型号"],
    ["降压芯片", "SY8120B1ABC", "SOT-23-6", "自行查询", "Silergy"],
    ["IMU", "BMI270", "LGA-14", "立创可能无货", "需淘宝外购"],
    ["气压计", "SPL06-001", "LGA-8", "立创可能无货", "需淘宝外购"],
    ["LoRa 模块", "Ra-01SC", "模块", "立创可能有", ""],
    ["电流检测运放", "INA180A3", "SOT-23-5", "自行查询", "或 INA181"],
    ["SPI Flash", "W25Q128", "SOP-8", "C29277", ""],
], [26*mm, 32*mm, 22*mm, 32*mm, 28*mm]))

# ═════════════════════════════ 8 ═════════════════════════════
story.append(Paragraph("8. 最终检查清单", h1_style))
checklist = [
    "电池+到每个电调入口铜皮宽度 >=15mm (1oz) 或 7mm (2oz)",
    "电池-回路同样宽度，与+紧贴走",
    "输入电解电容离电池焊盘 <=5mm",
    "每个 FD6288Q 旁边有独立 100uF + 22uF",
    "整条 12V 路径没有缩颈",
    "过孔数量足够 (MOS 6-8个，电池入口 50-60 个)",
    "三相电机线短、粗、直 (第4层，不换层)",
    "第2层 GND 完整无切割",
    "IMU 下方第4层无 MOSFET",
    "第2/3层间距 0.2-0.3mm",
    "DRC 通过 (间距、隔离环、GND 完整性)",
]
for item in checklist:
    story.append(Paragraph(f"<bullet>&bull;</bullet> {item}", bul_style))

story.append(Spacer(1, 10*mm))
story.append(HRFlowable(width="100%", thickness=1, color=GRID))
story.append(Paragraph("文档版本 V1.0 | 2026-05-16 | 基于 CoreFly F2 项目计划书汇总", small_style))

doc.build(story)
print("Done:", output_path)
