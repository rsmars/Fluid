#include <iostream>
#include <unordered_map>
#include <algorithm>
#include "glut.h"
#include "FluidInterface.h"
#include "time.h"
using namespace std;
using float4 = SPH::float4;
SPH::System& fs = *getSPHSystem();

SPH::float4 wall_min = { -25, -25, -25 };
SPH::float4 wall_max = { 25, 25, 25 };
SPH::float4 fluid_min = { -15, 0, -15 };
SPH::float4 fluid_max = { 15, 20, 15 };
SPH::float4 gravity = { 0.0, -9.8f, 0 };
//param
const int maxPoints = 10000;
float m_distance;
float4 m_angleXY;
static float dis = 100;
static int px = -1, py = -1;
static int mouse_btn = 0;

void setCamera(float dist, float4 angleXY){
	m_distance = dist;
	m_angleXY = angleXY;
	glMatrixMode(GL_MODELVIEW);// 选择模型观察矩阵   
	glLoadIdentity();// 重置模型观察矩阵   
	gluLookAt(0, 0, m_distance, 0, 0, 0, 0, 1, 0);
	glRotated(m_angleXY.x, 1, 0, 0);
	glRotated(m_angleXY.y, 0, 1, 0);
}
GLvoid ReSizeGLScene(GLsizei width, GLsizei height){
	//fr.reshpae(width, height);
	height = std::max(1, height);//防止除0；	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);// 选择投影矩阵   
	glLoadIdentity();// 重置投影矩阵   
	// 设置视口的大小   
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);
	setCamera(m_distance, m_angleXY);
	glutPostRedisplay();
}
int InitGL(GLvoid){
	glShadeModel(GL_SMOOTH);// 启用阴影平滑 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);// 黑色背景 
	glClearDepth(1.0f);// 设置深度缓存   
	glEnable(GL_DEPTH_TEST);// 启用深度测试   
	glDepthFunc(GL_LEQUAL);// 所作深度测试的类型  
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);// 告诉系统对透视进行修正  
	return true;
}
void glDisplay(void){
	//fr.display();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // 清空屏幕
	glPushMatrix();

	const GLfloat vertex_list[][3] = {
		wall_min.x, wall_min.y, wall_min.z,
		wall_min.x, wall_min.y, wall_max.z,
		wall_min.x, wall_max.y, wall_min.z,
		wall_min.x, wall_max.y, wall_max.z,
		wall_max.x, wall_min.y, wall_min.z,
		wall_max.x, wall_min.y, wall_max.z,
		wall_max.x, wall_max.y, wall_min.z,
		wall_max.x, wall_max.y, wall_max.z
	};
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GLint index_list[][4] = {
		0, 2, 3, 1,
		0, 4, 6, 2,
		0, 1, 5, 4,
		4, 5, 7, 6,
		1, 3, 7, 5,
		2, 6, 7, 3,
	};
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	for (int i = 0; i<6; ++i)         // 有六个面，循环六次
		for (int j = 0; j<4; ++j)     // 每个面有四个顶点，循环四次
			glVertex3fv(vertex_list[index_list[i][j]]);
	glEnd();

	glColor3f(1, 0, 0);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < fs.getPointCounts(); i++){
		const SPH::Point& p = fs.getPointBuf()[i];
		glVertex3f(p.position.x, p.position.y, p.position.z);
	}
	glEnd();
	glPopMatrix();
	glutSwapBuffers();
}

clock_t  start = 0;
bool flag = true;
void glIdle(void){
	clock_t  stop = clock();
	if (stop - start > CLOCKS_PER_SEC / 40){
		fs.tick();
		glutPostRedisplay();
		start = clock();
		fs.printPerformance();
	}
	
	/*angleXY.x = (angleXY.x + 1);
	angleXY.y = (angleXY.y + 1);
	fr.setCamera(dis, angleXY);
	glutPostRedisplay();*/
	//glupdate
}
void mouse(int button, int state, int x, int y){

	if (state == GLUT_DOWN){
		px = x;
		py = y;
		mouse_btn = button;

	}
}
void motion(int x, int y){
	switch (mouse_btn){
	case GLUT_LEFT_BUTTON:
		m_angleXY.y = (m_angleXY.y - 0.1f*(px - x));
		m_angleXY.x = (m_angleXY.x - 0.1f*(py - y));
		break;
	case GLUT_RIGHT_BUTTON:
		dis -= 0.01f*(py - y);
		break;
	default:
		break;
	}
	setCamera(dis, m_angleXY);
	glutPostRedisplay();
	px = x; py = y;
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(400, 400);
	glutCreateWindow("OpenGL程序渲染展示");
	glutDisplayFunc(glDisplay);
	glutIdleFunc(glIdle);
	glutReshapeFunc(ReSizeGLScene);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	InitGL();
	fs.init(maxPoints, wall_min, wall_max, fluid_min, fluid_max, gravity);
	fs.tick();
	glutMainLoop();
	return 0;
}