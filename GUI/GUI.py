#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR语法分析器图形化界面 - 中文版
支持LR(0)、SLR(1)、LR(1)语法分析的完整功能
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, filedialog, font
import subprocess
import os
import sys
import threading
import json
import re
import tempfile
from datetime import datetime

class CompleteLRGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("LR Parser Analyzer") 
        self.root.geometry("1200x800")
        self.root.configure(bg='#f5f5f5')
        
        # Setup styles
        self.setup_styles()
        
        # Initialize variables
        self.grammar_file = None
        self.analyzer_type = tk.StringVar(value="SLR1")
        self.current_data = {}
        
        # Create main interface
        self.create_widgets()
        
        # Bind events
        self.bind_events()
        
        # Status info
        self.analysis_running = False
        
    def get_cpp_executable(self):
        """获取C++可执行文件路径 - 跨平台兼容"""
        # Try different executable names based on platform
        executable_names = []
        if os.name == 'nt':  # Windows
            executable_names = ["lr_cli.exe", "lr_cli"]
        else:  # Unix-like (Linux, macOS)
            executable_names = ["lr_cli", "lr_cli.exe"]
        
        for exe_name in executable_names:
            test_path = os.path.join(os.path.dirname(__file__), "..", "Algorithm", exe_name)
            if os.path.exists(test_path):
                return test_path
        
        return None
    
    def setup_styles(self):
        """设置界面样式"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Define Chinese font
        self.chinese_font = self.get_chinese_font()
        
        # Configure custom styles with Chinese font
        style.configure('Title.TLabel', 
                       font=(self.chinese_font, 18, 'bold'),
                       background='#f5f5f5',
                       foreground='#2c3e50')
        
        style.configure('Header.TLabel',
                       font=(self.chinese_font, 11, 'bold'),
                       background='#f5f5f5',
                       foreground='#34495e')
        
        style.configure('Action.TButton',
                       font=(self.chinese_font, 10, 'bold'),
                       padding=(15, 8))
        
        style.configure('Success.TLabel',
                       font=(self.chinese_font, 10),
                       background='#f5f5f5',
                       foreground='#27ae60')
        
        style.configure('Error.TLabel',
                       font=(self.chinese_font, 10),
                       background='#f5f5f5',
                       foreground='#e74c3c')
    
    def get_chinese_font(self):
        """获取可用的中文字体"""
        chinese_fonts = [
            'Noto Sans CJK SC',
            'WenQuanYi Zen Hei', 
            'WenQuanYi Micro Hei',
            'SimHei',
            'Microsoft YaHei',
            'Liberation Sans',
            'DejaVu Sans',
            'Sans'
        ]
        
        # 获取系统可用字体
        available_fonts = list(tk.font.families())
        
        # Test each font with Chinese characters
        for font_name in chinese_fonts:
            if font_name in available_fonts:
                try:
                    test_font = tk.font.Font(family=font_name, size=12)
                    # 测试中文字符显示
                    if self._test_chinese_font(test_font):
                        return font_name
                except:
                    continue
        
        # 如果没有找到合适的中文字体，使用系统默认字体
        print("警告: 未找到合适的中文字体，使用系统默认字体")
        return 'TkDefaultFont'
    
    def _test_chinese_font(self, font_obj):
        """测试字体是否支持中文显示"""
        try:
            # 创建临时测试组件
            test_label = tk.Label(self.root, text="测试", font=font_obj)
            test_label.pack()
            test_label.update_idletasks()
            test_label.destroy()
            return True
        except:
            return False
        
    def create_widgets(self):
        """创建界面组件"""
        # Create main title
        self.create_header()
        
        # Create main content area
        self.create_main_area()
        
        # Create status bar
        self.create_status_bar()
        
        # Load default grammar (after all components are created)
        self.load_example_grammar()
        
    def create_header(self):
        """创建顶部标题区域"""
        header_frame = ttk.Frame(self.root)
        header_frame.pack(fill='x', padx=15, pady=10)
        # Main title
        title_label = ttk.Label(header_frame,
                       text="LR语法分析器",
                       font=(self.chinese_font, 17, 'bold'),
                       style='Title.TLabel')
        title_label.pack(side='left')
        # Version info
        version_label = ttk.Label(header_frame, 
                                 text="v2.0 - 支持 LR(0) / SLR(1) / LR(1)", 
                                 font=('Arial', 9),
                                 foreground='#7f8c8d')
        version_label.pack(side='right')
        
    def create_main_area(self):
        """创建主要内容区域"""
        # Create horizontal split window
        main_paned = ttk.PanedWindow(self.root, orient='horizontal')
        main_paned.pack(fill='both', expand=True, padx=15, pady=5)
        
        # Left control panel
        left_frame = ttk.Frame(main_paned)
        main_paned.add(left_frame, weight=1)
        
        # Right result panel
        right_frame = ttk.Frame(main_paned)
        main_paned.add(right_frame, weight=3)
        
        # Create control panel
        self.create_control_panel(left_frame)
        
        # Create result panel
        self.create_result_panel(right_frame)
        
    def create_control_panel(self, parent):
        """创建左侧控制面板"""
        # Create a scrollable frame for the control panel
        canvas = tk.Canvas(parent, bg='#f5f5f5')
        scrollbar = ttk.Scrollbar(parent, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)
        
        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )
        
        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)
        
        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        
        # Grammar input area
        self.create_grammar_section(scrollable_frame)
        
        # Input string area
        self.create_input_section(scrollable_frame)
        
        # Analyzer type selection
        self.create_analyzer_section(scrollable_frame)
        
        # Operation buttons area
        self.create_operation_section(scrollable_frame)
        
        # Bind mousewheel to canvas
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1*(event.delta/120)), "units")
        canvas.bind_all("<MouseWheel>", _on_mousewheel)
        
    def create_grammar_section(self, parent):
        """创建文法输入区域"""
        grammar_frame = ttk.LabelFrame(parent, text="文法输入", padding=15)
        grammar_frame.pack(fill='x', padx=5, pady=5)
        
        # Grammar operation buttons
        btn_frame = ttk.Frame(grammar_frame)
        btn_frame.pack(fill='x', pady=(0, 10))
        
        ttk.Button(btn_frame, text="加载文件", 
                  command=self.load_grammar_file,
                  style='Action.TButton').pack(side='left', padx=(0, 5))
        
        ttk.Button(btn_frame, text="保存文件", 
                  command=self.save_grammar_file,
                  style='Action.TButton').pack(side='left', padx=5)
        
        ttk.Button(btn_frame, text="示例文法", 
                  command=self.load_example_grammar,
                  style='Action.TButton').pack(side='left', padx=5)
        
        # Grammar input description
        ttk.Label(grammar_frame, text="文法规则（每行一个产生式）:", 
                 style='Header.TLabel').pack(anchor='w', pady=(0, 5))
        
        # Grammar input text box
        self.grammar_text = scrolledtext.ScrolledText(
            grammar_frame, 
            height=10, 
            width=45,
            font=(self.chinese_font, 11), 
            wrap='none',
            bg='white',
            selectbackground='#3498db',
            selectforeground='white')
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
        """创建右侧结果面板"""
        # Create Notebook tabs
        self.notebook = ttk.Notebook(parent)
        self.notebook.pack(fill='both', expand=True, padx=5, pady=5)
        
        # Analysis result tab
        self.create_analysis_tab()
        
        # ACTION table tab
        self.create_action_tab()
        
        # GOTO table tab
        self.create_goto_tab()
        
        # Item sets tab
        self.create_itemsets_tab()
        
        # Analysis process tab
        self.create_process_tab()
        
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
        """绑定事件"""
        # Hotkeys
        self.root.bind('<Control-o>', lambda e: self.load_grammar_file())
        self.root.bind('<Control-s>', lambda e: self.save_grammar_file())
        self.root.bind('<F5>', lambda e: self.construct_table())
        self.root.bind('<F6>', lambda e: self.analyze_input())
        self.root.bind('<F7>', lambda e: self.show_item_sets())
        
        # Enter key analysis
        self.input_entry.bind('<Return>', lambda e: self.analyze_input())
        
        # Tab switching
        self.notebook.bind('<<NotebookTabChanged>>', self.on_tab_changed)
        
    def load_grammar_file(self):
        """加载文法文件"""
        file_path = filedialog.askopenfilename(
            title="选择文法文件",
            filetypes=[
                ("文本文件", "*.txt"),
                ("所有文件", "*.*")
            ],
            initialdir=os.path.join(os.path.dirname(__file__))
        )
        
        if file_path:
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                self.grammar_text.delete(1.0, tk.END)
                self.grammar_text.insert(1.0, content)
                self.grammar_file = file_path
                self.update_status(f"已加载文法文件: {os.path.basename(file_path)}")
            except Exception as e:
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
        example_grammar = """E -> E + T
