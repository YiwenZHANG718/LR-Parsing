#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR语法分析器图形化界面 - 中文版
支持LR(0)、SLR(1)、LR(1)语法分析的完整功能

本GUI应用程序提供了一个完整的图形化界面，用于：
1. 输入和管理上下文无关文法
2. 构造LR分析表（支持LR(0)、SLR(1)、LR(1)三种类型）
3. 分析输入串并显示详细的分析过程
4. 可视化项目集、ACTION表、GOTO表
5. 导出完整的分析报告

=== 软件架构 ===

1. 界面结构：
   ┌─────────────────────────────────────────
   │ 标题栏 (Header)                          
   ├─────────────┬───────────────────────────
   │ 控制面板     │ 结果显示区域               
   │ - 文法输入   │ - 分析结果 (总览)          
   │ - 输入串     │ - ACTION表                
   │ - 分析器选择 │ - GOTO表                  
   │ - 操作按钮   │ - 项目集                  
   │             │ - 分析过程                
   └─────────────┴───────────────────────────
   │ 状态栏 (Status Bar + Progress)          
   └─────────────────────────────────────────

2. 数据流：
   文法输入 → C++后端处理 → JSON结果 → GUI显示 → 用户交互

3. 线程模型：
   - 主线程：GUI事件处理和界面更新
   - 工作线程：C++后端调用，避免界面冻结
   - 线程安全：通过root.after()在主线程更新GUI

4. 跨平台支持：
   - Windows: 查找lr_cli.exe
   - Linux/macOS: 查找lr_cli
   - 字体适配：自动检测中文字体
   - 文件路径：使用os.path.join()确保兼容性

技术特点：
- 跨平台兼容（Windows/Linux/macOS）
- 多线程处理，避免界面冻结
- 中文字体自动检测和适配
- 完整的错误处理和用户反馈
- 可滚动的界面布局，适应不同屏幕尺寸
- 丰富的键盘快捷键支持
- JSON和文本双格式导出

作者：ZJJ
版本：v2.0
日期：2025.6.10

