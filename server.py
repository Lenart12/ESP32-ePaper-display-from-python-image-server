#!/usr/bin/env python3.7
import socket
import random

# pip install pillow
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

HOST = '0.0.0.0'
PORT = 56789

W = 640
H = 384

WHITE = 0xFF
BLACK = 0x00
RED   = 0X0F

def generateImage():
    img = Image.new('L', (W, H), WHITE)
    draw  = ImageDraw.Draw(img)

    draw.line([(0,0) , (W - 1, H - 1)], BLACK, 10)
    draw.line([(W - 1, 0) , (0, H - 1)], RED, 10)
    draw.arc( [(W/2 - 50, H/2 - 50), (W/2 + 50, H/2 + 50)], 0, 360, RED, 20)
    draw.text( (W/4, H/4), '1234567890ABC', BLACK, ImageFont.truetype('arial.ttf'))
    return img

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.bind((HOST, PORT))
    print('Started image server')
    while True:
        sock.listen()
        conn, addr = sock.accept()

        with conn:
            print(f'Request from {addr}')
            img = generateImage()
            imgBytes = img.tobytes()
            hashBytes = (hash(imgBytes) % 4294967296).to_bytes(4, 'little')
            sendBytes = hashBytes + imgBytes

            conn.sendall(sendBytes)
            print( f'Sent {len(sendBytes)} bytes - hash { "".join( [hex(h)[-2:] for h in hashBytes] ).upper() }' )

        
        