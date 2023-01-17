#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <string>
#include <GL/freeglut_std.h>
#include <iostream>
#include <chrono>
#include <thread>
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

vertex S;
vertex s;
vertex e;
vertex os;
vertex ociste;

unsigned int numVertices = 0;
unsigned int numFaces = 0;
unsigned int numSplineVertices = 0;

GLdouble xmax;
GLdouble ymax;
GLdouble zmax;
GLdouble xmin;
GLdouble ymin;
GLdouble zmin;

void myDisplay();
void updatePerspective();
void myReshape(int width, int height);
void myIdle();
bool loadOBJ(const char* path, vector<vertex>& out_vertices, vector<face>& faces);

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

		// t ide od 0 do 1, inkrementiramo po 0.01
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

			// dodajemo tocku u splajn krivulju
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

	// skaliramo i centriramo tocke splajn krivulje i tangenata
	xmax = spline.at(0).pos.x, xmin = vertices.at(0).pos.x;
	ymax = spline.at(0).pos.y, ymin = vertices.at(0).pos.y;
	zmax = spline.at(0).pos.z, zmin = vertices.at(0).pos.z;

	for (int i = 0; i < vertices.size() - 1; i++) {
		if (xmax < vertices.at(i).pos.x) xmax = vertices.at(i).pos.x;
		if (xmin > vertices.at(i).pos.x) xmin = vertices.at(i).pos.x;
		if (ymax < vertices.at(i).pos.y) ymax = vertices.at(i).pos.y;
		if (ymin > vertices.at(i).pos.y) ymin = vertices.at(i).pos.y;
		if (zmax < vertices.at(i).pos.z) zmax = vertices.at(i).pos.z;
		if (zmin > vertices.at(i).pos.z) zmin = vertices.at(i).pos.z;
	}

	vertex spline_S;

	spline_S.pos.x = (xmax + xmin) / 2;
	spline_S.pos.y = (ymax + ymin) / 2;
	spline_S.pos.z = (zmax + zmin) / 2;

	GLdouble M = xmax - xmin;
	if (M < ymax - ymin) M = ymax - ymin;
	if (M < zmax - zmin) M = zmax - zmin;

	for (vertex v : spline) {
		v.pos.x = (v.pos.x - spline_S.pos.x) * 2 / M;
		v.pos.y = (v.pos.y - spline_S.pos.y) * 2 / M;
		v.pos.z = (v.pos.z - spline_S.pos.z) * 2 / M;
	}
	for (vertex v : tangentEnds) {
		v.pos.x = (v.pos.x - spline_S.pos.x) * 2 / M;
		v.pos.y = (v.pos.y - spline_S.pos.y) * 2 / M;
		v.pos.z = (v.pos.z - spline_S.pos.z) * 2 / M;
	}
	for (vertex v : tangentStarts) {
		v.pos.x = (v.pos.x - spline_S.pos.x) * 2 / M;
		v.pos.y = (v.pos.y - spline_S.pos.y) * 2 / M;
		v.pos.z = (v.pos.z - spline_S.pos.z) * 2 / M;
	}

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	window = glutCreateWindow("Pracenje putanje");
	glutReshapeFunc(myReshape);
	glutKeyboardFunc(myKeyboard);
	glutDisplayFunc(myDisplay);
	glutIdleFunc(myIdle);

	glutMainLoop();
	return 0;
}
bool loadOBJ(const char* path, std::vector<vertex>& vertices, vector<face>& faces)
{
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file!\n");
		return false;
	}
	
	S.pos.x = 0.0f;
	S.pos.y = 0.0f;
	S.pos.z = 0.0f;

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

			S.pos.x += v.pos.x;
			S.pos.y += v.pos.y;
			S.pos.z += v.pos.z;

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

	//skaliranje tijela na [-1, 1] i centriranje u (0,0,0)
	xmax = vertices.at(0).pos.x, xmin = vertices.at(0).pos.x;
	ymax = vertices.at(0).pos.y, ymin = vertices.at(0).pos.y;
	zmax = vertices.at(0).pos.z, zmin = vertices.at(0).pos.z;

	for (int i = 0; i < vertices.size() - 1; i++) {
		if (xmax < vertices.at(i).pos.x) xmax = vertices.at(i).pos.x;
		if (xmin > vertices.at(i).pos.x) xmin = vertices.at(i).pos.x;
		if (ymax < vertices.at(i).pos.y) ymax = vertices.at(i).pos.y;
		if (ymin > vertices.at(i).pos.y) ymin = vertices.at(i).pos.y;
		if (zmax < vertices.at(i).pos.z) zmax = vertices.at(i).pos.z;
		if (zmin > vertices.at(i).pos.z) zmin = vertices.at(i).pos.z;
	}

	GLdouble M = xmax - xmin;
	if (M < ymax - ymin) M = ymax - ymin;
	if (M < zmax - zmin) M = zmax - zmin;

	for (vertex v : vertices) {
		v.pos.x = (v.pos.x - S.pos.x) * 2 / M;
		v.pos.y = (v.pos.y - S.pos.y) * 2 / M;
		v.pos.z = (v.pos.z - S.pos.z) * 2 / M;
	}
}

