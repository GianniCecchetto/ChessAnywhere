# test_board_com.py
from board_com_ctypes import fmt_ping, fmt_led_set, fmt_stream, LineBuffer, parse_line

def test_formatters():
    print("=== Formatters ===")
    print(repr(fmt_ping()))
    print(repr(fmt_led_set(12, 255, 0, 0)))  # env. E2
    print(repr(fmt_stream(True)))

def test_linebuffer_and_parser():
    print("\n=== LineBuffer + Parser ===")
    lb = LineBuffer()
    sim = b"EVT LIFT E2 t=102345\r\nOK VER FW=1.2.0 HW=CHESS-01\r\n"
    lb.feed(sim)
    for line in lb.pop_all():
        print("[RX]", line)
        print(" ->", parse_line(line))

if __name__ == "__main__":
    test_formatters()
    test_linebuffer_and_parser()
