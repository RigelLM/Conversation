import struct
import socket

from enum import IntEnum

class MessageType(IntEnum):
    HEARTBEAT = 0
    LOGIN     = 1
    PULL      = 2
    TEXT      = 3


class Message:
    # 定义网络字节序的消息头格式：2个 uint32, 1个 uint8, 2个 uint32
    HEADER_FORMAT = '!IIBII'
    HEADER_SIZE   = struct.calcsize(HEADER_FORMAT)

    def __init__(self,
                 target_uid: int = 0,
                 sender_uid: int = 0,
                 msg_type: MessageType = MessageType.HEARTBEAT,
                 timestamp: int = 0,
                 data: bytes = b''):
        self.m_TargetUID  = target_uid
        self.m_SenderUID  = sender_uid
        self.m_Type       = msg_type
        self.m_Timestamp  = timestamp
        self.m_Data       = data  # bytes
        self.m_Size       = len(data)

    def _recv_all(self, sock: socket.socket, n: int) -> bytes:
        """接收指定长度的数据，直到读取完成或连接中断。"""
        buf = bytearray()
        while len(buf) < n:
            chunk = sock.recv(n - len(buf))
            if not chunk:
                return None
            buf.extend(chunk)
        return bytes(buf)

    def receive(self, sock: socket.socket) -> bool:
        # 接收并解析头部
        header = self._recv_all(sock, self.HEADER_SIZE)
        if not header:
            return False
        t_uid, s_uid, type_byte, ts, size = struct.unpack(self.HEADER_FORMAT, header)
        self.m_TargetUID = t_uid
        self.m_SenderUID = s_uid
        self.m_Type      = MessageType(type_byte)
        self.m_Timestamp = ts
        self.m_Size      = size

        # 接收数据部分
        data = self._recv_all(sock, self.m_Size)
        if data is None:
            return False
        self.m_Data = data
        return True

    def send(self, sock: socket.socket) -> bool:
        # 更新消息大小
        self.m_Size = len(self.m_Data)
        # 打包头部
        header = struct.pack(
            self.HEADER_FORMAT,
            self.m_TargetUID,
            self.m_SenderUID,
            self.m_Type.value,
            self.m_Timestamp,
            self.m_Size
        )
        # 发送头部和数据
        try:
            sock.sendall(header + self.m_Data)
            return True
        except socket.error:
            return False