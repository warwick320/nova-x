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

def debugPrint(level, ctx):
    print("[*] " + ctx) if level == 0 else print("[!] " + ctx)


def scanComPorts():
    ports = list(list_ports.comports())
    portList = []

    if not ports:
        debugPrint(1, "No COM ports found")
    else:
        debugPrint(0, f"Found {len(ports)} COM ports:")

    for i, p in enumerate(ports):
        debugPrint(0, f"{i+1}. {p.device} - {p.description}")
        portList.append(p.device)

    return portList


class UploadThread(QThread):
    log = pyqtSignal(str)
    finished = pyqtSignal(int)

    def __init__(self, comPort):
        super().__init__()
        self.comPort = comPort
    def run(self):
        toolsDir = resourcePath(os.path.join("realtek_tools", "1.1.1"))
        uploaderPath = os.path.join(toolsDir, "upload_image_tool_windows.exe")

        if not os.path.exists(uploaderPath):
            self.log.emit("[!] Uploader tool not found\n")
            self.finished.emit(-1)
            return

        cmd = [
            uploaderPath,
            toolsDir,
            self.comPort,
            "{board}",
            "Enable",
            "Disable",
            "1500000"
        ]

        proc = sp.Popen(
            cmd,
            cwd=toolsDir,
            stdout=sp.PIPE,
            stderr=sp.PIPE,
            bufsize=0,
            universal_newlines=True,
            creationflags=sp.CREATE_NO_WINDOW
        )

        buffer = ""

        while True:
            ch = proc.stdout.read(1)
            if not ch:
                break

            buffer += ch

            if ch == "\n":
                self.log.emit(buffer)
                buffer = ""
            elif ch == ".":
                self.log.emit(buffer)
                buffer = ""

        if buffer:
            self.log.emit(buffer)

        err = proc.stderr.read()
        if err:
            self.log.emit(err)

        self.finished.emit(proc.wait())



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
        self.titleBar.setObjectName("titleBar")

        titleLayout = QHBoxLayout()
        titleLayout.setContentsMargins(10, 0, 10, 0)

        self.titleLabel = QLabel("Image Uploader By warwick.320")
        self.titleLabel.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)

        closeBtn = QPushButton("✕")
        closeBtn.setFixedSize(30, 24)
        closeBtn.clicked.connect(self.close)
        closeBtn.setObjectName("closeBtn")

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

            /* ===== Title Bar ===== */
            #titleBar {
                background-color: #0B0B0B;
            }

            #titleBar QLabel {
                color: #E0E0E0;
                background-color: #0B0B0B;
            }

            /* ===== Buttons ===== */
            QPushButton {
                background-color: #1E1E1E;
                border: 1px solid #333;
                border-radius: 6px;
            }

            QPushButton:hover {
                background-color: #1E1E1E;
            }

            #closeBtn {
                background-color: #0B0B0B;
                border: none;
                color: #E0E0E0;
            }

            #closeBtn:hover {
                background-color: #0B0B0B;
            }

            /* ===== Inputs ===== */
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
            delta = QPoint(event.globalPos() - self.oldPos)
            self.move(self.x() + delta.x(), self.y() + delta.y())
            self.oldPos = event.globalPos()

    def log(self, msg):
        self.logBox.moveCursor(self.logBox.textCursor().End)
        self.logBox.insertPlainText(msg)
        self.logBox.ensureCursorVisible()

    def refreshPorts(self):
        self.portCombo.clear()
        ports = scanComPorts()
        for p in ports:
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
            self.log(f"\n[!] Upload failed (exit code {code})\n")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec_())
