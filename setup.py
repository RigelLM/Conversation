# setup.py
from setuptools import setup

APP = ['Client/EntryPoint.py']      # 指向你的入口脚本
DATA_FILES = []               # 如果有其他资源，按需要添加
OPTIONS = {
    # 双击 .app 时能接收命令行参数（如拖拽文件到图标）
    'argv_emulation': False,

    # 应用图标：相对路径到项目根
    'iconfile': 'Client/icon.icns',

    # 强制包含 PyQt5 相关模块
    'includes': [
        'PyQt5',
        'PyQt5.QtCore',
        'PyQt5.QtGui',
        'PyQt5.QtWidgets',
    ],

    # 排除不必要的模块，加快打包速度、减小体积
    'excludes': [
        'tkinter',
        'unittest',
    ],

    # 如果你的项目还有其他包需要整个拷贝，可以在这里列出
    'packages': [],
}

setup(
    name='LiteChat',
    app=APP,
    data_files=DATA_FILES,
    options={'py2app': OPTIONS},
    setup_requires=['py2app'],
)