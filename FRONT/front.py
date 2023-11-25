import socket
import struct
import pygame as pg
import math

pg.init()

screen = pg.display.set_mode((1280, 720))
font = pg.font.Font(None, 34)
clock = pg.time.Clock()
running = True
dt = 0

raio = 200

LD = [0,0,0,0,0,0,0]
D  = [0,0,0,0,0,0,0]

gyro_pos = pg.Vector2(200, 200)

class HoleSprite( pg.sprite.Sprite ):
    def __init__( self ):
        pg.sprite.Sprite.__init__( self )
        self.hole_image = pg.Surface( ( raio*2, raio*2 ), pg.SRCALPHA )
        self.hole_image.fill("purple")
        pg.draw.circle(self.hole_image, (0, 0, 0, 0), (raio,raio), raio/2)
        self.image  = self.hole_image
        self.rect   = self.image.get_rect()
        self.rect.x = gyro_pos.x  - raio # centred
        self.rect.y = gyro_pos.y  - raio
        self.last   = 0

    def update( self ):
        self.image = self.hole_image

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
        D[6] += 1 * dt
    if keys[pg.K_q]:
        D[6] += 1 * dt

    D = receive_data()
    LD = D

    points = [(gyro_pos.x-(raio*math.cos(D[4]*1.57)),(gyro_pos.y-(raio*math.sin(D[4]*1.57)))+D[5]*raio/2),
              (gyro_pos.x+(raio*math.cos(D[4]*1.57)),(gyro_pos.y+(raio*math.sin(D[4]*1.57)))+D[5]*raio/2),
              (gyro_pos.x-(raio*math.sin(D[4]*1.57)),gyro_pos.y+(raio*math.cos(D[4]*1.57)))]

    screen.fill("purple")
    pg.draw.circle(screen, "red", gyro_pos, raio)
    pg.draw.polygon(screen, "blue", points)
    anims.draw(screen)
    anims.update()

    for i,v in enumerate(D[:4]):
        draw_text('Sensor {}: {:0>5.1f}'.format(i + 1, v),10,500 + (i*30))
    
    for i,v in enumerate(D[4:]):
        draw_text('Giroscopio {}: {:.2f}'.format(i + 1, v),200,500 + (i*30))

    pg.display.flip()
    dt = clock.tick(60) / 1000

pg.quit()