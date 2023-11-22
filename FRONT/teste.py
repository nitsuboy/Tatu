import pygame
import random

screen = pygame.display.set_mode((800,800))
pygame.init()
sprite_list = pygame.sprite.Group()


class Block(pygame.sprite.Sprite):
    def __init__(self, size, pos):
        pygame.sprite.Sprite.__init__(self)

        # create transparent background image
        self.image = pygame.Surface( size, pygame.SRCALPHA, 32 )  
        self.rect = self.image.get_rect()
        self.rect.topleft = pos

        # Draw a random polygon into the image
        points = [((self.rect[0]+ self.size[0]/2),self.rect[1]), (self.rect[0], self.rect[1] + self.size[1]), (self.rect[0]+ self.size[0], self.rect[1] + self.size[1])]
        pygame.draw.polygon( self.image, 'white', points )

        # Create the collision mask (anything not transparent)
        self.mask = pygame.mask.from_surface( self.image )  

    def update( self ):
       pass  # TODO  

for i in range(5):
    cube = Block((20,20), (random.randint(0,800),random.randint(0,800)))
    sprite_list.add(cube)

run = True

while run:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False

    screen.fill((255,255,255))

    sprite_list.update()

    sprite_list.draw(screen)
    pygame.display.update()
pythonpygame