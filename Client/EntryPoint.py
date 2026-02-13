import sys
from PyQt5.QtWidgets import QApplication
from ChatClient import ChatClient
from LoginWindow import LoginWindow

def main():
    app = QApplication(sys.argv)
    client = ChatClient()
    login = LoginWindow(client)
    login.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