int t = 0;

void myDisplay(void)
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glTranslatef(-5, -5, -75.0);

	// crtamo B-splajn krivulju
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < size(spline); i++) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(spline.at(i).pos.x, spline.at(i).pos.y, spline.at(i).pos.z);
	}
	glEnd();

	// crtamo tangente (samo 4 po segmentu, ne sve)
	glBegin(GL_LINES);
	for (int i = 0; i < size(tangentStarts); i += 25) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(tangentStarts.at(i).pos.x, tangentStarts.at(i).pos.y, tangentStarts.at(i).pos.z);
		glVertex3f(tangentEnds.at(i).pos.x, tangentEnds.at(i).pos.y, tangentEnds.at(i).pos.z);
	}
	glEnd();

	// objekt crtamo na poziciji splajn krivulje
	glTranslatef(spline.at(t).pos.x, spline.at(t).pos.y, spline.at(t).pos.z);

	// pocetna rotacija
	s.pos.x = 0.0f;
	s.pos.y = 0.0f;
	s.pos.z = 1.0f;

	// racunamo ciljnu orijentaciju iz tangente
	e.pos.x = tangentEnds.at(t).pos.x - tangentStarts.at(t).pos.x;
	e.pos.y = tangentEnds.at(t).pos.y - tangentStarts.at(t).pos.y;
	e.pos.z = tangentEnds.at(t).pos.z - tangentStarts.at(t).pos.z;
	
	// racunamo kut
	double normS = glm::l2Norm(s.pos);
	double normE = glm::l2Norm(e.pos);
	double se = glm::dot(s.pos, e.pos);
	double phi = acos(se / (normS * normE));
	phi = phi / (2 * 3.14159265359f) * 360;

	// rotiramo
	glRotatef(phi, os.pos.x, os.pos.y, os.pos.z);

	glTranslatef(-S.pos.x, -S.pos.y, -S.pos.z);

	glBegin(GL_LINES);
	for (int i = 0; i < numFaces; i++) {
		vertex v1 = faces.at(i).v1;
		vertex v2 = faces.at(i).v2;
		vertex v3 = faces.at(i).v3;

		glColor3f(1.0f, 1.0f, 1.0f);

		glVertex3f(v1.pos.x, v1.pos.y, v1.pos.z);
		glVertex3f(v2.pos.x, v2.pos.y, v2.pos.z);

		glVertex3f(v2.pos.x, v2.pos.y, v2.pos.z);
		glVertex3f(v3.pos.x, v3.pos.y, v3.pos.z);

		glVertex3f(v3.pos.x, v3.pos.y, v3.pos.z);
		glVertex3f(v1.pos.x, v1.pos.y, v1.pos.z);
	}
	glEnd();
	t++;

	// pauza da animacija ne ide prebrzo
	this_thread::sleep_for(chrono::milliseconds(10));
	if (t == 100 * (numSplineVertices - 3)) t = 0;

	glFlush();
}******************************************************************************

void myReshape(int w, int h)
{
	width = w; height = h;
	glViewport(0, 0, width, height);
	updatePerspective();
}

void updatePerspective()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)width / height, 0.5, 75.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(ociste.pos.x, ociste.pos.y, ociste.pos.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}
void myIdle() {
	updatePerspective();
	myDisplay();
}
