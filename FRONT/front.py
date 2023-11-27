import socket
import struct
import pygame as pg
import numpy as np
import math


pg.init()

screen = pg.display.set_mode((1280, 720))
font = pg.font.Font(None, 50)

clock = pg.time.Clock()
running = True

dt = 0
raio = 250
zoom = 1

SENSOR_DISPLAY_SIZE = (450, 450)
SENSOR_DISPLAY_POS = (100, 380 - (SENSOR_DISPLAY_SIZE[0]/2))
SENSOR_DISPLAY = pg.Surface(SENSOR_DISPLAY_SIZE)

LD = [50,0,0,0,0,0,0]
D  = [50,0,0,0,0,0,0]

distancias = np.array([])
graus = np.array([])

gyro_pos = pg.Vector2(920, 300)

class HoleSprite( pg.sprite.Sprite ):
    def __init__( self ):
        pg.sprite.Sprite.__init__( self )
        self.hole_image = pg.Surface( ( raio*2, raio*2 ), pg.SRCALPHA )
        self.hole_image.fill((79,79,79))
        pg.draw.circle(self.hole_image, (0, 0, 0, 0), (raio,raio), raio/2)
        self.image  = self.hole_image
        self.rect   = self.image.get_rect()
        self.rect.x = gyro_pos.x  - raio # centred
        self.rect.y = gyro_pos.y  - raio
        self.last   = 0

    def update( self ):
        self.image = self.hole_image

def draw_sensor_data():
    SENSOR_DISPLAY.fill("white")
    for i in range(0, graus.size):
        pg.draw.circle(SENSOR_DISPLAY, "red", (SENSOR_DISPLAY_SIZE[0]/2 + (distancias[i]*math.sin(graus[i]*.017)*zoom),
                                                 SENSOR_DISPLAY_SIZE[1]/2 + (distancias[i]*math.cos(graus[i]*.017)*zoom)), 4)

def draw_text(text, x, y):
    surface = font.render(text, True, (255, 255, 255))
    screen.blit(surface, (x, y))

def receive_data():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.01)
    server_address = ('0.0.0.0', 2000)
    sock.bind(server_address)

    try:
        data, address = sock.recvfrom(2000)
        d1, d2, d3, d4, gx, gy, gz = struct.unpack('<iiiiddd', data)
        return [d1, d2, d3, d4,gx,gy,gz]
    except:
        return LD

anims = pg.sprite.GroupSingle()
holey = HoleSprite()
anims.add(holey)

while running:
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False

    keys = pg.key.get_pressed()
    if keys[pg.K_w]:
        D[5] -= .5 * dt
    if keys[pg.K_s]:
        D[5] += .5 * dt
    if keys[pg.K_a]:
        D[4] -= .5 * dt
    if keys[pg.K_d]:
        D[4] += .5 * dt
    if keys[pg.K_e]:
        D[6] -= 10 * dt
    if keys[pg.K_q]:
        D[6] += 10 * dt
    if keys[pg.K_i]:
        zoom += 10 * dt
    if keys[pg.K_o]:
        zoom -= 10 * dt

    D = receive_data()
    LD = D

    if int(D[6]) not in graus:
        graus = np.append(graus,int(D[6]))
        distancias = np.append(distancias,int(D[0]))
        print(graus)
        print(distancias)

    points = [(gyro_pos.x-(raio*math.cos(D[4]*1.57)),(gyro_pos.y-(raio*math.sin(D[4]*1.57)))+D[5]*raio/2),
              (gyro_pos.x+(raio*math.cos(D[4]*1.57)),(gyro_pos.y+(raio*math.sin(D[4]*1.57)))+D[5]*raio/2),
              (gyro_pos.x-(raio*math.sin(D[4]*1.57)),gyro_pos.y+(raio*math.cos(D[4]*1.57)))]

    screen.fill((79,79,79))  
    pg.draw.circle(screen, (114, 188, 212), gyro_pos, raio)  # Cor do círculo vermelho
    pg.draw.polygon(screen, (165, 104, 42), points)  # Cor do polígono azul
    anims.draw(screen)

    for i,v in enumerate(D[:4]):
        draw_text('Sensor {}: {:0>5.1f}'.format(i + 1, v),620,500 + (i*40))
    
    for i,v in enumerate(D[4:]):
        draw_text('Giroscopio {}: {:.2f}'.format(i + 1, v),900,510 + (i*40))

    draw_text('Horizonte de Eventos'.format(i + 1, v), 740, 100)

    draw_text('Mapa'.format(i + 1, v), 270, 80)

    draw_sensor_data()
    screen.blit(SENSOR_DISPLAY, SENSOR_DISPLAY_POS)

    pg.display.flip()
    dt = clock.tick(60) / 1000

pg.quit()