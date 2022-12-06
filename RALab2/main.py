from pyglet.gl import *
from random import *
import numpy as np

window = pyglet.window.Window()
tex = pyglet.image.load('smoke.bmp').get_texture()

class cestica:

    def __init__(self, poz):
        self.poz = poz.copy()
        self.poz[0] += gauss(0.0, 10.0)
        self.poz[1] += gauss(-150.0, 50.0)
        self.ds = np.array([uniform(-15.0, 15.0), uniform(50.0, 150.0), 0.0])
        self.zivot = gauss(1500.0, 500.0)
        self.trenutni_zivot = self.zivot
        self.velicina = 25

    def update(self, dt):
        self.poz += self.ds * dt
        self.velicina = (self.trenutni_zivot/self.zivot) * self.velicina
        self.trenutni_zivot -= dt

class sustav_cestica:

    def __init__(self, broj_cestica, pocetni_polozaj):
        self.cestice = []
        self.pocetni_polozaj = pocetni_polozaj
        self.dodaj_cestice(broj_cestica)

    def dodaj_cestice(self, broj_cestica):
        for i in range(broj_cestica):
            c = cestica(np.array(self.pocetni_polozaj))
            self.cestice.append(c)

    def draw(self):
        glColor3f(1.0, 1.0, 1.0)
        glEnable(tex.target)
        glBindTexture(tex.target, tex.id)
        glEnable(GL_BLEND)
        glBlendFunc(GL_ONE, GL_ONE)
        glBegin(GL_QUADS)
        for p in self.cestice:
            velicina = p.velicina
            glTexCoord2f(0, 0)
            glVertex3f(p.poz[0] - velicina, p.poz[1] - velicina, p.poz[2])
            glTexCoord2f(1, 0)
            glVertex3f(p.poz[0] + velicina, p.poz[1] - velicina, p.poz[2])
            glTexCoord2f(1, 1)
            glVertex3f(p.poz[0] + velicina, p.poz[1] + velicina, p.poz[2])
            glTexCoord2f(0, 1)
            glVertex3f(p.poz[0] - velicina, p.poz[1] + velicina, p.poz[2])
        glEnd()
        glDisable(GL_BLEND)
        glDisable(tex.target)

    def update(self, dt):
        for i in range(len(self.cestice) - 1):
            c = self.cestice[i]
            c.update(dt)
            if c.trenutni_zivot <= 0 or c.velicina <= 0.1 or c.poz[1] >= 500:
                self.cestice[i] = cestica(self.pocetni_polozaj)


dim = sustav_cestica(500, np.array([300.0, 0.0, 0.0]))

@window.event
def on_draw():
    glClearColor(0, 0, 0, 0)
    glClear(GL_COLOR_BUFFER_BIT)
    dim.draw()

def update(dt):
    dim.update(dt)

pyglet.clock.schedule_interval(update, 1.0/1000.0)
pyglet.app.run()