"""

# 标准库导入
import tkinter as tk                    # GUI基础框架
from tkinter import ttk, scrolledtext, messagebox, filedialog, font  # GUI组件
import subprocess                       # 子进程调用（调用C++后端）
import os                              # 操作系统相关功能
import sys                             # 系统相关功能
import threading                       # 多线程处理
import json                            # JSON数据处理
import re                              # 正则表达式
import tempfile                        # 临时文件处理
from datetime import datetime          # 日期时间处理

class CompleteLRGUI:
    """
    完整的LR语法分析器图形用户界面类
    
    这个类实现了一个功能完整的LR语法分析器GUI，包括：
    
    主要功能：
    1. 文法输入和管理（支持文件加载/保存）
    2. 三种LR分析器类型选择（LR(0)/SLR(1)/LR(1)）
    3. 分析表构造和可视化
    4. 输入串语法分析
    5. 项目集显示
    6. 完整报告导出
    
    界面特点：
    - 左右分栏布局：左侧控制面板，右侧结果显示
    - 多标签页结果展示：分析结果、ACTION表、GOTO表、项目集、分析过程
    - 可滚动的控制面板，适应不同屏幕尺寸
    - 响应式设计，支持窗口缩放
    
    技术实现：
    - 多线程处理C++后端调用，避免界面冻结
    - 跨平台可执行文件检测
    - 中文字体自动适配
    - 完整的错误处理和用户反馈
    """
    
    def __init__(self, root):
        """
        初始化GUI界面
        
        Args:
            root (tk.Tk): Tkinter根窗口对象
            
        初始化步骤：
        1. 设置窗口基本属性（标题、大小、背景色）
        2. 配置界面样式和字体
        3. 初始化变量（文法文件路径、分析器类型等）
        4. 创建主要界面组件
        5. 绑定事件处理器
        6. 加载默认示例文法
        """
        self.root = root
        self.root.title("LR Parser Analyzer") 
        self.root.geometry("1200x800")          # 设置窗口大小为1200x800像素
        self.root.configure(bg='#f5f5f5')       # 设置背景色为浅灰色
        
        # 设置界面样式和字体
        self.setup_styles()
        
        # 初始化实例变量
        self.grammar_file = None                 # 当前加载的文法文件路径
        self.analyzer_type = tk.StringVar(value="SLR1")  # 分析器类型（默认SLR1）
        self.current_data = {}                   # 存储当前分析结果的数据
        
        # 创建主要界面组件
        self.create_widgets()
        
        # 绑定键盘和鼠标事件
        self.bind_events()
        
        # 运行状态标志
        self.analysis_running = False            # 标识是否正在执行分析操作
        
    def get_cpp_executable(self):
        """
        获取C++可执行文件路径 - 跨平台兼容
        
        该方法实现了跨平台的C++后端可执行文件检测：
        1. 根据操作系统类型确定可能的文件名
        2. 按优先级顺序查找可执行文件
        3. 返回第一个找到的可执行文件路径
        
        平台支持：
        - Windows: 优先查找 lr_cli.exe，备选 lr_cli
        - Linux/macOS: 优先查找 lr_cli，备选 lr_cli.exe
        
        Returns:
            str or None: 找到的可执行文件的完整路径，如果未找到则返回None
            
        查找位置：
            相对于GUI目录的 ../Algorithm/ 目录
        """
        # 根据操作系统确定可执行文件名列表（按优先级排序）
        executable_names = []
        if os.name == 'nt':  # Windows系统
            executable_names = ["lr_cli.exe", "lr_cli"]
        else:  # Unix-like系统 (Linux, macOS)
            executable_names = ["lr_cli", "lr_cli.exe"]
        
        # 按优先级顺序查找可执行文件
        for exe_name in executable_names:
            test_path = os.path.join(os.path.dirname(__file__), "..", "Algorithm", exe_name)
            if os.path.exists(test_path):
                return test_path
        
        return None  # 未找到任何可执行文件
    
    def setup_styles(self):
        """
        设置界面样式和主题
        
        该方法负责配置整个应用程序的视觉样式：
        1. 设置主题风格
        2. 检测和配置中文字体
        3. 定义自定义样式（标题、按钮、标签等）
        4. 设置颜色方案
        
        样式类型：
        - Title.TLabel: 主标题样式（大字体，粗体，深色）
        - Header.TLabel: 小标题样式（中等字体，粗体）
        - Action.TButton: 操作按钮样式（粗体，内边距）
        - Success.TLabel: 成功消息样式（绿色）
        - Error.TLabel: 错误消息样式（红色）
        """
        style = ttk.Style()
        style.theme_use('clam')  # 使用现代化的clam主题
        
        # 检测和设置支持中文的字体
        self.chinese_font = self.get_chinese_font()
        
        # 配置自定义样式，使用检测到的中文字体
        # 主标题样式：大字体、粗体、深蓝色
        style.configure('Title.TLabel', 
                       font=(self.chinese_font, 18, 'bold'),
                       background='#f5f5f5',
                       foreground='#2c3e50')
        
        # 小标题样式：中等字体、粗体、深灰色
        style.configure('Header.TLabel',
                       font=(self.chinese_font, 11, 'bold'),
                       background='#f5f5f5',
                       foreground='#34495e')
        
        # 操作按钮样式：粗体、较大内边距
        style.configure('Action.TButton',
                       font=(self.chinese_font, 10, 'bold'),
                       padding=(15, 8))
        
        # 成功消息样式：绿色文字
        style.configure('Success.TLabel',
                       font=(self.chinese_font, 10),
                       background='#f5f5f5',
                       foreground='#27ae60')
        
        # 错误消息样式：红色文字
        style.configure('Error.TLabel',
                       font=(self.chinese_font, 10),
                       background='#f5f5f5',
                       foreground='#e74c3c')
    
    def get_chinese_font(self):
        """
        获取系统中可用的中文字体
        
        该方法自动检测系统中可用的中文字体，确保界面能够正确显示中文：
        1. 定义中文字体优先级列表
        2. 获取系统可用字体列表
        3. 按优先级测试每个字体的中文显示能力
        4. 返回第一个可用的中文字体
        
        字体优先级（从高到低）：
        1. Noto Sans CJK SC - Google开源中文字体，显示效果最佳
        2. WenQuanYi Zen Hei - 文泉驿正黑体（Linux常用）
        3. WenQuanYi Micro Hei - 文泉驿微米黑（Linux轻量版）
        4. SimHei - 黑体（Windows系统字体）
        5. Microsoft YaHei - 微软雅黑（Windows现代字体）
        6. Liberation Sans - 开源字体（跨平台）
        7. DejaVu Sans - 开源字体（备选）
        8. Sans - 系统默认无衬线字体
        
        Returns:
            str: 可用的中文字体名称，如果都不可用则返回'TkDefaultFont'
        """
        # 定义中文字体优先级列表
        chinese_fonts = [
            'Microsoft YaHei UI',   # 微软雅黑UI（Windows 10+推荐）
            'Microsoft YaHei',      # 微软雅黑（Windows现代字体）
            'SimHei',               # 黑体（Windows系统字体）
            'SimSun',               # 宋体（Windows传统字体）
            'NSimSun',              # 新宋体（Windows字体）
            'FangSong',             # 仿宋（Windows字体）
            'KaiTi',                # 楷体（Windows字体）
            'Noto Sans CJK SC',     # Google开源中文字体，显示效果最佳
            'WenQuanYi Zen Hei',    # 文泉驿正黑体（Linux常用）
            'WenQuanYi Micro Hei',  # 文泉驿微米黑（Linux轻量版）
            'PingFang SC',          # 苹方（macOS中文字体）
            'Hiragino Sans GB',     # 冬青黑体（macOS中文字体）
            'STHeiti',              # 华文黑体（跨平台）
            'Liberation Sans',      # 开源字体（跨平台）
            'DejaVu Sans',         # 开源字体（备选）
            'Arial Unicode MS',     # 支持Unicode的Arial
            'Sans'                 # 系统默认无衬线字体
        ]
        
        # 获取系统中所有可用的字体
        available_fonts = list(tk.font.families())
        
        # 按优先级测试每个字体
        for font_name in chinese_fonts:
            if font_name in available_fonts:
                try:
                    # 创建测试字体对象
                    test_font = tk.font.Font(family=font_name, size=12)
                    # 测试该字体是否能正确显示中文字符
                    if self._test_chinese_font(test_font):
                        return font_name
                except:
                    continue  # 如果字体创建失败，继续测试下一个
        
        # 如果没有找到合适的中文字体，输出警告并使用系统默认字体
        print("警告: 未找到合适的中文字体，使用系统默认字体")
        return 'TkDefaultFont'
    
    def _test_chinese_font(self, font_obj):
        """
        测试指定字体是否支持中文显示
        
        该方法通过创建临时标签组件来测试字体是否能正确渲染中文字符：
        1. 创建临时标签并设置测试字体
        2. 尝试渲染中文字符"测试"
        3. 如果渲染成功且无异常，则认为字体可用
        4. 清理临时组件
        
        Args:
            font_obj (tk.font.Font): 要测试的字体对象
            
        Returns:
            bool: True表示字体支持中文显示，False表示不支持
            
        注意：
            该方法会创建临时UI组件，可能会有轻微的性能开销
        """
        try:
            # 创建临时测试标签组件
            test_label = tk.Label(self.root, text="测试", font=font_obj)
            test_label.pack()               # 添加到界面中
            test_label.update_idletasks()   # 强制更新显示
            test_label.destroy()            # 立即销毁临时组件
            return True                     # 如果没有异常，说明字体可用
        except:
            return False                    # 如果有任何异常，说明字体不可用
        
    def create_widgets(self):
        """
        创建主要界面组件
        
        该方法负责构建整个GUI的界面结构：
        1. 创建顶部标题区域（应用名称和版本信息）
        2. 创建主要内容区域（左右分栏布局）
        3. 创建底部状态栏（显示操作状态和进度）
        4. 加载默认示例文法
        
        界面布局结构：
        ┌─────────────────────────────────────┐
        │ 标题区域 (Header)                    │
        ├─────────────┬───────────────────────┤
        │ 控制面板     │ 结果显示区域           │
        │ (Control)   │ (Results Notebook)    │
        │            │                       │
        └─────────────┴───────────────────────┤
        │ 状态栏 (Status Bar)                 │
        └─────────────────────────────────────┘
        """
        # 创建顶部标题区域
        self.create_header()
        
        # 创建主要内容区域（左右分栏）
        self.create_main_area()
        
        # 创建底部状态栏
        self.create_status_bar()
        
        # 加载默认示例文法（所有组件创建完成后）
        self.load_example_grammar()
        
    def create_header(self):
        """
        创建顶部标题区域
        
        该方法创建应用程序的顶部标题栏，包含：
        1. 左侧：应用程序名称（大字体、粗体）
        2. 右侧：版本信息和功能说明
        
        布局：
        ┌──────────────────────────────────────────────┐
        │ LR语法分析器               v2.0 - 支持 LR... │
        └──────────────────────────────────────────────┘
        """
        # 创建标题容器框架
        header_frame = ttk.Frame(self.root)
        header_frame.pack(fill='x', padx=15, pady=10)
        
        # 左侧主标题：应用程序名称
        title_label = ttk.Label(header_frame,
                       text="LR语法分析器",
                       font=(self.chinese_font, 17, 'bold'),
                       style='Title.TLabel')
        title_label.pack(side='left')
        
        # 右侧版本信息：版本号和功能描述
        version_label = ttk.Label(header_frame, 
                                 text="v2.0 - 支持 LR(0) / SLR(1) / LR(1)", 
                                 font=('Arial', 9),
                                 foreground='#7f8c8d')
        version_label.pack(side='right')
        
    def create_main_area(self):
        """
        创建主要内容区域
        
        该方法创建应用程序的主要工作区域，采用左右分栏布局：
        1. 左侧：控制面板（文法输入、参数设置、操作按钮）
        2. 右侧：结果显示面板（多标签页展示不同类型的结果）
        
        分栏比例：
        - 左侧控制面板：权重1（较窄）
        - 右侧结果面板：权重3（较宽）
        
        这种布局让用户可以在左侧进行操作，在右侧查看结果，
        符合从左到右的操作流程习惯。
        """
        # 创建水平分割的主面板容器
        main_paned = ttk.PanedWindow(self.root, orient='horizontal')
        main_paned.pack(fill='both', expand=True, padx=15, pady=5)
        
        # 左侧控制面板框架
        left_frame = ttk.Frame(main_paned)
        main_paned.add(left_frame, weight=1)  # 权重1，相对较窄
        
        # 右侧结果显示框架
        right_frame = ttk.Frame(main_paned)
        main_paned.add(right_frame, weight=3)  # 权重3，相对较宽
        
        # 创建左侧控制面板的内容
        self.create_control_panel(left_frame)
        
        # 创建右侧结果面板的内容
        self.create_result_panel(right_frame)
        
    def create_control_panel(self, parent):
        """
        创建左侧控制面板
        
        该方法创建一个可滚动的控制面板，包含所有用户操作控件：
        1. 文法输入区域：文本编辑器和文件操作按钮
        2. 输入串区域：待分析字符串的输入框
        3. 分析器类型选择：LR(0)/SLR(1)/LR(1)单选按钮
        4. 操作按钮区域：主要功能按钮和辅助功能按钮
        
        可滚动设计原因：
        - 适应不同屏幕尺寸和分辨率
        - 确保所有控件在小屏幕上也能访问
        - 支持鼠标滚轮操作，提升用户体验
        
        Args:
            parent: 父容器组件
        """
        # 创建可滚动框架的组件结构
        # Canvas + Scrollbar + Frame 的组合实现可滚动区域
        canvas = tk.Canvas(parent, bg='#f5f5f5')
        scrollbar = ttk.Scrollbar(parent, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)
        
        # 配置滚动区域：当内容框架大小改变时，更新滚动区域
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        # 将可滚动框架嵌入到Canvas中
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        # 布局Canvas和滚动条
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # 在可滚动框架中创建各个功能区域
        # 文法输入区域：文法规则编辑和文件操作
        self.create_grammar_section(scrollable_frame)
        
        # 输入串区域：待分析的输入字符串
        self.create_input_section(scrollable_frame)
        
        # 分析器类型选择区域：LR(0)/SLR(1)/LR(1)
        self.create_analyzer_section(scrollable_frame)
        
        # 操作按钮区域：主要功能和辅助功能按钮
        self.create_operation_section(scrollable_frame)
        
        # 绑定鼠标滚轮事件到Canvas，实现滚轮滚动
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1*(event.delta/120)), "units")
        canvas.bind_all("<MouseWheel>", _on_mousewheel)
        
    def create_grammar_section(self, parent):
        """
        创建文法输入区域
        
        该方法创建文法规则编辑区域，是用户主要的输入界面：
        
        组件结构：
        1. 顶部按钮栏：文件操作按钮（加载、保存、示例）
        2. 说明标签：提示用户输入格式
        3. 多行文本编辑器：支持滚动的文法输入框
        
        文本编辑器特性：
        - 固定高度：10行，确保界面紧凑
        - 固定宽度：45字符，适合文法规则
        - 中文字体：使用检测到的中文字体
        - 无自动换行：保持文法格式清晰
        - 高亮选择：蓝色背景突出显示选中文本
        
        按钮功能：
        - 加载文件：从外部文件导入文法
        - 保存文件：将当前文法保存到文件
        - 示例文法：加载预设的示例文法
        
        Args:
            parent: 父容器组件
        """
        # 创建文法输入区域的容器
        grammar_frame = ttk.LabelFrame(parent, text="文法输入", padding=15)
        grammar_frame.pack(fill='x', padx=5, pady=5)
        
        # 创建顶部按钮栏
        btn_frame = ttk.Frame(grammar_frame)
        btn_frame.pack(fill='x', pady=(0, 10))
        
        # 文件操作按钮
        ttk.Button(btn_frame, text="加载文件", 
                  command=self.load_grammar_file,
                  style='Action.TButton').pack(side='left', padx=(0, 5))
        
        ttk.Button(btn_frame, text="保存文件", 
                  command=self.save_grammar_file,
                  style='Action.TButton').pack(side='left', padx=5)
        
        ttk.Button(btn_frame, text="示例文法", 
                  command=self.load_example_grammar,
                  style='Action.TButton').pack(side='left', padx=5)
        
        # 输入说明标签
        ttk.Label(grammar_frame, text="文法规则（每行一个产生式）:", 
                 style='Header.TLabel').pack(anchor='w', pady=(0, 5))
        
        # 文法输入文本框
        self.grammar_text = scrolledtext.ScrolledText(
            grammar_frame, 
            height=10,                      # 显示10行文本
            width=45,                       # 显示45个字符宽度
            font=(self.chinese_font, 11),   # 使用中文字体，11号大小
            wrap='none',                    # 不自动换行，保持格式
            bg='white',                     # 白色背景
            selectbackground='#3498db',     # 蓝色选择背景
            selectforeground='white')       # 白色选择前景
        self.grammar_text.pack(fill='x', pady=5)
        
    def create_input_section(self, parent):
        """创建输入串区域"""
        input_frame = ttk.LabelFrame(parent, text="输入串", padding=10)
        input_frame.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(input_frame, text="待分析的输入串:", 
                 style='Header.TLabel').pack(anchor='w')
        
        self.input_entry = ttk.Entry(input_frame, font=(self.chinese_font, 11))
        self.input_entry.pack(fill='x', pady=3)
        self.input_entry.insert(0, "a + a * a")  # Default input
        
        # Input tip
        tip_label = ttk.Label(input_frame, 
                             text="提示: 用空格分隔符号",
                             font=('Arial', 9),
                             foreground='#7f8c8d')
        tip_label.pack(anchor='w', pady=(3, 0))
        
    def create_analyzer_section(self, parent):
        """创建分析器类型选择区域"""
        type_frame = ttk.LabelFrame(parent, text="分析器类型", padding=15)
        type_frame.pack(fill='x', padx=5, pady=5)
        
        # Radio buttons
        ttk.Radiobutton(type_frame, text="LR(0) - 基础LR分析", 
                       variable=self.analyzer_type, 
                       value="LR0").pack(anchor='w', pady=2)
        
        ttk.Radiobutton(type_frame, text="SLR(1) - 简单LR分析", 
                       variable=self.analyzer_type, 
                       value="SLR1").pack(anchor='w', pady=2)
        
        ttk.Radiobutton(type_frame, text="LR(1) - 规范LR分析", 
                       variable=self.analyzer_type, 
                       value="LR1").pack(anchor='w', pady=2)
        
    def create_operation_section(self, parent):
        """创建操作按钮区域"""
        action_frame = ttk.LabelFrame(parent, text="操作", padding=10)
        action_frame.pack(fill='x', padx=5, pady=5)
        
        # Main operation buttons
        ttk.Button(action_frame, text="构造分析表", 
                  command=self.construct_table,
                  style='Action.TButton').pack(fill='x', pady=2)
        
        ttk.Button(action_frame, text="分析输入串", 
                  command=self.analyze_input,
                  style='Action.TButton').pack(fill='x', pady=2)
        
        ttk.Button(action_frame, text="显示项目集", 
                  command=self.show_item_sets,
                  style='Action.TButton').pack(fill='x', pady=2)
        
        # Separator line
        ttk.Separator(action_frame, orient='horizontal').pack(fill='x', pady=5)
        
        # Auxiliary operation buttons
        ttk.Button(action_frame, text="清空结果", 
                  command=self.clear_results).pack(fill='x', pady=2)
        
        ttk.Button(action_frame, text="导出报告", 
                  command=self.export_report).pack(fill='x', pady=2)
        
    def create_result_panel(self, parent):
        """
        创建右侧结果显示面板
        
        该方法创建多标签页的结果显示区域，是分析结果的主要展示界面：
        
        标签页结构：
        1. 分析结果：总体摘要和状态信息
        2. ACTION表：LR分析器的动作表
        3. GOTO表：LR分析器的跳转表  
        4. 项目集：LR(0)/SLR(1)/LR(1)的项目集族
        5. 分析过程：输入串的详细分析步骤
        
        设计优势：
        - 标签页分离：不同类型的结果独立显示，避免混乱
        - 统一样式：所有标签页使用相同的布局和字体
        - 工具栏：每个标签页都有复制按钮等操作
        - 可扩展：易于添加新的结果类型标签页
        
        用户体验：
        - 直观导航：标签页名称清晰表达内容
        - 快速切换：点击标签页即可查看不同结果
        - 复制功能：每个标签页都支持内容复制
        
        Args:
            parent: 父容器组件
        """
        # 创建多标签页容器
        self.notebook = ttk.Notebook(parent)
        self.notebook.pack(fill='both', expand=True, padx=5, pady=5)
        
        # 创建各个功能标签页
        self.create_analysis_tab()     # 分析结果总览标签页
        self.create_action_tab()       # ACTION表标签页
        self.create_goto_tab()         # GOTO表标签页
        self.create_itemsets_tab()     # 项目集标签页
        self.create_process_tab()      # 分析过程标签页
        
    def create_analysis_tab(self):
        """创建分析结果标签页"""
        self.analysis_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.analysis_frame, text="分析结果")
        
        # Toolbar
        toolbar = ttk.Frame(self.analysis_frame)
        toolbar.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(toolbar, text="分析结果:", style='Header.TLabel').pack(side='left')
        
        ttk.Button(toolbar, text="复制", 
                  command=lambda: self.copy_text(self.analysis_text)).pack(side='right', padx=5)
        
        # Result text box
        self.analysis_text = scrolledtext.ScrolledText(
            self.analysis_frame, 
            font=(self.chinese_font, 10),
            wrap='none',
            bg='#fefefe',
            selectbackground='#3498db',
            selectforeground='white')
        self.analysis_text.pack(fill='both', expand=True, padx=5, pady=5)
        
    def create_action_tab(self):
        """创建ACTION表标签页"""
        self.action_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.action_frame, text="ACTION表")
        
        # Toolbar
        toolbar = ttk.Frame(self.action_frame)
        toolbar.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(toolbar, text="ACTION表:", style='Header.TLabel').pack(side='left')
        
        ttk.Button(toolbar, text="复制", 
                  command=lambda: self.copy_text(self.action_text)).pack(side='right', padx=5)
        
        # ACTION table display area
        self.action_text = scrolledtext.ScrolledText(
            self.action_frame, 
            font=('Consolas', 10),
            wrap='none',
            bg='#fefefe',
            selectbackground='#3498db',
            selectforeground='white')
        self.action_text.pack(fill='both', expand=True, padx=5, pady=5)
        
    def create_goto_tab(self):
        """创建GOTO表标签页"""
        self.goto_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.goto_frame, text="GOTO表")
        
        # Toolbar
        toolbar = ttk.Frame(self.goto_frame)
        toolbar.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(toolbar, text="GOTO表:", style='Header.TLabel').pack(side='left')
        
        ttk.Button(toolbar, text="复制", 
                  command=lambda: self.copy_text(self.goto_text)).pack(side='right', padx=5)
        
        # GOTO table display area
        self.goto_text = scrolledtext.ScrolledText(
            self.goto_frame, 
            font=('Consolas', 10),
            wrap='none',
            bg='#fefefe',
            selectbackground='#3498db',
            selectforeground='white')
        self.goto_text.pack(fill='both', expand=True, padx=5, pady=5)
        
    def create_itemsets_tab(self):
        """创建项目集标签页"""
        self.itemsets_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.itemsets_frame, text="项目集")
        
        # Toolbar
        toolbar = ttk.Frame(self.itemsets_frame)
        toolbar.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(toolbar, text="项目集:", style='Header.TLabel').pack(side='left')
        
        ttk.Button(toolbar, text="复制", 
                  command=lambda: self.copy_text(self.itemsets_text)).pack(side='right', padx=5)
        
        # Item sets display area
        self.itemsets_text = scrolledtext.ScrolledText(
            self.itemsets_frame, 
            font=('Consolas', 10),
            wrap='none',
            bg='#fefefe',
            selectbackground='#3498db',
            selectforeground='white')
        self.itemsets_text.pack(fill='both', expand=True, padx=5, pady=5)
        
    def create_process_tab(self):
        """创建分析过程标签页"""
        self.process_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.process_frame, text="分析过程")
        
        # Toolbar
        toolbar = ttk.Frame(self.process_frame)
        toolbar.pack(fill='x', padx=5, pady=5)
        
        ttk.Label(toolbar, text="详细分析过程:", style='Header.TLabel').pack(side='left')
        
        ttk.Button(toolbar, text="复制", 
                  command=lambda: self.copy_text(self.process_text)).pack(side='right', padx=5)
        
        # Analysis process display area
        self.process_text = scrolledtext.ScrolledText(
            self.process_frame, 
            font=(self.chinese_font, 10),
            wrap='none',
            bg='#fefefe',
            selectbackground='#3498db',
            selectforeground='white')
        self.process_text.pack(fill='both', expand=True, padx=5, pady=5)
        
    def create_status_bar(self):
        """创建状态栏"""
        status_frame = ttk.Frame(self.root)
        status_frame.pack(fill='x', side='bottom')
        
        self.status_bar = ttk.Label(status_frame, text="就绪", relief='sunken')
        self.status_bar.pack(fill='x', padx=15, pady=5)
        
        # Progress bar (hidden state)
        self.progress_bar = ttk.Progressbar(status_frame, mode='indeterminate')
        
    def bind_events(self):
        """
        绑定键盘快捷键和事件处理器
        
        该方法为GUI应用程序配置各种事件绑定，提升用户体验：
        
        键盘快捷键：
        - Ctrl+O: 打开文法文件
        - Ctrl+S: 保存文法文件  
        - F5: 构造分析表
        - F6: 分析输入串
        - F7: 显示项目集
        - Enter: 在输入框中按回车键执行分析
        
        界面事件：
        - 标签页切换事件：更新状态栏显示
        
        设计目的：
        - 提供快速访问常用功能的方式
        - 符合用户的操作习惯（如Ctrl+O打开文件）
        - 提升操作效率，减少鼠标点击
        """
        # 文件操作快捷键
        self.root.bind('<Control-o>', lambda e: self.load_grammar_file())  # Ctrl+O: 打开
        self.root.bind('<Control-s>', lambda e: self.save_grammar_file())  # Ctrl+S: 保存
        
        # 分析功能快捷键
        self.root.bind('<F5>', lambda e: self.construct_table())    # F5: 构造分析表
        self.root.bind('<F6>', lambda e: self.analyze_input())     # F6: 分析输入串
        self.root.bind('<F7>', lambda e: self.show_item_sets())    # F7: 显示项目集
        
        # 输入框回车键绑定：按回车执行输入串分析
        self.input_entry.bind('<Return>', lambda e: self.analyze_input())
        
        # 标签页切换事件：更新状态栏显示当前标签页
        self.notebook.bind('<<NotebookTabChanged>>', self.on_tab_changed)
        
    def load_grammar_file(self):
        """
        加载文法文件
        
        该方法提供文件选择对话框，让用户加载外部文法文件：
        1. 显示文件选择对话框，支持.txt文件过滤
        2. 读取选中文件的内容（使用UTF-8编码）
        3. 将内容显示在文法编辑区
        4. 更新状态栏显示操作结果
        
        支持的文件格式：
        - .txt文件（主要）
        - 所有文件类型（备选）
        
        错误处理：
        - 文件不存在或无法读取
        - 编码问题
        - 权限问题
        
        用户体验：
        - 默认在当前目录打开
        - 显示友好的文件类型过滤
        - 提供操作反馈（状态栏更新）
        """
        # 显示文件选择对话框
        file_path = filedialog.askopenfilename(
            title="选择文法文件",
            filetypes=[
                ("文本文件", "*.txt"),    # 主要支持的文件类型
                ("所有文件", "*.*")       # 备选：支持所有文件
            ],
            initialdir=os.path.join(os.path.dirname(__file__))  # 默认目录：GUI脚本所在目录
        )
        
        if file_path:  # 如果用户选择了文件（没有取消）
            try:
                # 读取文件内容，使用UTF-8编码确保中文兼容性
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                # 更新文法编辑区内容
                self.grammar_text.delete(1.0, tk.END)      # 清空现有内容
                self.grammar_text.insert(1.0, content)     # 插入新内容
                
                # 记录当前文件路径
                self.grammar_file = file_path
                
                # 更新状态栏显示操作成功
                self.update_status(f"已加载文法文件: {os.path.basename(file_path)}")
                
            except Exception as e:
                # 错误处理：显示用户友好的错误信息
                messagebox.showerror("错误", f"无法加载文件: {str(e)}")
                
    def save_grammar_file(self):
        """保存文法文件"""
        content = self.grammar_text.get(1.0, tk.END).strip()
        if not content:
            messagebox.showwarning("警告", "文法内容为空")
            return
            
        file_path = filedialog.asksaveasfilename(
            title="保存文法文件",
            defaultextension=".txt",
            filetypes=[
                ("文本文件", "*.txt"),
                ("所有文件", "*.*")
            ],
            initialdir=os.path.join(os.path.dirname(__file__))
        )
        
        if file_path:
            try:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(content)
                self.grammar_file = file_path
                self.update_status(f"已保存文法文件: {os.path.basename(file_path)}")
            except Exception as e:
                messagebox.showerror("错误", f"无法保存文件: {str(e)}")
                
    def load_example_grammar(self):
        """加载示例文法"""
        example_grammar = """
        E -> E + T
        E -> T
        T -> T * F
        T -> F
        F -> ( E )
        F -> a
        """
        self.grammar_text.delete(1.0, tk.END)
        self.grammar_text.insert(1.0, example_grammar)
        self.update_status("已加载示例文法")
        
    def construct_table(self):
        """
        构造LR分析表
        
        该方法是分析表构造的主入口函数：
        1. 验证文法输入的有效性
        2. 更新界面状态（显示进度条，更新状态栏）
        3. 启动后台线程执行实际的构造工作
        
        设计特点：
        - 异步处理：避免阻塞GUI主线程
        - 用户反馈：显示进度指示和状态信息
        - 错误处理：验证输入有效性
        
        工作流程：
        输入验证 → 显示进度 → 后台线程处理 → 更新结果
        """
        # 首先验证文法输入是否有效
        if not self.validate_grammar():
            return
            
        # 设置运行状态和用户界面反馈
        self.analysis_running = True
        self.update_status("正在构造分析表...")
        self.show_progress()
        
        # 在新线程中执行构造工作，避免阻塞GUI
        threading.Thread(target=self._construct_table_worker, daemon=True).start()
        
    def _construct_table_worker(self):
        """
        构造分析表的后台工作线程
        
        该方法在独立线程中执行分析表构造的具体工作：
        1. 将当前文法保存到临时文件
        2. 获取C++后端可执行文件路径
        3. 调用C++程序执行分析表构造
        4. 解析返回的JSON结果
        5. 通过主线程更新GUI显示
        
        线程安全设计：
        - 所有GUI更新操作都通过root.after()在主线程中执行
        - 使用try-except确保异常不会导致线程崩溃
        - 无论成功失败都会隐藏进度条
        
        C++后端调用：
        命令格式：lr_cli <文法文件> -t <分析器类型> --table --json
        """
        try:
            # 第一步：将当前文法内容保存到临时文件
            temp_grammar_file = os.path.join(tempfile.gettempdir(), "lr_grammar.txt")
            grammar_content = self.grammar_text.get(1.0, tk.END).strip()
            
            with open(temp_grammar_file, 'w', encoding='utf-8') as f:
                f.write(grammar_content)
            
            # 第二步：获取分析器类型和C++可执行文件路径
            analyzer_type = self.analyzer_type.get().lower()
            cpp_executable = self.get_cpp_executable()
            
            # 第三步：检查C++可执行文件是否存在
            if not cpp_executable:
                error_msg = "找不到C++可执行文件\n请确保lr_cli或lr_cli.exe存在于Algorithm目录中"
                self.root.after(0, lambda: self.show_error(error_msg))
                return
            
            # 第四步：构造命令行并调用C++程序
            cmd = [cpp_executable, temp_grammar_file, "-t", analyzer_type, "--table", "--json"]
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8')
            
            # 第五步：在主线程中更新界面显示结果
            self.root.after(0, lambda: self._update_table_results(result))
            
        except Exception as e:
            # 异常处理：在主线程中显示错误信息
            self.root.after(0, lambda: self.show_error(f"构造分析表出错: {str(e)}"))
        finally:
            # 清理工作：无论成功失败都要隐藏进度条
            self.root.after(0, self.hide_progress)
            
    def _update_table_results(self, result):
        """
        更新分析表构造结果
        
        该方法处理C++后端返回的分析表构造结果：
        1. 检查命令执行状态
        2. 解析JSON格式的返回数据
        3. 更新各个标签页的显示内容
        4. 处理错误情况并提供用户友好的错误信息
        
        成功流程：
        解析JSON → 更新ACTION表 → 更新GOTO表 → 更新项目集 → 显示摘要 → 切换到结果标签页
        
        失败流程：
        格式化错误信息 → 显示错误对话框 → 在分析结果标签页显示详细错误
        
        Args:
            result: subprocess.run()返回的结果对象，包含returncode、stdout、stderr
        """
        self.analysis_running = False
        
        if result.returncode == 0:
            # 成功情况：解析并显示结果
            try:
                # 解析C++后端返回的JSON数据
                output = result.stdout.strip()
                self.current_data = json.loads(output)
                
                # 更新各个标签页的显示内容
                self.display_action_table()    # 更新ACTION表显示
                self.display_goto_table()     # 更新GOTO表显示
                self.display_item_sets()      # 更新项目集显示
                
                # 在分析结果标签页显示构造摘要
                self.display_analysis_summary()
                
                # 更新状态栏并切换到ACTION表标签页
                self.update_status("分析表构造完成")
                self.notebook.select(self.action_frame)
                
            except json.JSONDecodeError as e:
                # JSON解析失败的错误处理
                self.show_error(f"解析JSON数据失败: {str(e)}")
        else:
            # 失败情况：显示错误信息
            error_msg = result.stderr or result.stdout
            formatted_error = self.format_error_message(error_msg)
            self.show_error(f"分析表构造失败:\n{formatted_error}")
            
            # 同时在分析结果标签页显示详细错误信息
            self.analysis_text.delete(1.0, tk.END)
            self.analysis_text.insert(tk.END, formatted_error)
            
            # 切换到分析结果标签页以显示错误
            self.notebook.select(self.analysis_frame)
            
    def display_action_table(self):
        """
        显示ACTION表
        
        该方法将JSON格式的ACTION表数据转换为易读的表格格式：
        1. 从current_data中提取action_table数据
        2. 分析所有终结符，作为表格的列标题
        3. 创建表格标题行和分隔线
        4. 按状态编号顺序创建每一行数据
        5. 添加图例说明各种动作的含义
        
        表格格式：
        状态\符号    a    +    (    )    $
        ================================
        0          s5        s1        
        1          s5        s1        
        2               s8             acc
        ...
        
        图例说明：
        - s<数字>: 移进到状态<数字>
        - r<数字>: 用产生式<数字>归约
        - acc: 接受
        - 空白: 错误（语法错误）
        """
        if 'action_table' not in self.current_data:
            return
            
        # 清空之前的内容
        self.action_text.delete(1.0, tk.END)
        
        action_table = self.current_data['action_table']
        
        # 收集所有终结符作为列标题
        terminals = set()
        for state_actions in action_table.values():
            terminals.update(state_actions.keys())
        terminals = sorted(list(terminals))  # 按字母顺序排序
        
        # 创建表格标题行
        header = "状态\\符号".ljust(12)  # 第一列：状态编号
        for terminal in terminals:
            header += terminal.ljust(10)   # 每个终结符占10个字符宽度
        self.action_text.insert(tk.END, header + "\n")
        
        # 创建分隔线
        self.action_text.insert(tk.END, "=" * len(header) + "\n")
        
        # 创建表格内容：按状态编号排序
        states = sorted([int(state) for state in action_table.keys()])
        for state in states:
            row = str(state).ljust(12)  # 状态编号列
            state_actions = action_table.get(str(state), {})
            
            # 为每个终结符填入对应的动作
            for terminal in terminals:
                action = state_actions.get(terminal, "")  # 如果没有动作则为空
                row += action.ljust(10)
            
            self.action_text.insert(tk.END, row + "\n")
            
        # 添加图例说明
        self.action_text.insert(tk.END, "\n图例:\n")
        self.action_text.insert(tk.END, "s<数字> = 移进到状态<数字>\n")
        self.action_text.insert(tk.END, "r<数字> = 用产生式<数字>归约\n")
        self.action_text.insert(tk.END, "acc = 接受\n")
        self.action_text.insert(tk.END, "空白 = 错误\n")
        
    def display_goto_table(self):
        """显示GOTO表"""
        if 'goto_table' not in self.current_data:
            return
            
        self.goto_text.delete(1.0, tk.END)
        
        goto_table = self.current_data['goto_table']
        
        # Get all nonterminals
        nonterminals = set()
        for state_gotos in goto_table.values():
            nonterminals.update(state_gotos.keys())
        nonterminals = sorted(list(nonterminals))
        
        if not nonterminals:
            self.goto_text.insert(tk.END, "无GOTO表数据\n")
            return
        
        # Create header
        header = "状态\\符号".ljust(12)
        for nonterminal in nonterminals:
            header += nonterminal.ljust(10)
        self.goto_text.insert(tk.END, header + "\n")
        self.goto_text.insert(tk.END, "=" * len(header) + "\n")
        
        # Create table content
        states = sorted([int(state) for state in goto_table.keys()])
        for state in states:
            row = str(state).ljust(12)
            state_gotos = goto_table.get(str(state), {})
            
            for nonterminal in nonterminals:
                goto_state = state_gotos.get(nonterminal, "")
                row += str(goto_state).ljust(10)
            
            self.goto_text.insert(tk.END, row + "\n")
            
        # Add explanation
        self.goto_text.insert(tk.END, "\n图例:\n")
        self.goto_text.insert(tk.END, "<数字> = 转到状态<数字>\n")
        self.goto_text.insert(tk.END, "空白 = 无转移\n")
        
    def display_item_sets(self):
        """显示项目集"""
        if 'item_sets' not in self.current_data:
            return
            
        self.itemsets_text.delete(1.0, tk.END)
        
        item_sets = self.current_data['item_sets']
        
        for item_set in item_sets:
            set_id = item_set['id']
            items = item_set['items']
            
            self.itemsets_text.insert(tk.END, f"I{set_id}:\n")
            
            for item in items:
                self.itemsets_text.insert(tk.END, f"    {item}\n")
            
            self.itemsets_text.insert(tk.END, "\n")
            
    def display_analysis_summary(self):
        """显示分析总结"""
        self.analysis_text.delete(1.0, tk.END)
        
        analyzer_type = self.analyzer_type.get()
        
        # Summary info
        summary = f"=== {analyzer_type} 分析表构造结果 ===\n\n"
        summary += f"构造时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n"
        summary += f"分析器类型: {analyzer_type}\n\n"
        
        # Statistics
        if 'action_table' in self.current_data:
            num_states = len(self.current_data['action_table'])
            summary += f"状态数: {num_states}\n"
            
        if 'item_sets' in self.current_data:
            num_item_sets = len(self.current_data['item_sets'])
            summary += f"项目集数: {num_item_sets}\n"
            
        # Terminal and nonterminal statistics
        terminals = set()
        nonterminals = set()
        
        if 'action_table' in self.current_data:
            for state_actions in self.current_data['action_table'].values():
                terminals.update(state_actions.keys())
                
        if 'goto_table' in self.current_data:
            for state_gotos in self.current_data['goto_table'].values():
                nonterminals.update(state_gotos.keys())
                
        summary += f"终结符数: {len(terminals)}\n"
        summary += f"非终结符数: {len(nonterminals)}\n\n"
        
        # List symbols
        if terminals:
            summary += f"终结符: {', '.join(sorted(terminals))}\n"
        if nonterminals:
            summary += f"非终结符: {', '.join(sorted(nonterminals))}\n\n"
        
        summary += "=== 分析表构造成功 ===\n"
        summary += "可在其他标签页查看详细的ACTION表、GOTO表和项目集。\n"
        
        self.analysis_text.insert(tk.END, summary)
        
    def analyze_input(self):
        """分析输入串"""
        if not self.current_data:
            messagebox.showwarning("警告", "请先构造分析表")
            return
            
        input_string = self.input_entry.get().strip()
        if not input_string:
            messagebox.showwarning("警告", "请输入待分析的字符串")
            return
            
        self.analysis_running = True
        self.update_status("正在分析输入串...")
        self.show_progress()
        
        # Execute in new thread
        threading.Thread(target=self._analyze_input_worker, 
                        args=(input_string,), daemon=True).start()
        
    def _analyze_input_worker(self, input_string):
        """分析输入串工作线程"""
        try:
            # Save grammar to temporary file
            temp_grammar_file = os.path.join(tempfile.gettempdir(), "lr_grammar.txt")
            grammar_content = self.grammar_text.get(1.0, tk.END).strip()
            
            with open(temp_grammar_file, 'w', encoding='utf-8') as f:
                f.write(grammar_content)
            
            # Get C++ executable path
            analyzer_type = self.analyzer_type.get().lower()
            cpp_executable = self.get_cpp_executable()
            
            if not cpp_executable:
                error_msg = "找不到C++可执行文件\n请确保lr_cli或lr_cli.exe存在于Algorithm目录中"
                self.root.after(0, lambda: self.show_error(error_msg))
                return
            
            cmd = [cpp_executable, temp_grammar_file, "-t", analyzer_type, "-s", input_string]
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8')
            
            # Update interface in main thread
            self.root.after(0, lambda: self._update_analysis_results(result, input_string))
            
        except Exception as e:
            self.root.after(0, lambda: self.show_error(f"分析输入串出错: {str(e)}"))
        finally:
            self.root.after(0, self.hide_progress)
            
    def _update_analysis_results(self, result, input_string):
        """更新分析结果"""
        self.analysis_running = False
        
        if result.returncode == 0:
            output = result.stdout
            self.process_text.delete(1.0, tk.END)
            self.process_text.insert(1.0, output)
            
            # Update analysis result tab
            self.analysis_text.delete(1.0, tk.END)
            summary = f"=== 输入串分析结果 ===\n\n"
            summary += f"输入串: {input_string}\n"
            summary += f"分析器: {self.analyzer_type.get()}\n"
            summary += f"结果: 分析成功\n\n"
            summary += "详细分析过程请查看\"分析过程\"标签页。\n"
            self.analysis_text.insert(1.0, summary)
            
            self.notebook.select(self.process_frame)
            self.update_status("输入串分析完成")
        else:
            error_msg = result.stderr or result.stdout
            self.process_text.delete(1.0, tk.END)
            self.process_text.insert(1.0, f"输入串分析失败:\n{error_msg}")
            self.update_status("输入串分析失败")
            
    def show_item_sets(self):
        """显示项目集"""
        if not self.validate_grammar():
            return
            
        self.analysis_running = True
        self.update_status("正在获取项目集...")
        self.show_progress()
        
        # Execute in new thread
        threading.Thread(target=self._show_item_sets_worker, daemon=True).start()
    
    def _show_item_sets_worker(self):
        """获取项目集工作线程"""
        try:
            # Save grammar to temporary file
            temp_grammar_file = os.path.join(tempfile.gettempdir(), "lr_grammar.txt")
            grammar_content = self.grammar_text.get(1.0, tk.END).strip()
            
            with open(temp_grammar_file, 'w', encoding='utf-8') as f:
                f.write(grammar_content)
            
            # Get C++ executable path
            analyzer_type = self.analyzer_type.get().lower()
            cpp_executable = self.get_cpp_executable()
            
            if not cpp_executable:
                error_msg = "找不到C++可执行文件\n请确保lr_cli或lr_cli.exe存在于Algorithm目录中"
                self.root.after(0, lambda: self.show_error(error_msg))
                return
            
            # Run command to get item sets
            cmd = [cpp_executable, temp_grammar_file, "-t", analyzer_type, "--items", "--json"]
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8')
            
            # Update interface in main thread
            self.root.after(0, lambda: self._update_item_sets_results(result))
            
        except Exception as e:
            self.root.after(0, lambda: self.show_error(f"获取项目集出错: {str(e)}"))
        finally:
            self.root.after(0, self.hide_progress)
    
    def _update_item_sets_results(self, result):
        """更新项目集结果"""
        self.analysis_running = False
        
        if result.returncode == 0:
            try:
                # Parse JSON data
                output = result.stdout.strip()
                data = json.loads(output)
                
                # Update current_data with item sets
                if 'item_sets' in data:
                    self.current_data['item_sets'] = data['item_sets']
                
                # Display item sets
                self.display_item_sets()
                
                # Switch to item sets tab
                self.notebook.select(self.itemsets_frame)
                self.update_status("项目集获取完成")
                
            except json.JSONDecodeError as e:
                self.show_error(f"解析项目集JSON数据失败: {str(e)}")
        else:
            error_msg = result.stderr or result.stdout
            self.show_error(f"获取项目集失败:\n{error_msg}")
            # Also display error in analysis result tab
            self.analysis_text.delete(1.0, tk.END)
            error_summary = f"=== 项目集获取失败 ===\n\n"
            error_summary += f"分析器类型: {self.analyzer_type.get()}\n"
            error_summary += f"错误信息:\n{error_msg}\n\n"
            error_summary += "请检查文法是否正确，并确保符合所选分析器类型的要求。"
            self.analysis_text.insert(tk.END, error_summary)
        
    def clear_results(self):
        """清空所有结果"""
        self.analysis_text.delete(1.0, tk.END)
        self.action_text.delete(1.0, tk.END)
        self.goto_text.delete(1.0, tk.END)
        self.itemsets_text.delete(1.0, tk.END)
        self.process_text.delete(1.0, tk.END)
        self.current_data = {}
        self.update_status("已清空所有结果")
        
    def export_report(self):
        """
        导出完整分析报告
        
        该方法允许用户将所有分析结果导出为文件，支持两种格式：
        1. JSON格式：结构化数据，便于程序处理
        2. 文本格式：人类可读的完整报告
        
        JSON格式内容：
        - 元数据：生成时间、分析器类型、文法内容
        - 分析表：ACTION表、GOTO表
        - 项目集：完整的项目集族
        - 分析过程：如果有输入串分析结果
        
        文本格式内容：
        - 报告标题和元信息
        - 文法规则
        - 分析结果摘要
        - ACTION表（格式化表格）
        - GOTO表（格式化表格）
        - 项目集列表
        - 分析过程详情
        
        特点：
        - 支持中文内容的正确编码
        - 提供完整的上下文信息
        - 格式化输出，便于阅读和打印
        """
        # 显示文件保存对话框
        file_path = filedialog.asksaveasfilename(
            title="导出完整分析报告",
            defaultextension=".txt",
            filetypes=[
                ("文本文件", "*.txt"),    # 人类可读格式
                ("JSON文件", "*.json"),   # 结构化数据格式
                ("所有文件", "*.*")       # 其他格式
            ]
        )
        
        if file_path:
            try:
                if file_path.endswith('.json'):
                    # 导出JSON格式：结构化数据
                    export_data = {
                        "metadata": {
                            "生成时间": datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                            "分析器类型": self.analyzer_type.get(),
                            "文法": self.grammar_text.get(1.0, tk.END).strip()
                        },
                        "tables": self.current_data  # 包含ACTION表、GOTO表、项目集等
                    }
                    
                    # 如果有分析过程结果，也包含进去
                    process_content = self.process_text.get(1.0, tk.END).strip()
                    if process_content:
                        export_data["分析过程"] = process_content
                    
                    # 写入JSON文件，确保中文正确显示
                    with open(file_path, 'w', encoding='utf-8') as f:
                        json.dump(export_data, f, indent=2, ensure_ascii=False)
                else:
                    # 导出文本格式：完整的人类可读报告
                    with open(file_path, 'w', encoding='utf-8') as f:
                        # 报告标题和分隔线
                        f.write("=" * 60 + "\n")
                        f.write("             LR语法分析器完整报告\n")
                        f.write("=" * 60 + "\n\n")
                        
                        # 元信息
                        f.write(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                        f.write(f"分析器类型: {self.analyzer_type.get()}\n\n")
                        
                        # 文法规则部分
                        f.write("=" * 30 + " 文法规则 " + "=" * 30 + "\n")
                        grammar_content = self.grammar_text.get(1.0, tk.END).strip()
                        f.write(grammar_content)
                        f.write("\n\n")
                        
                        # 分析总结部分
                        f.write("=" * 30 + " 分析总结 " + "=" * 30 + "\n")
                        analysis_content = self.analysis_text.get(1.0, tk.END).strip()
                        if analysis_content:
                            f.write(analysis_content)
                        else:
                            f.write("暂无分析结果")
                        f.write("\n\n")
                        
                        # ACTION表部分
                        f.write("=" * 30 + " ACTION表 " + "=" * 30 + "\n")
                        action_content = self.action_text.get(1.0, tk.END).strip()
                        if action_content:
                            f.write(action_content)
                        else:
                            f.write("暂无ACTION表数据")
                        f.write("\n\n")
                        
                        # GOTO表部分
                        f.write("=" * 30 + " GOTO表 " + "=" * 30 + "\n")
                        goto_content = self.goto_text.get(1.0, tk.END).strip()
                        if goto_content:
                            f.write(goto_content)
                        else:
                            f.write("暂无GOTO表数据")
                        f.write("\n\n")
                        
                        # 项目集部分
                        f.write("=" * 30 + " 项目集 " + "=" * 30 + "\n")
                        itemsets_content = self.itemsets_text.get(1.0, tk.END).strip()
                        if itemsets_content:
                            f.write(itemsets_content)
                        else:
                            f.write("暂无项目集数据")
                        f.write("\n\n")
                        
                        # 分析过程部分
                        f.write("=" * 30 + " 分析过程 " + "=" * 30 + "\n")
                        process_content = self.process_text.get(1.0, tk.END).strip()
                        if process_content:
                            f.write(process_content)
                        else:
                            f.write("暂无分析过程数据\n")
                            f.write("提示: 请先执行'分析输入串'操作来生成分析过程")
                        f.write("\n\n")
                        
                        # 报告结束标记
                        f.write("=" * 60 + "\n")
                        f.write("           报告生成完成\n")
                        f.write("=" * 60 + "\n")
                        
                # 成功反馈
                self.update_status(f"完整报告已导出到: {os.path.basename(file_path)}")
                messagebox.showinfo("成功", f"完整分析报告已导出到:\n{file_path}")
                
            except Exception as e:
                # 错误处理
                messagebox.showerror("错误", f"无法导出报告: {str(e)}")
                
    def copy_text(self, text_widget):
        """复制文本到剪贴板"""
        try:
            content = text_widget.get(1.0, tk.END)
            self.root.clipboard_clear()
            self.root.clipboard_append(content)
            self.update_status("内容已复制到剪贴板")
        except Exception as e:
            messagebox.showerror("错误", f"复制失败: {str(e)}")
            
    def validate_grammar(self):
        """验证文法输入"""
        content = self.grammar_text.get(1.0, tk.END).strip()
        if not content:
            messagebox.showwarning("警告", "请输入文法规则")
            return False
        return True
    
    def format_error_message(self, error_msg):
        """
        格式化错误信息，提供用户友好的错误显示
        
        该方法将C++后端返回的原始错误信息转换为结构化、易理解的格式：
        1. 检测和解析不同类型的冲突（移进-归约、归约-归约）
        2. 提取状态和符号信息
        3. 添加错误原因分析和解决建议
        4. 包含分析器类型和时间戳等上下文信息
        
        支持的错误类型：
        - 移进/归约冲突：在某个状态对某个符号既可以移进又可以归约
        - 归约/归约冲突：在某个状态对某个符号可以用多个产生式归约
        - 其他语法分析错误
        
        Args:
            error_msg (str): C++后端返回的原始错误信息
            
        Returns:
            str: 格式化后的用户友好错误信息
        """
        formatted = f"错误: 分析表构造失败\n\n"
        
        # 检查是否包含冲突信息
        if "冲突" in error_msg or "conflict" in error_msg.lower():
            conflicts = []
            
            # 使用正则表达式解析移进-归约冲突
            # 匹配模式：包含"移进"、"归约"、"冲突"、"状态"、"符号"的文本
            sr_pattern = r"移进[/－-]归约冲突.*?状态\s*(\d+).*?符号\s*([^\s\n]+)"
            sr_matches = re.findall(sr_pattern, error_msg)
            for state, symbol in sr_matches:
                conflicts.append(f"  移进/归约冲突在状态 {state} 符号 {symbol}")
            
            # 使用正则表达式解析归约-归约冲突
            rr_pattern = r"归约[/－-]归约冲突.*?状态\s*(\d+).*?符号\s*([^\s\n]+)"
            rr_matches = re.findall(rr_pattern, error_msg)
            for state, symbol in rr_matches:
                conflicts.append(f"  归约/归约冲突在状态 {state} 符号 {symbol}")
            
            # 如果没有找到具体冲突模式，尝试其他方式提取信息
            if not conflicts:
                # 查找包含"状态"和数字的行
                state_lines = re.findall(r".*状态\s*(\d+).*", error_msg)
                for line in state_lines:
                    if "冲突" in line:
                        conflicts.append(f"  {line.strip()}")
            
            # 显示找到的冲突信息
            if conflicts:
                formatted += "✗ 检测到冲突：\n"
                formatted += "\n".join(conflicts)
                formatted += "\n\n"
            else:
                formatted += f"✗ 分析表构造失败\n\n"
        else:
            formatted += f"✗ 分析表构造失败\n\n"
        
        # 添加原始错误信息以供技术用户参考
        formatted += f"原始错误信息:\n{error_msg}\n\n"
        
        # 添加上下文信息
        formatted += f"分析器类型: {self.analyzer_type.get()}\n"
        formatted += f"构造时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        # 提供问题分析和解决建议
        formatted += "可能的原因:\n"
        formatted += "1. 文法不符合所选分析器类型的要求\n"
        formatted += "2. 文法存在语法错误或格式问题\n"
        formatted += "3. 文法存在冲突（如移进-归约冲突、归约-归约冲突）\n\n"
        formatted += "建议:\n"
        formatted += "- 检查文法规则的格式是否正确\n"
        formatted += "- 尝试使用更强的分析器类型（LR(0) < SLR(1) < LR(1)）\n"
        formatted += "- 检查文法是否存在左递归或其他问题\n"
        formatted += "- 如果是冲突问题，考虑重写文法规则"
        
        return formatted
        
    def update_status(self, message):
        """更新状态栏"""
        self.status_bar.config(text=message)
        self.root.update_idletasks()
        
    def show_progress(self):
        """显示进度条"""
        self.progress_bar.pack(fill='x', padx=15, pady=2)
        self.progress_bar.start()
        
    def hide_progress(self):
        """隐藏进度条"""
        self.progress_bar.stop()
        self.progress_bar.pack_forget()
        
    def show_error(self, message):
        """显示错误消息"""
        self.analysis_running = False
        messagebox.showerror("错误", message)
        self.update_status("操作失败")
        
    def on_tab_changed(self, event):
        """标签页切换事件"""
        selection = event.widget.select()
        tab_text = event.widget.tab(selection, "text")
        self.update_status(f"已切换到: {tab_text}")

def main():
    """
    主函数 - 应用程序入口点
    
    该函数负责应用程序的启动和初始化：
    1. 跨平台C++后端可执行文件检测
    2. 用户友好的错误提示和指导
    3. GUI窗口创建和居中显示
    4. 主事件循环启动和异常处理
    
    启动流程：
    检测C++后端 → 创建GUI → 居中窗口 → 启动事件循环
    
    平台兼容性：
    - Windows: 查找 lr_cli.exe 或 lr_cli
    - Linux/Unix: 查找 lr_cli 或 lr_cli.exe
    
    用户指导：
    如果未找到C++后端，提供具体的编译指令和继续选项
    """
    # 第一步：检测C++后端可执行文件 - 跨平台兼容
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(current_dir)
    
    # 根据操作系统确定可执行文件名和平台信息
    executable_names = []
    if os.name == 'nt':  # Windows系统
        executable_names = ["lr_cli.exe", "lr_cli"]
        platform_info = "Windows"
    else:  # Unix-like系统 (Linux, macOS)
        executable_names = ["lr_cli", "lr_cli.exe"]
        platform_info = "Linux/Unix"
    
    # 搜索可用的C++可执行文件
    cpp_executable = None
    for exe_name in executable_names:
        test_path = os.path.join(parent_dir, "Algorithm", exe_name)
        if os.path.exists(test_path):
            cpp_executable = test_path
            print(f"找到C++可执行文件: {exe_name} (平台: {platform_info})")
            break
    
    # 第二步：处理C++后端缺失的情况
    if not cpp_executable:
        print(f"警告: 找不到C++可执行文件 (平台: {platform_info})")
        print("请在Algorithm目录中运行构建命令来编译:")
        if os.name == 'nt':  # Windows
            print("  Windows: .\\build.bat 或 g++ -o lr_cli.exe ...")
        else:  # Unix-like
            print("  Linux/Unix: make 或 g++ -o lr_cli ...")
        
        # 询问用户是否继续启动GUI
        response = input("继续启动GUI吗? (y/n): ")
        if response.lower() != 'y':
            return
    
    # 第三步：创建主窗口和应用程序
    root = tk.Tk()
    app = CompleteLRGUI(root)
    
    # 第四步：窗口居中显示
    # 强制更新窗口尺寸信息
    root.update_idletasks()
    width = root.winfo_width()
    height = root.winfo_height()
    
    # 计算屏幕中心位置
    x = (root.winfo_screenwidth() // 2) - (width // 2)
    y = (root.winfo_screenheight() // 2) - (height // 2)
    
    # 设置窗口位置（居中显示）
    root.geometry(f"{width}x{height}+{x}+{y}")
    
    # 第五步：启动主事件循环
    try:
        root.mainloop()  # 进入GUI事件循环，等待用户交互
    except KeyboardInterrupt:
        print("\n程序被用户中断")  # 处理Ctrl+C中断

if __name__ == "__main__":
    main()
