from PyQt5.QtWidgets import QWidget, QFormLayout, QLineEdit, QPushButton, QVBoxLayout, QMessageBox
from PyQt5.QtCore import Qt
from ChatClient import ChatClient
from MainWindow import MainWindow

class LoginWindow(QWidget):
    def __init__(self, chat_client: ChatClient):
        super().__init__()
        self.chat_client = chat_client
        self.init_ui()
        self.chat_client.login_result.connect(self.on_login_result)
        self.chat_client.error.connect(self.on_error)

    def init_ui(self):
        self.setWindowTitle('登录')
        self.resize(300, 150)

        form_layout = QFormLayout()
        self.username_edit = QLineEdit()
        self.pwd_edit = QLineEdit()
        self.pwd_edit.setEchoMode(QLineEdit.Password)
        form_layout.addRow('Username:', self.username_edit)
        form_layout.addRow('Password:', self.pwd_edit)

        login_btn = QPushButton('登录')
        login_btn.clicked.connect(self.attempt_login)

        main_layout = QVBoxLayout()
        main_layout.addLayout(form_layout)
        main_layout.addWidget(login_btn, alignment=Qt.AlignCenter)
        self.setLayout(main_layout)

    def attempt_login(self):
        username = self.username_edit.text().strip()
        pwd = self.pwd_edit.text()
        self.chat_client.login(username, pwd)

    def on_login_result(self, result):
        if result == 'Success':
            self.main_win = MainWindow(self.chat_client)
            self.main_win.show()
            self.close()
        elif result == 'Failed':
            QMessageBox.warning(self, '登录失败', '无效的用户名或密码')
        elif result == 'Conflict':
            QMessageBox.warning(self, '登录失败', '已在别的设备登入')
        elif result == 'Connection':
            QMessageBox.critical(self, '登录失败', '无法连接到服务器')

    def on_error(self, msg):
        QMessageBox.critical(self, '错误', msg)