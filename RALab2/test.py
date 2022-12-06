from pyglet.gl import *
from pyglet.window import key
from pyglet.window import mouse

window = pyglet.window.Window()
triCol = [1.0, 0.0, 0.0]

@window.event
def on_mouse_press(x, y, button, modifiers):
    global triCol
    if button & mouse.LEFT:
        triCol = triCol[2:] + triCol[:2]

@window.event
def on_key_press(symbol, modifiers):
    global triCol
    if modifiers & key.MOD_SHIFT:
        triCol = triCol[1:] + triCol[0:1]

@window.event
def on_draw():
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    glColor3f(triCol[0], triCol[1], triCol[2])
    glBegin(GL_TRIANGLES)
    glVertex2f(0,0)
    glVertex2f(window.width, 0)
    glVertex2f(window.width/2.0, window.height)
    glEnd()

@window.event
def on_resize(width, height):
    glViewport(0,0,width, height)
    glMatrixMode(gl.GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0,width,0,height, -1, 1)
    glMatrixMode(gl.GL_MODELVIEW)

pyglet.app.run()


