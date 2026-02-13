from PyQt5.QtWidgets import QWidget, QTextEdit, QPushButton, QHBoxLayout, QVBoxLayout
from PyQt5.QtCore import Qt
from InputTextEdit import InputTextEdit
from PyQt5.QtGui import QTextCursor, QTextBlockFormat
from PyQt5.QtWidgets import QListWidget
from PyQt5.QtWidgets import QListWidgetItem

class MainWindow(QWidget):
    def __init__(self, chat_client):
        super().__init__()
        self.chat_client = chat_client
        self.target_uid = None
        self.init_ui()
        self.chat_client.message_received.connect(self.on_message_received)
        self.chat_client.message_sent.connect(self.on_message_sent)
        self.chat_client.error.connect(self.on_error)
        self.chat_client.disconnected.connect(self.on_disconnected)
        self.chat_client.friendlist_updated.connect(self.refresh_friend_list)

    def init_ui(self):

        # 左侧：好友列表
        self.friend_list = QListWidget()
        for username, uid in self.chat_client.friendlist:
            item = QListWidgetItem(username)
            # 把真正的 UID 存到 UserRole 里
            item.setData(Qt.UserRole, uid)
            self.friend_list.addItem(item)
        self.friend_list.currentItemChanged.connect(self.on_friend_selected)

        # 右侧：聊天显示区
        self.text_display = QTextEdit()
        self.text_display.setReadOnly(True)
        self.text_input = InputTextEdit()
        self.text_input.setPlaceholderText('在此输入…(回车发送，Shift+回车换行)')
        send_btn = QPushButton('发送')
        send_btn.clicked.connect(self.send_message)
        self.text_input.send.connect(self.send_message)

        input_layout = QHBoxLayout()
        input_layout.addWidget(self.text_input)
        input_layout.addWidget(send_btn)

        right_layout = QVBoxLayout()
        right_layout.addWidget(self.text_display, stretch=3)
        right_layout.addLayout(input_layout, stretch=1)

        # 整体左右布局
        main_layout = QHBoxLayout()
        main_layout.addWidget(self.friend_list, stretch=1)
        main_layout.addLayout(right_layout, stretch=3)
        self.setLayout(main_layout)
        
    def on_friend_selected(self, current, previous):
        if current:
            self.target_uid = current.data(Qt.UserRole)
            # 这里可以清空或加载对应对话历史
            self.text_display.clear()

    def send_message(self):
        content = self.text_input.toPlainText().strip()
        if content and self.target_uid is not None:
            self.chat_client.send_text(self.target_uid, content)
            self.text_input.clear()

    def on_message_received(self, text):
        self.append_message(text, Qt.AlignLeft)

    def on_message_sent(self, text):
        self.append_message(text, Qt.AlignRight)

    def on_error(self, msg):
        self.text_display.append(f'Error: {msg}')

    def on_disconnected(self):
        self.text_display.append('Disconnected from server')

    def append_message(self, text: str, alignment):
        cursor = self.text_display.textCursor()
        cursor.movePosition(QTextCursor.End)
        block_fmt = QTextBlockFormat()
        block_fmt.setAlignment(alignment)
        cursor.insertBlock(block_fmt)
        cursor.insertText(text)
        # 滚动到最后
        self.text_display.setTextCursor(cursor)

    def refresh_friend_list(self, friends: list):
        # 清空列表
        self.friend_list.clear()
        # 重新填充
        for username, uid in friends:
            item = QListWidgetItem(username)
            # 把真正的 UID 存到 UserRole 里
            item.setData(Qt.UserRole, uid)
            self.friend_list.addItem(item)
