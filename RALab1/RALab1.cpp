#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <GL/freeglut_std.h>
#include <iostream>
using namespace std;

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

GLuint window;
GLuint width = 512, height = 512;

typedef struct vertex {
	glm::vec3 pos;
} vertex;

typedef struct face {
	vertex v1;
	vertex v2;
	vertex v3;
} face;

vector<vertex> vertices;
vector<face> faces;
vector<vertex> splineVertices;
vector<vertex> spline;
vector<vertex> tangentStarts;
vector<vertex> tangentEnds;

unsigned int numVertices = 0;
unsigned int numFaces = 0;
unsigned int numSplineVertices = 0;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
bool loadOBJ(const char* path, vector<vertex>& out_vertices, vector<face>& faces);

//*********************************************************************************
//	Glavni program.
//*********************************************************************************

int main(int argc, char** argv)
{
	string filename;
	cout << "Unesite ime objekta koji zelite ucitati:\n";
	cin >> filename;
	filename += ".obj";

	loadOBJ(filename.c_str(), vertices, faces);

	cout << "Unesite ime datoteke gdje se nalaze vrhovi B-splajn krivulje:\n";
	cin >> filename;
	filename += ".txt";

	FILE* file = fopen(filename.c_str(), "r");
	if (file == NULL) {
		printf("Impossible to open the file!\n");
		return false;
	}

	while (1) {
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF

		if (strcmp(lineHeader, "v") == 0) {
			vertex vertex;
			int matches = fscanf(file, "%f %f %f\n", &vertex.pos.x, &vertex.pos.y, &vertex.pos.z);
			if (matches != 3) {
				printf("File can't be read\n");
				return false;
			}
			splineVertices.push_back(vertex);
			numSplineVertices++;
		}
	}

	// računanje B-splajn krivulje
	unsigned int numSegments = numSplineVertices - 3;
	
	// prolazimo kroz segmente, racunamo tocke B-splajn krivulje i tangente
	for (int i = 0; i < numSegments; i++) {
		vertex r0 = splineVertices.at(i);
		vertex r1 = splineVertices.at(i + 1);
		vertex r2 = splineVertices.at(i + 2);
		vertex r3 = splineVertices.at(i + 3);

		// t ide od 0 do 1, inkrementiran po 0.01
		for (float t = 0.0; t < 1; t += 0.01) {
			// racunamo T * B / 6
			float f1 = 1.0 / 6.0 * (-1 * pow(t, 3.0) + 3 * pow(t, 2.0) - 3 * t + 1);
			float f2 = 1.0 / 6.0 * (3 * pow(t, 3.0) + - 6 * pow(t, 2.0) + 4);
			float f3 = 1.0 / 6.0 * (-3 * pow(t, 3.0) + 3 * pow(t, 2.0) + 3 * t + 1);
			float f4 = 1.0 / 6.0 * pow(t, 3.0);

			vertex splineVertex;
			splineVertex.pos.x = f1 * r0.pos.x + f2 * r1.pos.x + f3 * r2.pos.x + f4 * r3.pos.x;
			splineVertex.pos.y = f1 * r0.pos.y + f2 * r1.pos.y + f3 * r2.pos.y + f4 * r3.pos.y;
			splineVertex.pos.z = f1 * r0.pos.z + f2 * r1.pos.z + f3 * r2.pos.z + f4 * r3.pos.z;

			// dodajemo tocku u splajn
			spline.push_back(splineVertex);

			// racunamo tangente
			float t1 = 1.0 / 2.0 * (-1 * pow(t, 2.0) + 2 * t - 1);
			float t2 = 1.0 / 2.0 * (3 * pow(t, 2.0) - 4 * t);
			float t3 = 1.0 / 2.0 * (-3 * pow(t, 2.0) + 2 * t + 1);
			float t4 = 1.0 / 2.0 * (pow(t, 2.0));

			vertex tangentStart;
			tangentStart.pos.x = f1 * r0.pos.x + f2 * r1.pos.x + f3 * r2.pos.x + f4 * r3.pos.x;
			tangentStart.pos.y = f1 * r0.pos.y + f2 * r1.pos.y + f3 * r2.pos.y + f4 * r3.pos.y;
			tangentStart.pos.z = f1 * r0.pos.z + f2 * r1.pos.z + f3 * r2.pos.z + f4 * r3.pos.z;

			tangentStarts.push_back(tangentStart);

			float vx = t1 * r0.pos.x + t2 * r1.pos.x + t3 * r2.pos.x + t4 * r3.pos.x;
			float vy = t1 * r0.pos.y + t2 * r1.pos.y + t3 * r2.pos.y + t4 * r3.pos.y;
			float vz = t1 * r0.pos.z + t2 * r1.pos.z + t3 * r2.pos.z + t4 * r3.pos.z;

			vertex tangentEnd;

			tangentEnd.pos.x = tangentStart.pos.x + vx/4;
			tangentEnd.pos.y = tangentStart.pos.y + vy/4;
			tangentEnd.pos.z = tangentStart.pos.z + vz/4;
			
			tangentEnds.push_back(tangentEnd);
		}
	}

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	window = glutCreateWindow("Pracenje putanje");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);

	glutMainLoop();
	return 0;
}

//*********************************************************************************
//	Ucitavanje objekta.
//*********************************************************************************

bool loadOBJ(const char* path, std::vector<vertex>& vertices, vector<face>& faces)
{
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file!\n");
		return false;
	}
	while (1) {
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF

		if (strcmp(lineHeader, "v") == 0) {
			vertex v;
			int matches = fscanf(file, "%f %f %f\n", &v.pos.x, &v.pos.y, &v.pos.z);
			if (matches != 3) {
				printf("File can't be read\n");
				return false;
			}
			v.pos.x *= 4;
			v.pos.y *= 4;
			v.pos.z *= 4;

			vertices.push_back(v);
			numVertices++;
		}

		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[3];
			int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			if (matches != 3) {
				printf("File can't be read\n");
				return false;
			}
			face face;
			face.v1 = vertices.at(vertexIndex[0] - 1);
			face.v2 = vertices.at(vertexIndex[1] - 1);
			face.v3 = vertices.at(vertexIndex[2] - 1);
			faces.push_back(face);
			numFaces++;
		}
	}
}

//*********************************************************************************
//	Osvjezavanje prikaza. 
//*********************************************************************************

void myDisplay(void)
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(-5.0, -5.0, -75.0);

	// crtamo B-splajn krivulju
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < size(spline); i++) {
		glVertex3f(spline.at(i).pos.x, spline.at(i).pos.y, spline.at(i).pos.z);
	}
	glEnd();

	// crtamo tangente (samo 4 po segmentu, ne sve)
	glBegin(GL_LINES);
	for (int i = 0; i < size(tangentStarts); i += 25) {
		glVertex3f(tangentStarts.at(i).pos.x, tangentStarts.at(i).pos.y, tangentStarts.at(i).pos.z);
		glVertex3f(tangentEnds.at(i).pos.x, tangentEnds.at(i).pos.y, tangentEnds.at(i).pos.z);
	}
	glEnd();

	glFlush();
}

//*********************************************************************************
//	Promjena velicine prozora.
//*********************************************************************************

void myReshape(int w, int h)
{
	width = w; height = h;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLoadIdentity();
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
}