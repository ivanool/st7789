import struct

FONT_WIDTH = 8
FONT_HEIGHT = 12
START_CHAR = 32  
END_CHAR = 127  

def read_font_bin(file_path):
    font_data = {}

    with open(file_path, "rb") as f:
        for char_code in range(START_CHAR, END_CHAR):
            char_bitmap = struct.unpack(f"{FONT_HEIGHT}B", f.read(FONT_HEIGHT))
            font_data[chr(char_code)] = char_bitmap

    return font_data

def print_character(char_bitmap):
    for row in char_bitmap:
        line = "".join("█" if (row & (1 << (7 - x))) else " " for x in range(FONT_WIDTH))
        print(line)
    print()  


font = read_font_bin("font.bin")


for char in "ASDFGHJKLÑZXCVBNMQWERTYUIOP1234567890":
    if char in font:
        print(f"Carácter: {char}")
        print_character(font[char])
