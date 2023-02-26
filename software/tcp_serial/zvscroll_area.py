from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

class ZVScrollArea(QScrollArea):
    def __init__(self, parent):
        super().__init__(parent)

        self.vbox = QVBoxLayout()
        self.vbox.setSpacing(0)

        vbox = QVBoxLayout()
        vbox.addLayout(self.vbox)
        vbox.addStretch()
        
        self.widget = QWidget()
        # self.widget.setFixedHeight(160)
        self.widget.setLayout(vbox)
        
        self.setWidget(self.widget)
        self.setWidgetResizable(True)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

    def add_widget(self, widget):
        # self.widget.setFixedHeight((self.vbox.count() + 1) * widget.height())
        self.vbox.addWidget(widget)

    def delete(self, id):
        item = self.vbox.itemAt(id)
        self.vbox.removeItem(item)
        item.widget().deleteLater()

        for i in range(self.vbox.count()):
            item = self.vbox.itemAt(i)
            item.widget().set_id(i)

    def count(self):
        return self.vbox.count()

    def at(self, id):
        return self.vbox.itemAt(id)

