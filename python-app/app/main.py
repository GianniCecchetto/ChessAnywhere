import tkinter as tk
from app.gui import MainWindow

def main():
    root = tk.Tk()
    app = MainWindow(root)
    app.pack(fill="both", expand=True)
    root.mainloop()

if __name__ == "__main__":
    main()