E -> T
T -> T * F
T -> F
F -> ( E )
F -> a"""
        
        self.grammar_text.delete(1.0, tk.END)
        self.grammar_text.insert(1.0, example_grammar)
        self.update_status("已加载示例文法")
        
    def construct_table(self):
        """构造分析表"""
        if not self.validate_grammar():
            return
            
        self.analysis_running = True
        self.update_status("正在构造分析表...")
        self.show_progress()
        
        # Execute in new thread
        threading.Thread(target=self._construct_table_worker, daemon=True).start()
        
    def _construct_table_worker(self):
        """构造分析表工作线程"""
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
            
            # Run command to get JSON data
            cmd = [cpp_executable, temp_grammar_file, "-t", analyzer_type, "--table", "--json"]
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8')
            
            # Update interface in main thread
            self.root.after(0, lambda: self._update_table_results(result))
            
        except Exception as e:
            self.root.after(0, lambda: self.show_error(f"构造分析表出错: {str(e)}"))
        finally:
            self.root.after(0, self.hide_progress)
            
    def _update_table_results(self, result):
        """更新分析表结果"""
        self.analysis_running = False
        
        if result.returncode == 0:
            try:
                # Parse JSON data
                output = result.stdout.strip()
                self.current_data = json.loads(output)
                
                # Update each tab
                self.display_action_table()
                self.display_goto_table()
                self.display_item_sets()
                
                # Update analysis result summary
                self.display_analysis_summary()
                
                self.update_status("分析表构造完成")
                self.notebook.select(self.action_frame)
                
            except json.JSONDecodeError as e:
                self.show_error(f"解析JSON数据失败: {str(e)}")
        else:
            error_msg = result.stderr or result.stdout
            formatted_error = self.format_error_message(error_msg)
            self.show_error(f"分析表构造失败:\n{formatted_error}")
            
            # Also display detailed error in analysis result tab
            self.analysis_text.delete(1.0, tk.END)
            self.analysis_text.insert(tk.END, formatted_error)
            
            # Switch to analysis result tab to show error
            self.notebook.select(self.analysis_frame)
            
    def display_action_table(self):
        """显示ACTION表"""
        if 'action_table' not in self.current_data:
            return
            
        self.action_text.delete(1.0, tk.END)
        
        action_table = self.current_data['action_table']
        
        # Get all terminals
        terminals = set()
        for state_actions in action_table.values():
            terminals.update(state_actions.keys())
        terminals = sorted(list(terminals))
        
        # Create header
        header = "状态\\符号".ljust(12)
        for terminal in terminals:
            header += terminal.ljust(10)
        self.action_text.insert(tk.END, header + "\n")
        self.action_text.insert(tk.END, "=" * len(header) + "\n")
        
        # Create table content
        states = sorted([int(state) for state in action_table.keys()])
        for state in states:
            row = str(state).ljust(12)
            state_actions = action_table.get(str(state), {})
            
            for terminal in terminals:
                action = state_actions.get(terminal, "")
                row += action.ljust(10)
            
            self.action_text.insert(tk.END, row + "\n")
            
        # Add explanation
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
        """导出完整分析报告"""
        file_path = filedialog.asksaveasfilename(
            title="导出完整分析报告",
            defaultextension=".txt",
            filetypes=[
                ("文本文件", "*.txt"),
                ("JSON文件", "*.json"),
                ("所有文件", "*.*")
            ]
        )
        
        if file_path:
            try:
                if file_path.endswith('.json'):
                    # Export JSON format
                    export_data = {
                        "metadata": {
                            "生成时间": datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                            "分析器类型": self.analyzer_type.get(),
                            "文法": self.grammar_text.get(1.0, tk.END).strip()
                        },
                        "tables": self.current_data
                    }
                    
                    # Add analysis process if available
                    process_content = self.process_text.get(1.0, tk.END).strip()
                    if process_content:
                        export_data["分析过程"] = process_content
                    
                    with open(file_path, 'w', encoding='utf-8') as f:
                        json.dump(export_data, f, indent=2, ensure_ascii=False)
                else:
                    # Export comprehensive text format
                    with open(file_path, 'w', encoding='utf-8') as f:
                        # Header
                        f.write("=" * 60 + "\n")
                        f.write("             LR语法分析器完整报告\n")
                        f.write("=" * 60 + "\n\n")
                        
                        f.write(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                        f.write(f"分析器类型: {self.analyzer_type.get()}\n\n")
                        
                        # Grammar section
                        f.write("=" * 30 + " 文法规则 " + "=" * 30 + "\n")
                        grammar_content = self.grammar_text.get(1.0, tk.END).strip()
                        f.write(grammar_content)
                        f.write("\n\n")
                        
                        # Analysis summary
                        f.write("=" * 30 + " 分析总结 " + "=" * 30 + "\n")
                        analysis_content = self.analysis_text.get(1.0, tk.END).strip()
                        if analysis_content:
                            f.write(analysis_content)
                        else:
                            f.write("暂无分析结果")
                        f.write("\n\n")
                        
                        # ACTION table
                        f.write("=" * 30 + " ACTION表 " + "=" * 30 + "\n")
                        action_content = self.action_text.get(1.0, tk.END).strip()
                        if action_content:
                            f.write(action_content)
                        else:
                            f.write("暂无ACTION表数据")
                        f.write("\n\n")
                        
                        # GOTO table
                        f.write("=" * 30 + " GOTO表 " + "=" * 30 + "\n")
                        goto_content = self.goto_text.get(1.0, tk.END).strip()
                        if goto_content:
                            f.write(goto_content)
                        else:
                            f.write("暂无GOTO表数据")
                        f.write("\n\n")
                        
                        # Item sets
                        f.write("=" * 30 + " 项目集 " + "=" * 30 + "\n")
                        itemsets_content = self.itemsets_text.get(1.0, tk.END).strip()
                        if itemsets_content:
                            f.write(itemsets_content)
                        else:
                            f.write("暂无项目集数据")
                        f.write("\n\n")
                        
                        # Analysis process
                        f.write("=" * 30 + " 分析过程 " + "=" * 30 + "\n")
                        process_content = self.process_text.get(1.0, tk.END).strip()
                        if process_content:
                            f.write(process_content)
                        else:
                            f.write("暂无分析过程数据\n")
                            f.write("提示: 请先执行'分析输入串'操作来生成分析过程")
                        f.write("\n\n")
                        
                        # Footer
                        f.write("=" * 60 + "\n")
                        f.write("           报告生成完成\n")
                        f.write("=" * 60 + "\n")
                        
                self.update_status(f"完整报告已导出到: {os.path.basename(file_path)}")
                messagebox.showinfo("成功", f"完整分析报告已导出到:\n{file_path}")
            except Exception as e:
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
        """格式化错误信息，美化显示"""
        formatted = f"错误: 分析表构造失败\n\n"
        
        # 检查是否包含冲突信息
        if "冲突" in error_msg or "conflict" in error_msg.lower():
            conflicts = []
            
            # 解析移进-归约冲突
            sr_pattern = r"移进[/－-]归约冲突.*?状态\s*(\d+).*?符号\s*([^\s\n]+)"
            sr_matches = re.findall(sr_pattern, error_msg)
            for state, symbol in sr_matches:
                conflicts.append(f"  移进/归约冲突在状态 {state} 符号 {symbol}")
            
            # 解析归约-归约冲突
            rr_pattern = r"归约[/－-]归约冲突.*?状态\s*(\d+).*?符号\s*([^\s\n]+)"
            rr_matches = re.findall(rr_pattern, error_msg)
            for state, symbol in rr_matches:
                conflicts.append(f"  归约/归约冲突在状态 {state} 符号 {symbol}")
            
            # 如果没有找到具体冲突，尝试其他模式
            if not conflicts:
                # 查找包含"状态"和数字的行
                state_lines = re.findall(r".*状态\s*(\d+).*", error_msg)
                for line in state_lines:
                    if "冲突" in line:
                        conflicts.append(f"  {line.strip()}")
            
            if conflicts:
                formatted += "✗ 检测到冲突：\n"
                formatted += "\n".join(conflicts)
                formatted += "\n\n"
            else:
                formatted += f"✗ 分析表构造失败\n\n"
        else:
            formatted += f"✗ 分析表构造失败\n\n"
        
        # 添加原始错误信息
        formatted += f"原始错误信息:\n{error_msg}\n\n"
        
        # 添加分析器信息
        formatted += f"分析器类型: {self.analyzer_type.get()}\n"
        formatted += f"构造时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
        
        # 添加建议
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
    """主函数"""
    # Check C++ executable - cross-platform detection
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(current_dir)
    
    # Try different executable names based on platform
    executable_names = []
    if os.name == 'nt':  # Windows
        executable_names = ["lr_cli.exe", "lr_cli"]
        platform_info = "Windows"
    else:  # Unix-like (Linux, macOS)
        executable_names = ["lr_cli", "lr_cli.exe"]
        platform_info = "Linux/Unix"
    
    cpp_executable = None
    for exe_name in executable_names:
        test_path = os.path.join(parent_dir, "Algorithm", exe_name)
        if os.path.exists(test_path):
            cpp_executable = test_path
            print(f"找到C++可执行文件: {exe_name} (平台: {platform_info})")
            break
    
    if not cpp_executable:
        print(f"警告: 找不到C++可执行文件 (平台: {platform_info})")
        print("请在Algorithm目录中运行构建命令来编译:")
        if os.name == 'nt':  # Windows
            print("  Windows: .\\build.bat 或 g++ -o lr_cli.exe ...")
        else:  # Unix-like
            print("  Linux/Unix: make 或 g++ -o lr_cli ...")
        response = input("继续启动GUI吗? (y/n): ")
        if response.lower() != 'y':
            return
    
    # Create main window
    root = tk.Tk()
    app = CompleteLRGUI(root)
    
    # Center the window
    root.update_idletasks()
    width = root.winfo_width()
    height = root.winfo_height()
    x = (root.winfo_screenwidth() // 2) - (width // 2)
    y = (root.winfo_screenheight() // 2) - (height // 2)
    root.geometry(f"{width}x{height}+{x}+{y}")
    
    try:
        root.mainloop()
    except KeyboardInterrupt:
        print("\n程序被用户中断")

if __name__ == "__main__":
    main()
