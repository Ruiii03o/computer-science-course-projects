import random
import time
import tkinter as tk
from tkinter import messagebox
import threading
import math
import pygame  # 用于音效播放

# 初始化pygame音频系统
pygame.mixer.init()


class Particle:
    """基础粒子类，用于创建各种动画特效"""

    def __init__(self, x, y, color, size=3):
        self.x = x
        self.y = y
        self.color = color
        self.size = size
        self.speed_x = random.uniform(-2, 2)
        self.speed_y = random.uniform(-2, 2)
        self.life = random.uniform(0.5, 1.5)  # 粒子生命周期
        self.max_life = self.life
        self.gravity = random.uniform(0.05, 0.1)  # 粒子重力效果

    def update(self, dt):
        """更新粒子状态"""
        self.x += self.speed_x
        self.y += self.speed_y
        self.speed_y += self.gravity  # 应用重力
        self.life -= dt

    def is_alive(self):
        """判断粒子是否存活"""
        return self.life > 0

    def get_alpha_color(self):
        """获取带透明度的颜色"""
        alpha = int(255 * (self.life / self.max_life))
        r, g, b = self.color
        return f"#{r:02x}{g:02x}{b:02x}"


class FireworkParticle:
    """烟花粒子类，专门用于烟花效果"""

    def __init__(self, x, y, color):
        self.x = x
        self.y = y
        self.color = color
        self.size = random.randint(2, 4)

        # 随机方向和速度
        angle = random.uniform(0, 2 * math.pi)
        speed = random.uniform(1, 6)
        self.speed_x = math.cos(angle) * speed
        self.speed_y = math.sin(angle) * speed

        # 生命周期更长，带阻尼效果
        self.life = random.uniform(1.0, 2.0)
        self.max_life = self.life
        self.damping = 0.96  # 阻尼系数，使粒子逐渐减速
        self.gravity = 0.08  # 重力效果

    def update(self, dt):
        """更新烟花粒子状态"""
        # 应用阻尼
        self.speed_x *= self.damping
        self.speed_y *= self.damping

        # 应用重力
        self.speed_y += self.gravity

        # 更新位置
        self.x += self.speed_x
        self.y += self.speed_y

        # 减少生命值
        self.life -= dt

    def is_alive(self):
        """判断粒子是否存活"""
        return self.life > 0

    def get_alpha_color(self):
        """获取带透明度的颜色"""
        alpha = int(255 * (self.life / self.max_life))
        r, g, b = self.color
        return f"#{r:02x}{g:02x}{b:02x}"


