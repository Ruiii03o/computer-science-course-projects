# specfile.spec 打包配置
block_cipher = None

a = Analysis(
    ['app.py'],  # 入口文件（你的核心启动文件）
    pathex=[os.path.abspath('.')],  # 项目根目录
    excludes=['pygame'],  # 新增：排除pygame依赖
    cipher=block_cipher,
    binaries=[],
    datas=[
        # 关键：将静态资源、模板、数据文件夹打包进exe（格式：(源路径, 目标路径)）
        ('templates', 'templates'),  # 模板文件夹（HTML文件）
        ('static', 'static'),        # 静态资源（图片/CSS/JS）
        ('data', 'data'),            # 数据文件夹（食谱数据）
        ('utils', 'utils'),          # 工具文件夹（工具函数）
        ('config.py', '.'),          # 配置文件
        ('messages.txt', '.'),        # 留言文件
        ('Tool_urllib','Tool_urllib')
    ],
    hiddenimports=['flask'],  # 显式指定依赖（避免打包时遗漏Flask）
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    noarchive=False
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='RecipeWebsite',  # 生成的exe文件名（可自定义，如“盲盒食谱.exe”）
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,  # 压缩exe体积（可选，需要安装upx，不安装可设为False）
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,  # 打包后运行时显示终端（方便看启动日志，后续可改为False）
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=None  # 可选：添加exe图标，格式为 ('icon.ico', 0)，需提前准备ico文件
)