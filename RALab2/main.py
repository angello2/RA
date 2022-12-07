from pyglet.gl import *
from random import *
import numpy as np
import time

class SustavCesticaWindow(pyglet.window.Window):
    def __init__(self, width=600, height=800):
        super().__init__(width=width, height=height)
        self.width = width
        self.height = height
        self.tex = pyglet.image.load('smoke.bmp').get_texture()
        self.dim = sustav_cestica(np.array([width/2, 0.0, 0.0]), self.tex, self.width, self.height)

    def on_draw(self):
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClear(GL_COLOR_BUFFER_BIT)
        self.dim.draw()

    def get_dim(self):
        return self.dim

class cestica:
    def __init__(self, poz):
        self.poz = poz.copy()
        self.ds = np.array([uniform(-15.0, 15.0), uniform(20.0, 100.0), 0.0])
        self.zivot = gauss(3000.0, 1000.0)
        self.trenutni_zivot = self.zivot
        self.velicina = 25

    def update(self, dt):
        self.poz += self.ds * dt
        self.velicina = (self.trenutni_zivot/self.zivot) * self.velicina
        self.trenutni_zivot -= dt

class sustav_cestica:

    def __init__(self, pocetni_polozaj, tex, width, height):
        self.cestice = []
        self.pocetni_polozaj = pocetni_polozaj
        self.tex = tex
        self.width = width
        self.height = height

    def dodaj_cestice(self, broj_cestica):
        for i in range(broj_cestica):
            c = cestica(np.array(self.pocetni_polozaj))
            self.cestice.append(c)

    def draw(self):
        glColor3f(1.0, 1.0, 1.0)
        glEnable(self.tex.target)
        glBindTexture(self.tex.target, self.tex.id)
        glEnable(GL_BLEND)
        glBlendFunc(GL_ONE, GL_ONE)
        glBegin(GL_QUADS)
        for p in self.cestice:
            glTexCoord2f(0, 0)
            glVertex3f(p.poz[0] - p.velicina, p.poz[1] - p.velicina, p.poz[2])
            glTexCoord2f(1, 0)
            glVertex3f(p.poz[0] + p.velicina, p.poz[1] - p.velicina, p.poz[2])
            glTexCoord2f(1, 1)
            glVertex3f(p.poz[0] + p.velicina, p.poz[1] + p.velicina, p.poz[2])
            glTexCoord2f(0, 1)
            glVertex3f(p.poz[0] - p.velicina, p.poz[1] + p.velicina, p.poz[2])
        glEnd()
        glDisable(GL_BLEND)
        glDisable(self.tex.target)

    def update(self, dt):
        for i in range(len(self.cestice) - 1):
            c = self.cestice[i]
            c.update(dt)
            if c.trenutni_zivot <= 0 or c.velicina <= 0.1 or c.poz[1] >= self.height or c.poz[0] <= 0 or c.poz[0] >= self.width:
                self.cestice[i] = cestica(self.pocetni_polozaj)

    def get_num(self):
        return len(self.cestice)

if __name__ == '__main__':
    window = SustavCesticaWindow()

    def update(dt):
        dim = window.get_dim()
        if(dim.get_num() < 1000):
            dim.dodaj_cestice(1)

        dim.update(dt)

    pyglet.clock.schedule_interval(update, 1.0/120.0)
    pyglet.app.run()