class AnswerBookApp:
    def __init__(self, root):
        # 主窗口设置
        self.root = root
        self.root.title("答案之书")
        self.root.geometry("750x750")
        self.root.resizable(True, True)

        # 主题配色
        self.theme = {
            "bg": "#f5f1e6",  # 浅米色背景
            "text": "#704214",  # 棕褐色文字
            "button": "#d9b38c",  # 按钮颜色
            "entry": "#ffffff",  # 输入框背景
            "answer_bg": "#fffaf0",  # 答案区域背景
            "container": "#8b5a2b",  # 签筒颜色
            "container_light": "#a0522d",  # 签筒亮部
            "container_dark": "#6d4c41",  # 签筒暗部
            "stick": "#fff8e1",  # 签的颜色
            "stick_border": "#e0d6c0",  # 签的边框
            "red_dot": "#cc0000",  # 签上红点颜色
            "merit_text": "#006600",  # 功德文字颜色
            "pray_text": "#8B4513"  # 祈福文字颜色
        }
        self.root.configure(bg=self.theme["bg"])

        # 字体设置
        self.title_font = ("华文楷体", 24, "bold")
        self.normal_font = ("微软雅黑", 12)
        self.answer_font = ("华文行楷", 16, "italic")
        self.wait_font = ("微软雅黑", 16, "bold")
        self.drawing_font = ("微软雅黑", 20, "bold")  # 抽签中文字体
        self.merit_font = ("华文楷体", 16, "bold")  # 功德提示字体
        self.pray_font = ("微软雅黑", 10, "italic")  # 祈福文字字体

        # 动画控制变量
        self.animation_running = False
        self.phase = 0  # 0: 显示"抽签中", 1: 筒内摇晃, 2: 签筒消失与签出现
        self.animation_step = 0
        self.shake_offset = 0  # 签筒摇晃偏移量
        self.container_alpha = 1.0  # 签筒透明度
        self.selected_stick_alpha = 0.0  # 选中签的透明度
        self.other_sticks_alpha = 1.0  # 其他签的透明度
        self.last_animation_time = time.time()  # 用于粒子动画时间计算

        # 粒子效果变量
        self.particles = []
        self.emit_particles = False  # 是否发射粒子

        # 电子木鱼相关变量
        self.wooden_fish_animating = False
        self.wooden_fish_angle = 0
        self.merit_text_index = 0
        self.merit_texts = ["今日功德+1", "厄运-1"]
        self.wooden_fish_start_time = 0  # 初始化木鱼动画开始时间

        # 烟花效果变量
        self.fireworks = []  # 存储烟花粒子
        self.firework_active = False  # 烟花是否激活

        # 多根签的位置偏移
        self.stick_offsets = [0, 2, -3, 5, -2, 4, -5]  # 模拟签在筒内的自然分布

        # 保存原始答案框高度，用于动画时调整
        self.original_answer_height = 6

        # 当前答案存储
        self.current_answer = ""

        # 加载音效
        self.sounds = self.load_sounds()
        self.create_widgets()

        # 答案库 - 分类为积极和消极答案
        self.answers = {
            "positive": [
                "是的，毫无疑问",
                "很可能会有好结果，值得期待",
                "有很大几率如你所愿",
                "未来一片光明，大胆去做吧",
                "你的努力会得到回报",
                "机会就在眼前，抓住它",
                "这是一个明智的选择",
                "放心吧，一切都会顺利",
                "你的直觉非常准确",
                "坚持下去，成功就在前方",
                "好运会眷顾你的",
                "这件事会给你带来惊喜",
                "周围的人会支持你的决定",
                "时机正好，不要犹豫",
                "你的能力足以应对挑战",
                "这将是一个新的开始",
                "付出就会有收获",
                "积极的态度会带来积极的结果",
                "你已经走在正确的道路上",
                "相信自己，你能做到",
                "这件事会超出你的预期",
                "宇宙正在为你准备惊喜",
                "你的努力即将开花结果",
                "静候佳音",
                "有一定几率"
            ],
            "negative": [
                "可能不会如你所愿",
                "前景不太乐观",
                "别抱太大希望",
                "目前看来困难重重",
                "这不是一个好时机",
                "可能会遇到很多阻碍",
                "结果可能会令人失望",
                "风险大于收益",
                "或许应该重新考虑",
                "成功的可能性很低",
                "可能会浪费时间和精力",
                "形势对你不利",
                "这不是明智的选择",
                "阻碍比你想象的要多",
                "可能会有不好的后果",
                "时机尚未成熟",
                "困难会比预期的多",
                "不建议继续推进",
                "你可能低估了挑战",
                "这可能是个错误的决定"
            ]
        }

    def load_sounds(self):
        """加载音效文件，如无法加载则使用空对象避免错误"""
        sounds = {}
        try:
            # 尝试加载音效，如没有音效文件则跳过
            sounds["shake"] = pygame.mixer.Sound("shake.wav")  # 摇晃音效
            sounds["draw"] = pygame.mixer.Sound("draw.wav")  # 抽签音效
            sounds["reveal"] = pygame.mixer.Sound("reveal.wav")  # 揭示答案音效
            sounds["wooden_fish"] = pygame.mixer.Sound("wooden_fish.wav")  # 木鱼音效
            sounds["firework"] = pygame.mixer.Sound("firework.wav")  # 烟花音效
        except:
            # 如果音效文件不存在，创建空方法避免错误
            class DummySound:
                def play(self):
                    pass

            sounds["shake"] = DummySound()
            sounds["draw"] = DummySound()
            sounds["reveal"] = DummySound()
            sounds["wooden_fish"] = DummySound()
            sounds["firework"] = DummySound()
        return sounds

    def create_widgets(self):
        # 创建烟花画布（覆盖整个窗口，用于显示烟花效果）
        self.firework_canvas = tk.Canvas(
            self.root, bg=self.theme["bg"], highlightthickness=0
        )
        self.firework_canvas.place(x=0, y=0, relwidth=1, relheight=1)

        # 标题区域
        title_frame = tk.Frame(self.root, bg=self.theme["bg"])
        title_frame.pack(pady=20)
        tk.Label(
            title_frame, text="答案之书", font=self.title_font,
            bg=self.theme["bg"], fg=self.theme["text"]
        ).pack()

        # 问题输入区域
        input_frame = tk.Frame(self.root, bg=self.theme["bg"])
        input_frame.pack(padx=30, fill=tk.X, pady=(0, 10))

        tk.Label(
            input_frame, text="请输入你的问题：", font=self.normal_font,
            bg=self.theme["bg"], fg=self.theme["text"]
        ).pack(anchor=tk.W, pady=5)

        self.question_entry = tk.Text(
            input_frame, height=3, font=self.normal_font, wrap=tk.WORD,
            bd=2, relief=tk.SUNKEN, padx=10, pady=10,
            bg=self.theme["entry"], fg=self.theme["text"],
            highlightbackground=self.theme["button"],
            highlightcolor=self.theme["button"],
            highlightthickness=1
        )
        self.question_entry.pack(fill=tk.X)

        # 按钮区域
        button_frame = tk.Frame(self.root, bg=self.theme["bg"])
        button_frame.pack(pady=15)

        self.ask_button = tk.Button(
            button_frame, text="获取答案", font=("微软雅黑", 12, "bold"),
            command=self.start_process, bg=self.theme["button"], fg="white",
            padx=25, pady=8, relief=tk.FLAT, cursor="hand2",
            activebackground="#c0a080", activeforeground="white", bd=0
        )
        self.ask_button.pack(side=tk.LEFT, padx=10)
        self.ask_button.bind("<Enter>", lambda e: self.ask_button.config(bg="#c0a080"))
        self.ask_button.bind("<Leave>", lambda e: self.ask_button.config(bg=self.theme["button"]))

        tk.Button(
            button_frame, text="退出", font=("微软雅黑", 12, "bold"),
            command=self.root.quit, bg=self.theme["button"], fg="white",
            padx=25, pady=8, relief=tk.FLAT, cursor="hand2",
            activebackground="#c0a080", activeforeground="white", bd=0
        ).pack(side=tk.LEFT, padx=10)

        # 答案和木鱼效果组合区域
        self.main_content_frame = tk.Frame(self.root, bg=self.theme["bg"])
        self.main_content_frame.pack(padx=30, pady=10, fill=tk.BOTH, expand=True)

        # 答案显示区域（左侧）
        self.answer_frame = tk.Frame(self.main_content_frame, bg=self.theme["bg"])
        self.answer_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 15))

        tk.Label(
            self.answer_frame, text="答案：", font=self.normal_font,
            bg=self.theme["bg"], fg=self.theme["text"]
        ).pack(anchor=tk.W, pady=5)

        # 答案框容器（用于叠加木鱼画布）
        self.answer_container = tk.Frame(self.answer_frame, bg=self.theme["answer_bg"], bd=2, relief=tk.SUNKEN)
        self.answer_container.pack(fill=tk.BOTH, expand=True)

        # 答案文本框（放在容器内）
        self.answer_text = tk.Text(
            self.answer_container, height=self.original_answer_height, font=self.answer_font, wrap=tk.WORD,
            bd=0, padx=15, pady=15,
            bg=self.theme["answer_bg"], fg=self.theme["text"],
            state=tk.DISABLED, highlightthickness=0
        )
        self.answer_text.pack(fill=tk.BOTH, expand=True)

        # 创建木鱼和祈福文字的组合容器 - 初始完全隐藏
        self.wooden_fish_group = tk.Frame(
            self.answer_container, bg=self.theme["answer_bg"], highlightthickness=0
        )
        self.wooden_fish_group.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
        self.wooden_fish_group.lower()  # 放在答案文本下方
        self.wooden_fish_group.pack_forget()  # 确保初始不显示

        # 祈福文字标签（放置在木鱼上方）
        self.pray_label = tk.Label(
            self.wooden_fish_group, text="电子木鱼祈福中...", font=self.pray_font,
            bg=self.theme["answer_bg"], fg=self.theme["pray_text"],
            bd=0, highlightthickness=0
        )
        self.pray_label.pack(pady=(0, 10))  # 文字在上方，与木鱼保持间距

        # 电子木鱼画布（放置在祈福文字下方）
        self.wooden_fish_canvas = tk.Canvas(
            self.wooden_fish_group, width=180, height=180,
            bg=self.theme["answer_bg"], highlightthickness=0
        )
        self.wooden_fish_canvas.pack()

        # 功德标签 - 增加高度和内边距确保文字完整显示
        self.merit_label = tk.Label(
            self.answer_frame, text="", font=self.merit_font,
            bg=self.theme["answer_bg"], fg=self.theme["merit_text"],
            bd=1, relief=tk.SUNKEN, padx=15, pady=10,  # 增加上下内边距
            height=2,  # 明确设置高度
            highlightbackground=self.theme["button"], highlightthickness=1,
            wraplength=400  # 防止文字过长溢出
        )
        self.merit_label.pack(pady=5, fill=tk.X)
        self.merit_label.pack_forget()  # 初始隐藏

        # 抽签中提示（放在答案框下方）
        self.drawing_frame = tk.Frame(self.root, bg=self.theme["bg"])
        self.drawing_label = tk.Label(
            self.drawing_frame, text="抽签中...", font=self.drawing_font,
            bg=self.theme["bg"], fg=self.theme["text"]
        )
        # 初始不显示

        # 动画区域
        self.animation_frame = tk.Frame(self.root, bg=self.theme["bg"])
        self.animation_canvas = tk.Canvas(
            self.animation_frame, width=300, height=250,
            bg=self.theme["bg"], highlightthickness=0
        )
        self.animation_canvas.pack()

    def start_process(self):
        """开始整个流程：验证输入 -> 显示动画 -> 生成答案"""
        question = self.question_entry.get("1.0", tk.END).strip()
        if not question:
            messagebox.showwarning("提示", "请输入你的问题")
            return

        # 强制隐藏所有相关元素，确保每次查询都从干净状态开始
        self.wooden_fish_group.pack_forget()
        self.pray_label.pack_forget()  # 单独隐藏祈福文字
        self.wooden_fish_canvas.pack_forget()  # 单独隐藏木鱼
        self.merit_label.pack_forget()
        self.wooden_fish_animating = False
        self.fireworks = []
        self.firework_active = False

        # 清空之前的答案
        self.answer_text.config(state=tk.NORMAL)
        self.answer_text.delete("1.0", tk.END)
        self.answer_text.config(state=tk.DISABLED)

        # 随机选择一根签作为选中的签
        self.selected_stick_index = random.randint(0, len(self.stick_offsets) - 1)

        # 显示"抽签中"提示和动画区域，禁用按钮
        self.drawing_frame.pack(pady=5)
        self.drawing_label.pack()
        self.animation_frame.pack(pady=10)
        self.ask_button.config(state=tk.DISABLED)

        # 缩小答案框以便更好地展示动画
        self.answer_text.config(height=2)

        # 重置粒子系统
        self.particles = []
        self.emit_particles = False

        self.animation_running = True
        self.phase = 0
        self.animation_step = 0
        self.last_animation_time = time.time()
        self.update_animation()

        # 启动答案生成线程
        threading.Thread(target=self.generate_answer, daemon=True).start()

    def update_animation(self):
        """更新动画帧，控制动画流程"""
        if not self.animation_running:
            return

        # 计算时间差用于粒子动画
        current_time = time.time()
        dt = current_time - self.last_animation_time
        self.last_animation_time = current_time

        self.animation_canvas.delete("all")  # 清除上一帧

        # 更新和绘制粒子
        self.update_particles(dt)
        self.draw_particles()

        if self.phase == 0:
            # 第一阶段：仅显示"抽签中"文字（1秒）
            self.animation_step += 1
            if self.animation_step >= 20:  # 20 * 50ms = 1秒
                self.phase = 1
                self.animation_step = 0
                # 开始播放摇晃音效
                self.sounds["shake"].play()

        elif self.phase == 1:
            # 第二阶段：签在筒内摇晃（2秒）- 显示多根签
            # 计算摇晃偏移量（使用正弦函数使摇晃更自然）
            shake_intensity = max(0, 15 - (self.animation_step * 0.07))  # 逐渐减弱
            self.shake_offset = math.sin(self.animation_step * 0.4) * shake_intensity

            # 随机发射一些粒子增强效果
            if random.random() < 0.3:  # 30%概率发射粒子
                self.emit_particle(150 + self.shake_offset, 60)  # 从签筒顶部发射

            # 绘制长方形签筒（带摇晃效果）
            self.draw_container()

            # 绘制多根签
            self.draw_multiple_sticks()

            self.animation_step += 1
            if self.animation_step >= 40:  # 40 * 50ms = 2秒
                self.phase = 2
                self.animation_step = 0
                self.container_alpha = 1.0
                self.selected_stick_alpha = 0.0
                self.other_sticks_alpha = 1.0
                # 播放抽签音效
                self.sounds["draw"].play()
                # 开始发射更多粒子
                self.emit_particles = True

        elif self.phase == 2:
            # 第三阶段：签筒消失，显示抽出的一根签（1.5秒）
            # 签筒透明度降低
            self.container_alpha = max(0, self.container_alpha - 0.07)

            # 其他签透明度降低
            self.other_sticks_alpha = max(0, self.other_sticks_alpha - 0.07)

            # 选中的签透明度增加并上升
            self.selected_stick_alpha = min(1, self.selected_stick_alpha + 0.07)

            # 发射粒子效果
            if self.emit_particles and random.random() < 0.5:
                self.emit_particle(150, 150)  # 从签周围发射

            # 绘制正在消失的签筒
            if self.container_alpha > 0:
                self.draw_container()

            # 绘制多根签（部分消失）
            self.draw_multiple_sticks()

            # 绘制选中的签（逐渐显现并上升）
            self.draw_selected_stick()

            self.animation_step += 1
            if self.animation_step >= 30:  # 30 * 50ms = 1.5秒
                self.animation_running = False
                # 播放揭示答案音效
                self.sounds["reveal"].play()
                return

        # 继续下一帧动画
        self.root.after(50, self.update_animation)

    def emit_particle(self, x, y):
        """发射新粒子"""
        # 随机粒子颜色（暖色调为主）
        colors = [
            (255, 220, 180),  # 浅金色
            (255, 200, 150),  # 金色
            (255, 180, 120),  # 橙金色
            (255, 160, 100)  # 深金色
        ]
        color = random.choice(colors)
        # 添加多个粒子形成效果
        for _ in range(random.randint(2, 5)):
            self.particles.append(
                Particle(
                    x + random.uniform(-10, 10),
                    y + random.uniform(-10, 10),
                    color,
                    size=random.uniform(2, 4)
                )
            )

    def update_particles(self, dt):
        """更新所有粒子状态"""
        # 更新粒子并过滤掉已死亡的粒子
        self.particles = [p for p in self.particles if p.is_alive()]
        for particle in self.particles:
            particle.update(dt)

    def draw_particles(self):
        """绘制所有粒子"""
        for particle in self.particles:
            if particle.is_alive():
                self.animation_canvas.create_oval(
                    particle.x - particle.size,
                    particle.y - particle.size,
                    particle.x + particle.size,
                    particle.y + particle.size,
                    fill=particle.get_alpha_color(),
                    outline=""
                )

    def draw_container(self):
        """绘制长方形签筒，带有立体感"""
        # 签筒位置和尺寸
        x, y = 150 + self.shake_offset, 150
        width, height = 60, 180

        # 创建半透明颜色
        def get_alpha_color(base_color, alpha):
            r = int(base_color[1:3], 16)
            g = int(base_color[3:5], 16)
            b = int(base_color[5:7], 16)
            return f"#{int(r * alpha):02x}{int(g * alpha):02x}{int(b * alpha):02x}"

        # 签筒主体（使用三个矩形模拟立体感）
        # 暗面
        self.animation_canvas.create_rectangle(
            x - width // 2, y - height // 2,
            x + width // 2, y + height // 2,
            fill=get_alpha_color(self.theme["container_dark"], self.container_alpha), outline=""
        )
        # 正面
        self.animation_canvas.create_rectangle(
            x - width // 2, y - height // 2,
            x + width // 2 - 5, y + height // 2 - 5,
            fill=get_alpha_color(self.theme["container"], self.container_alpha), outline=""
        )
        # 亮面
        self.animation_canvas.create_rectangle(
            x + width // 2 - 5, y - height // 2,
            x + width // 2, y + height // 2 - 5,
            fill=get_alpha_color(self.theme["container_light"], self.container_alpha), outline=""
        )

        # 签筒开口
        self.animation_canvas.create_rectangle(
            x - width // 2 + 10, y - height // 2,
            x + width // 2 - 15, y - height // 2 + 5,
            fill=self.theme["bg"], outline=""
        )

    def draw_multiple_sticks(self):
        """在签筒内绘制多根签"""
        x, y = 150 + self.shake_offset, 150
        container_height = 180
        stick_length = 180  # 签在筒内的长度
        stick_width = 10

        start_y = y - container_height // 2 + 20
        end_y = start_y - stick_length * 0.7  # 签在筒内露出的长度

        # 绘制每一根签
        for i, offset in enumerate(self.stick_offsets):
            # 对于选中的签，在消失阶段不绘制（由draw_selected_stick单独绘制）
            if self.phase == 2 and i == self.selected_stick_index:
                continue

            # 确定当前签的透明度
            alpha = self.other_sticks_alpha if self.phase == 2 else 1.0

            # 创建半透明颜色
            def get_alpha_color(base_color, alpha):
                r = int(base_color[1:3], 16)
                g = int(base_color[3:5], 16)
                b = int(base_color[5:7], 16)
                return f"#{int(r * alpha):02x}{int(g * alpha):02x}{int(b * alpha):02x}"

            # 绘制签
            self.animation_canvas.create_line(
                x + offset, start_y, x + offset, end_y,
                width=stick_width, fill=get_alpha_color(self.theme["stick"], alpha),
                capstyle=tk.ROUND, joinstyle=tk.ROUND
            )
            # 签的边框
            self.animation_canvas.create_line(
                x + offset, start_y, x + offset, end_y,
                width=stick_width - 2, fill=get_alpha_color(self.theme["stick_border"], alpha),
                capstyle=tk.ROUND, joinstyle=tk.ROUND
            )

    def draw_selected_stick(self):
        """绘制选中的签，在签筒消失时显示"""
        x, y = 150, 150  # 选中的签最终位置在中心，不摇晃
        container_height = 180
        stick_length = 220
        stick_width = 12

        # 计算签的位置，随动画上升
        start_y = y - container_height // 2 + 20
        lift_amount = (self.animation_step / 30) * 30  # 上升的距离
        end_y = start_y - stick_length - lift_amount + (self.stick_offsets[self.selected_stick_index] * 2)

        # 创建半透明颜色
        def get_alpha_color(base_color, alpha):
            r = int(base_color[1:3], 16)
            g = int(base_color[3:5], 16)
            b = int(base_color[5:7], 16)
            return f"#{int(r * alpha):02x}{int(g * alpha):02x}{int(b * alpha):02x}"

        # 绘制签的阴影（增强立体感）
        shadow_offset = 3
        self.animation_canvas.create_line(
            x + shadow_offset, start_y + shadow_offset,
            x + shadow_offset, end_y + shadow_offset,
            width=stick_width, fill=get_alpha_color("#d0c8b0", self.selected_stick_alpha),
            capstyle=tk.ROUND, joinstyle=tk.ROUND
        )

        # 绘制签主体
        self.animation_canvas.create_line(
            x, start_y, x, end_y,
            width=stick_width, fill=get_alpha_color(self.theme["stick"], self.selected_stick_alpha),
            capstyle=tk.ROUND, joinstyle=tk.ROUND
        )
        # 签的边框
        self.animation_canvas.create_line(
            x, start_y, x, end_y,
            width=stick_width - 2, fill=get_alpha_color(self.theme["stick_border"], self.selected_stick_alpha),
            capstyle=tk.ROUND, joinstyle=tk.ROUND
        )

        # 绘制签上的红点
        dot_y = start_y - (stick_length * 0.3) - lift_amount
        self.animation_canvas.create_oval(
            x - 4 + 1, dot_y - 4 + 1,
            x + 4 + 1, dot_y + 4 + 1,
            fill=get_alpha_color("#a00000", self.selected_stick_alpha), outline=""
        )
        self.animation_canvas.create_oval(
            x - 4, dot_y - 4,
            x + 4, dot_y + 4,
            fill=get_alpha_color(self.theme["red_dot"], self.selected_stick_alpha), outline=""
        )

    def generate_answer(self):
        """生成答案（与动画时长同步）"""
        # 总动画时长：1秒抽签中 + 2秒摇晃 + 1.5秒消失与出现 = 4.5秒
        time.sleep(4.5)

        # 随机选择答案类型（7:3的比例，积极答案更多）
        answer_type = random.choices(["positive", "negative"], weights=[0.7, 0.3])[0]
        self.current_answer = random.choice(self.answers[answer_type])

        # 传递答案类型和内容
        self.root.after(0, self.show_answer, self.current_answer, answer_type)

    def show_answer(self, answer, answer_type):
        """显示答案并清理动画，恢复答案框大小"""
        # 隐藏动画区域和抽签中提示
        self.animation_frame.pack_forget()
        self.drawing_frame.pack_forget()

        # 恢复按钮和答案框状态
        self.ask_button.config(state=tk.NORMAL)
        self.answer_text.config(height=self.original_answer_height)

        # 显示完整答案
        self.answer_text.config(state=tk.NORMAL)
        self.answer_text.delete(1.0, tk.END)
        self.answer_text.insert(tk.END, answer)
        self.answer_text.config(state=tk.DISABLED)

        # 根据答案类型显示相应特效
        if answer_type == "negative":
            # 仅在消极答案时显示木鱼、祈福文字和功德提示
            self.show_wooden_fish()
        else:
            # 积极答案时确保这些元素都隐藏
            self.wooden_fish_group.pack_forget()
            self.pray_label.pack_forget()
            self.wooden_fish_canvas.pack_forget()
            self.merit_label.pack_forget()
            self.launch_fireworks()

    def show_wooden_fish(self):
        """显示电子木鱼区域并开始动画，同时显示祈福文字"""
        # 确保所有元素都被正确显示
        self.wooden_fish_group.pack()
        self.pray_label.pack(pady=(0, 10))  # 显式显示祈福文字
        self.wooden_fish_canvas.pack()  # 显式显示木鱼
        self.merit_label.pack(pady=5, fill=tk.X)

        # 重置动画状态
        self.wooden_fish_animating = True
        self.wooden_fish_start_time = time.time()

        # 开始动画
        self.update_wooden_fish()

    def update_wooden_fish(self):
        """更新电子木鱼动画"""
        if not self.wooden_fish_animating:
            return

        # 计算动画时间
        current_time = time.time()
        elapsed_time = current_time - self.wooden_fish_start_time

        # 如果超过5秒，停止动画并隐藏所有元素
        if elapsed_time >= 5:
            self.wooden_fish_animating = False
            self.wooden_fish_group.pack_forget()
            self.pray_label.pack_forget()
            self.wooden_fish_canvas.pack_forget()
            self.merit_label.pack_forget()
            return

        # 清除画布
        self.wooden_fish_canvas.delete("all")

        # 使用固定尺寸绘制
        canvas_width = 180
        canvas_height = 180
        center_x = canvas_width // 2
        center_y = canvas_height // 2

        # 绘制木鱼主体
        fish_width = 100
        fish_height = 50

        self.wooden_fish_canvas.create_oval(
            center_x - fish_width // 2, center_y - fish_height // 2,
            center_x + fish_width // 2, center_y + fish_height // 2,
            fill="#8B4513", outline="#5D4037", width=3
        )

        # 绘制木鱼纹理
        for i in range(-2, 3):
            y_offset = i * 8
            self.wooden_fish_canvas.create_line(
                center_x - fish_width // 2 + 10, center_y + y_offset,
                center_x + fish_width // 2 - 10, center_y + y_offset,
                fill="#A0522D", width=2
            )

        # 绘制木鱼顶部
        self.wooden_fish_canvas.create_oval(
            center_x - 15, center_y - 25,
            center_x + 15, center_y - 15,
            fill="#A0522D", outline="#5D4037", width=2
        )

        # 绘制锤子
        hammer_angle = math.sin(current_time * 3) * 1.5
        hammer_length = 40
        hammer_x = center_x + 50 * math.cos(hammer_angle)
        hammer_y = center_y - 10 + 20 * math.sin(hammer_angle)

        # 锤子手柄
        self.wooden_fish_canvas.create_line(
            hammer_x, hammer_y,
            hammer_x - hammer_length * math.cos(hammer_angle),
            hammer_y - hammer_length * math.sin(hammer_angle),
            fill="#5D4037", width=4
        )

        # 锤头
        hammer_size = 10
        self.wooden_fish_canvas.create_oval(
            hammer_x - hammer_size, hammer_y - hammer_size,
            hammer_x + hammer_size, hammer_y + hammer_size,
            fill="#8B4513", outline="#5D4037", width=2
        )

        # 更新功德文字（添加闪烁效果）
        text_index = int(current_time * 2) % 2
        text = self.merit_texts[text_index]

        # 文字闪烁效果
        blink_state = int(current_time * 4) % 2
        if blink_state == 0:
            self.merit_label.config(text=text, fg=self.theme["merit_text"])
        else:
            self.merit_label.config(text=text, fg="#009900")

        # 播放音效
        if abs(hammer_angle) < 0.3:  # 当锤子接近木鱼时
            self.sounds["wooden_fish"].play()

        # 继续下一帧动画
        self.root.after(100, self.update_wooden_fish)

    def launch_fireworks(self):
        """发射烟花，创建烟花效果"""
        self.firework_active = True
        self.fireworks = []

        # 随机颜色组合
        firework_colors = [
            (255, 50, 50),  # 红色
            (50, 255, 50),  # 绿色
            (50, 50, 255),  # 蓝色
            (255, 255, 50),  # 黄色
            (255, 50, 255),  # 粉色
            (50, 255, 255),  # 青色
            (255, 165, 0)  # 橙色
        ]

        # 创建3-5个烟花
        for _ in range(random.randint(3, 5)):
            # 随机位置（窗口上方）
            x = random.randint(100, self.root.winfo_width() - 100)
            y = random.randint(50, self.root.winfo_height() // 2)

            # 随机颜色
            color = random.choice(firework_colors)

            # 创建烟花粒子（每个烟花80-120个粒子）
            for _ in range(random.randint(80, 120)):
                self.fireworks.append(FireworkParticle(x, y, color))

        # 播放烟花音效
        self.sounds["firework"].play()

        # 开始更新烟花动画
        self.update_fireworks()

    def update_fireworks(self):
        """更新烟花动画"""
        if not self.firework_active:
            return

        # 清除烟花画布
        self.firework_canvas.delete("all")

        # 更新并绘制所有烟花粒子
        alive_particles = []
        for particle in self.fireworks:
            particle.update(0.05)  # 固定时间步长
            if particle.is_alive():
                alive_particles.append(particle)
                # 绘制粒子
                self.firework_canvas.create_oval(
                    particle.x - particle.size,
                    particle.y - particle.size,
                    particle.x + particle.size,
                    particle.y + particle.size,
                    fill=particle.get_alpha_color(),
                    outline=""
                )

        # 更新存活的粒子
        self.fireworks = alive_particles

        # 如果还有粒子或动画时间未到，继续更新
        if len(self.fireworks) > 0 or time.time() - self.last_animation_time < 6:
            self.root.after(50, self.update_fireworks)
        else:
            # 动画结束，清除画布
            self.firework_canvas.delete("all")
            self.firework_active = False


if __name__ == "__main__":
    root = tk.Tk()
    app = AnswerBookApp(root)
    root.mainloop()
