# Example file showing a circle moving on screen
import socket
import struct
import threading
import pygame as pg
import math

# pg setup
pg.init()
screen = pg.display.set_mode((1280, 720))
clock = pg.time.Clock()
running = True
dt = 0


raio = 100
d1, d2, d3, d4, gx, gy, gz = 0,0,0,0,0,0,0

player_pos = pg.Vector2(screen.get_width() / 2, screen.get_height() / 2)
incli = pg.Rect(player_pos.x-100,player_pos.y,200,100)


def receive_data():
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(0.01)
        server_address = ('0.0.0.0', 2000)
        sock.bind(server_address)

        try:
            data, address = sock.recvfrom(2000)
            print('Recebido %s bytes de %s' % (len(data), address))
            print(data)

            d1, d2, d3, d4, gx, gy, gz = struct.unpack('<iiiiddd', data)
        except:
            pass



class HoleSprite( pg.sprite.Sprite ):
    def __init__( self ):
        pg.sprite.Sprite.__init__( self )

        self.hole_image = pg.Surface( ( 200, 200 ), pg.SRCALPHA )
        self.hole_image.fill( ( 255,255,255 ) )
        pg.draw.circle(self.hole_image, (0, 0, 0, 0), (100,100), 40)

        # sprite housekeeping
        self.image  = self.hole_image
        self.rect   = self.image.get_rect()
        self.rect.x = player_pos.x  - 100 # centred
        self.rect.y = player_pos.y  - 100
        self.last   = 0

    def update( self ):
        self.image = self.hole_image
        
anims = pg.sprite.GroupSingle()
holey = HoleSprite( )
anims.add( holey )

while running:
    for event in pg.event.get():
        if event.type == pg.QUIT:
            running = False
    
    keys = pg.key.get_pressed()
    if keys[pg.K_w]:
        gy -= .5 * dt
    if keys[pg.K_s]:
        gy += .5 * dt
    if keys[pg.K_a]:
        gx -= .5 * dt
    if keys[pg.K_d]:
        gx += .5 * dt
    
    points = [(player_pos.x-(raio*math.cos(gx)),(player_pos.y-(raio*math.sin(gx)))+gy*40),(player_pos.x+(raio*math.cos(gx)),(player_pos.y+(raio*math.sin(gx)))+gy*40),(player_pos.x-(raio*math.sin(gx)),player_pos.y+(raio*math.cos(gx)))]

    screen.fill("purple")
    
    pg.draw.circle(screen, "red", player_pos, 40)
    pg.draw.polygon(screen, "blue", points)
    anims.draw(screen)
    anims.update()

    receive_data()

    pg.display.flip()

    dt = clock.tick(60) / 1000

pg.quit()
