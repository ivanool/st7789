import freetype
import struct

FONT_SIZE = 12  
FONT_WIDTH = 8  
FONT_HEIGHT = 12  
START_CHAR = 32  
END_CHAR = 127  

def render_character(face, char):
    face.set_char_size(FONT_SIZE * 64)  
    face.load_char(char, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
    bitmap = face.glyph.bitmap

    char_data = [0] * FONT_HEIGHT  

    for y in range(min(bitmap.rows, FONT_HEIGHT)):
        row_data = 0
        for x in range(min(bitmap.width, FONT_WIDTH)):
            byte_index = (x // 8) + y * bitmap.pitch
            bit_mask = 1 << (7 - (x % 8))
            if bitmap.buffer[byte_index] & bit_mask:
                row_data |= (1 << (7 - x))
        char_data[y] = row_data

    return char_data

def convert_font_to_bin(input_ttf, output_bin):
    face = freetype.Face(input_ttf)
    characters = list(range(START_CHAR, END_CHAR))

    with open(output_bin, "wb") as f:
        for char in characters:
            char_bitmap = render_character(face, chr(char))
            f.write(struct.pack(f'{FONT_HEIGHT}B', *char_bitmap))

    print(f"Fuente convertida y guardada en {output_bin}")

convert_font_to_bin("font.ttf", "font.bin")
