from PyQt5.QtWidgets import QTextEdit
from PyQt5.QtCore import pyqtSignal, Qt

class InputTextEdit(QTextEdit):
    send = pyqtSignal()

    def keyPressEvent(self, event):
        if event.key() in (Qt.Key_Return, Qt.Key_Enter) and not (event.modifiers() & (Qt.ShiftModifier | Qt.ControlModifier)):
            self.send.emit()
        else:
            super().keyPressEvent(event)