import pyglet
import numpy as np
import random

class Tetromino:
    def __init__(self):
        self.rotation = 0
        self.shapes = {
            'I': [[1, 1, 1, 1],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'J': [[0, 1, 0, 0],
                  [0, 1, 1, 1],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'L': [[0, 0, 1, 0],
                  [1, 1, 1, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'O': [[0, 1, 1, 0],
                  [0, 1, 1, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'S': [[0, 1, 1, 0],
                  [1, 1, 0, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'T': [[0, 1, 0, 0],
                  [1, 1, 1, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
            'Z': [[1, 1, 0, 0],
                  [0, 1, 1, 0],
                  [0, 0, 0, 0],
                  [0, 0, 0, 0]],
        }
        self.possible_shapes = ['Z', 'T', 'S', 'O', 'L', 'J', 'I']
        self.colors = [
            (255, 0, 0, 255),  # Red
            (0, 255, 0, 255),  # Green
            (0, 0, 255, 255),  # Blue
            (255, 165, 0, 255),  # Orange
            (128, 0, 128, 255),  # Purple
            (255, 192, 203, 255),  # Pink
            (255, 255, 0, 255),  # Yellow
            (165, 42, 42, 255)  # Brown
        ]
        self.shape_name = random.choice(self.possible_shapes)
        self.shape = self.shapes[self.shape_name]
        self.color = random.choice(self.colors)

    def rotate(self):
        return

    def spawn(self):
        if self.shape_name == 'I':





# Set window dimensions
width = 600
height = 800

# Set grid dimensions
grid_width = 300
grid_height = 600
offset = 30

# Create window
window = pyglet.window.Window(width=width, height=height)

score_label = pyglet.text.Label('Score: 0', font_name='Arial',
                              font_size=20, x=100, y=height-50,
                              anchor_x='center', anchor_y='center')

next_piece_label = pyglet.text.Label('Next piece', font_name='Arial',
                              font_size=20, x=width-90, y=height-50,
                              anchor_x='center', anchor_y='center')

grid = np.zeros((20, 10), dtype=float)
vertices = []
rect_size = 30
grid_width_rects = 10
grid_height_rects = 20
for y in range(offset, grid_height_rects*rect_size+offset, rect_size):
    for x in range(offset, grid_width_rects*rect_size+offset, rect_size):
        vertices.append([x, y])
        vertices.append([x+rect_size, y])
        vertices.append([x+rect_size, y+rect_size])
        vertices.append([x, y+rect_size])

vertices_array = np.array(vertices).reshape(grid_height_rects, grid_width_rects, 4, 2)

current_piece = Tetromino()
next_piece = Tetromino()

def draw_next_piece():
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (width - 150, height - 200, width - 150, height - 80)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES,
                         ('v2i', (width - 150, height - 200, width - 30, height - 200)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (width - 150, height - 80, width - 30, height - 80)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES,
                         ('v2i', (width - 30, height - 80, width - 30, height - 200)))

    for x in range(len(next_piece.shape[0])):
        for y in range(len(next_piece.shape[1])):
            if next_piece.shape[x][y]:
                rect_x = width - 120 + x * 30
                rect_y = height - 200 + y * 30
                pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, (
                'v2i', (rect_x, rect_y, rect_x + 30, rect_y, rect_x + 30, rect_y + 30, rect_x, rect_y + 30)),
                                     ('c4B', next_piece.color * 4))


def draw_lines():
    for x in range(offset, grid_width + offset, 30):
        pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (x, offset, x, grid_height + offset)))
    for y in range(offset, grid_height + offset, 30):
        pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (offset, y, grid_width + offset, y)))

    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (offset, offset, grid_width + offset, offset)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (offset, grid_height + offset, grid_width + offset, grid_height + offset)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (offset, offset, offset, grid_height + offset)))
    pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2i', (grid_width + offset, offset, grid_width + offset, grid_height + offset)))

def draw_grid():
    for x in range(grid_width_rects):
        for y in range(grid_height_rects):
            if grid[y][x]:
                pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, (
                    'v2i', (vertices_array[y][x][0][0], vertices_array[y][x][0][1],
                            vertices_array[y][x][1][0], vertices_array[y][x][1][1],
                            vertices_array[y][x][2][0], vertices_array[y][x][2][1],
                            vertices_array[y][x][3][0], vertices_array[y][x][3][1])),
                                         ('c4B', next_piece.color * 4))

@window.event
def on_draw():
    window.clear()
    draw_lines()
    score_label.draw()
    next_piece_label.draw()
    draw_next_piece()
    draw_grid()

def update(dt):
    current_piece.update()

pyglet.clock.schedule_interval(update, 1)
pyglet.app.run()
