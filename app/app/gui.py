import tkinter as tk
from tkinter import ttk
from app.logic import add_numbers

class MainWindow(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)

        parent.title("My Tkinter App")
        parent.geometry("400x200")

        self.entry1 = ttk.Entry(self)
        self.entry2 = ttk.Entry(self)
        self.result_label = ttk.Label(self, text="Result:")

        self.calc_button = ttk.Button(self, text="Calculate", command=self.calculate)

        self.entry1.pack(pady=5)
        self.entry2.pack(pady=5)
        self.calc_button.pack(pady=5)
        self.result_label.pack(pady=5)

    def calculate(self):
        try:
            a = float(self.entry1.get())
            b = float(self.entry2.get())
            result = add_numbers(a, b)
            self.result_label.config(text=f"Result: {result}")
        except ValueError:
            self.result_label.config(text="Invalid input")
