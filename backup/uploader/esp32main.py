import subprocess as sp
import os, sys
from serial.tools import list_ports

from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QComboBox, QTextEdit
)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QPoint

def resourcePath(relative_path):
    if hasattr(sys, "_MEIPASS"):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)

def scanComPorts():
    return [p.device for p in list_ports.comports()]

class UploadThread(QThread):
    log = pyqtSignal(str)
    finished = pyqtSignal(int)

    def __init__(self, comPort):
        super().__init__()
        self.comPort = comPort

    def run(self):
        fw_dir = resourcePath("firmware")

        esptool = os.path.join(fw_dir, "esptool.exe")
        merged_bin = os.path.join(
            fw_dir,
            "nova-x-esp32c5.ino.merged.bin"
        )

        if not os.path.exists(esptool):
            self.log.emit("[!] esptool.exe not found\n")
            self.finished.emit(-1)
            return

        if not os.path.exists(merged_bin):
            self.log.emit("[!] merged firmware not found\n")
            self.finished.emit(-1)
            return

        cmd = [
            esptool,
            "--chip", "esp32c5",
            "--port", self.comPort,
            "--baud", "1500000",
            "--before", "default-reset",
            "--after", "hard-reset",
            "write-flash",
            "--flash-mode", "dio",
            "--flash-freq", "80m",
            "--flash-size", "4MB",
            "0x0",
            merged_bin
        ]

        try:
            proc = sp.Popen(
                cmd,
                stdout=sp.PIPE,
                stderr=sp.STDOUT,
                universal_newlines=True,
                creationflags=sp.CREATE_NO_WINDOW
            )

            for line in proc.stdout:
                self.log.emit(line)

            self.finished.emit(proc.wait())

        except Exception as e:
            self.log.emit(f"[!] Exception: {e}\n")
            self.finished.emit(-1)

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Window)
        self.setFixedSize(520, 360)

        self.oldPos = QPoint()
        self.initUI()
        self.refreshPorts()

    def initUI(self):
        mainLayout = QVBoxLayout()
        mainLayout.setContentsMargins(0, 0, 0, 0)
        self.titleBar = QWidget()
        self.titleBar.setFixedHeight(36)

        titleLayout = QHBoxLayout()
        titleLayout.setContentsMargins(10, 0, 10, 0)

        self.titleLabel = QLabel("ESP32-C5 Image Uploader By warwick320")

        closeBtn = QPushButton("✕")
        closeBtn.setFixedSize(30, 24)
        closeBtn.clicked.connect(self.close)

        titleLayout.addWidget(self.titleLabel)
        titleLayout.addStretch()
        titleLayout.addWidget(closeBtn)

        self.titleBar.setLayout(titleLayout)

        content = QWidget()
        contentLayout = QVBoxLayout()

        portLayout = QHBoxLayout()
        portLayout.addWidget(QLabel("COM Port"))

        self.portCombo = QComboBox()
        portLayout.addWidget(self.portCombo)

        refreshBtn = QPushButton("Refresh")
        refreshBtn.clicked.connect(self.refreshPorts)
        portLayout.addWidget(refreshBtn)

        contentLayout.addLayout(portLayout)

        self.uploadBtn = QPushButton("Upload")
        self.uploadBtn.setFixedHeight(40)
        self.uploadBtn.clicked.connect(self.startUpload)
        contentLayout.addWidget(self.uploadBtn)

        self.logBox = QTextEdit()
        self.logBox.setReadOnly(True)
        contentLayout.addWidget(self.logBox)

        content.setLayout(contentLayout)

        mainLayout.addWidget(self.titleBar)
        mainLayout.addWidget(content)
        self.setLayout(mainLayout)

        self.setStyleSheet("""
            QWidget {
                background-color: #121212;
                color: #E0E0E0;
                font-family: Consolas;
            }
            QPushButton {
                background-color: #1E1E1E;
                border: 1px solid #333;
                border-radius: 6px;
            }
            QComboBox, QTextEdit {
                background-color: #1E1E1E;
                border: 1px solid #333;
                border-radius: 4px;
            }
        """)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton and event.pos().y() < 36:
            self.oldPos = event.globalPos()

    def mouseMoveEvent(self, event):
        if event.buttons() == Qt.LeftButton:
            delta = event.globalPos() - self.oldPos
            self.move(self.x() + delta.x(), self.y() + delta.y())
            self.oldPos = event.globalPos()
    def log(self, msg):
        self.logBox.moveCursor(self.logBox.textCursor().End)
        self.logBox.insertPlainText(msg)
        self.logBox.ensureCursorVisible()

    def refreshPorts(self):
        self.portCombo.clear()
        for p in scanComPorts():
            self.portCombo.addItem(p)

    def startUpload(self):
        comPort = self.portCombo.currentText()
        if not comPort:
            self.log("[!] COM port not selected\n")
            return

        self.uploadBtn.setEnabled(False)

        self.thread = UploadThread(comPort)
        self.thread.log.connect(self.log)
        self.thread.finished.connect(self.uploadFinished)
        self.thread.start()

    def uploadFinished(self, code):
        self.uploadBtn.setEnabled(True)
        if code == 0:
            self.log("\n[+] Upload completed successfully\n")
        else:
            self.log(f"\n[!] Upload failed (code {code})\n")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())
