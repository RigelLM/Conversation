import socket
import threading
import time
from datetime import datetime
from PyQt5.QtCore import QObject, pyqtSignal
from Message import Message, MessageType

SERVER_ID = 0

class ChatClient(QObject):
    message_received = pyqtSignal(str)
    message_sent = pyqtSignal(str)
    login_result = pyqtSignal(str)
    disconnected = pyqtSignal()
    error = pyqtSignal(str)
    friendlist_updated = pyqtSignal(list)

    def __init__(self, host='127.0.0.1', port=25565, parent=None):
        super().__init__(parent)
        self.host = host
        self.port = port
        self.socket = None
        self.uid = None
        self.running = False
        self.friendlist = []

    def connect_to_server(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
        except Exception as e:
            self.error.emit(f'Failed to connect to server: {e}')
            return False
        return True

    def login(self, username, password):
        if not self.connect_to_server():
            self.login_result.emit('Connection')
            return
        try:
            msg = Message(SERVER_ID, 0, MessageType.LOGIN, int(time.time()), f"{username}:{password}".encode('utf-8'))
            msg.send(self.socket)

            replymsg = Message()
            ok = replymsg.receive(self.socket)
            if not ok:
                self.login_result.emit('Connection')
                return
            
            self.login_result.emit(replymsg.m_Data.decode('utf-8'))
            if replymsg.m_Type == MessageType.LOGIN:
                if replymsg.m_Data.decode('utf-8') == 'Success':
                    self.uid = replymsg.m_TargetUID
                    self.pull_friendlist()
                    self.pull_offlinemsg()

                    self.running = True
                    self.start_threads()
                elif (replymsg.m_Data.decode('utf-8') == 'Failed' or replymsg.m_Data.decode('utf-8') == 'Conflict'):
                    self.socket.close()
                    
        except Exception as e:
            self.error.emit(f'Login error: {e}')
            self.login_result.emit('Connection')

    def start_threads(self):
        threading.Thread(target=self.receive_loop, daemon=True).start()
        threading.Thread(target=self.heartbeat_loop, daemon=True).start()

    def receive_loop(self):
        while self.running and self.socket.fileno() != -1:
            try:
                msg = Message()
                msg.receive(self.socket)

                date_str = datetime.fromtimestamp(msg.m_Timestamp).strftime('%Y/%m/%d')

                if msg.m_Type == MessageType.TEXT:
                    if msg.m_SenderUID == SERVER_ID:
                        text = f"Server@{date_str}: {msg.m_Data.decode('utf-8')}"
                    else:
                        for username, uid in self.friendlist:
                            if uid == msg.m_SenderUID:
                                text = f"{username}@{date_str}: {msg.m_Data.decode('utf-8')}"
                    self.message_received.emit(text)
            except Exception as e:
                self.error.emit(f'Receive error: {e}')
                break
        self.running = False
        self.disconnected.emit()

    def heartbeat_loop(self):
        while self.running and self.socket.fileno() != -1:
            try:
                time.sleep(10)
                msg = Message(SERVER_ID, self.uid, MessageType.HEARTBEAT, int(time.time()), "HEARTBEAT".encode('utf-8'))
                msg.send(self.socket)
            except Exception as e:
                self.error.emit(f'Heartbeat error: {e}')
                break

    def send_text(self, target_uid, text):
        if not self.running:
            return
        try:
            data = text.encode('utf-8')
            timestamp = int(time.time())
            msg = Message(target_uid, self.uid, MessageType.TEXT, timestamp, data)
            msg.send(self.socket)

            date_str = datetime.fromtimestamp(timestamp).strftime('%Y/%m/%d')
            content = f"You@{date_str}: {text}"
            self.message_sent.emit(content)

        except Exception as e:
            self.error.emit(f'Send error: {e}')

    def close(self):
        self.running = False
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
                self.socket.close()
            except:
                pass

    def pull_friendlist(self):
        try:
            msg = Message(SERVER_ID, self.uid, MessageType.PULL, int(time.time()), 'FriendList'.encode('utf-8'))
            msg.send(self.socket)

            replymsg = Message()
            replymsg.receive(self.socket)

            content = replymsg.m_Data.decode('utf-8')

            friends = [
                (
                    entry.split(",", 1)[0].strip(),          # 字符串部分
                    int(entry.split(",", 1)[1].strip())      # 转成整数的 UID
                )
                for entry in content.split(";")              # 先按分号切分
                if entry.strip()                             # 过滤掉空条目（若有多余分号）
            ]

            self.set_friendlist(friends)

        except Exception as e:
            self.error.emit(f'Pull error: {e}')

    def pull_offlinemsg(self):
        try:
            msg = Message(SERVER_ID, self.uid, MessageType.PULL, int(time.time()), 'OfflineMessage'.encode('utf-8'))
            msg.send(self.socket)

        except Exception as e:
            self.error.emit(f'Pull error: {e}')

    def set_friendlist(self, friends: list):
        self.friendlist = friends
        self.friendlist_updated.emit(self.friendlist)