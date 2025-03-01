from PIL import Image
import struct

# Abrir imagen
img = Image.open('tools/14.jpg')
img = img.convert('RGB')
img = img.resize((240, 135))  

with open('spiffs_image/14.bin', 'wb') as f:
    for y in range(img.height):
        for x in range(img.width):
            r, g, b = img.getpixel((x, y))
            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            f.write(struct.pack('H', rgb565))  
