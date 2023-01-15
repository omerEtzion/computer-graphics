#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Bezier1D.h"
#include <thread>
#include <chrono>

//params:
int seg = 0;
float t = 0;
float t_increment = 0.0001;
int seg_increment = 1;
Bezier1D* b;

static void printMat(const glm::mat4 mat)
{
	std::cout<<" matrix:"<<std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout<< mat[j][i]<<" ";
		std::cout<<std::endl;
	}
}

Game::Game() : Scene()
{
}

Game::Game(float angle ,float relationWH, float near1, float far1) : Scene(angle,relationWH,near1,far1)
{ 	
}

void Game::Init()
{		

	AddShader("../res/shaders/pickingShader");	
	AddShader("../res/shaders/basicShader");
	
	AddTexture("../res/textures/box0.bmp",false);

	seg = 0;
	t = 0;
	t_increment = 0.1;
	seg_increment = 1;

	// TODO what is the diff?
	int segNum = 4;
	int res = 4;
	Bezier1D *bezier = new Bezier1D(segNum, res, 1);
	b = bezier;
	float octahedron_scale = 0.25;
	AddShape(Octahedron, -1, TRIANGLES);
	shapes[0]->MyScale(glm::vec3(octahedron_scale, octahedron_scale, octahedron_scale));
	for(int i = 0; i < segNum; i++){
		for(int j = 1; j < 4; j++){
			glm::vec4 curr_point = bezier->GetControlPoint(i, j);
			// printf("\n\n\nsegment %i, ctrl point %i\n(%f,%f,%f)\n\n\n", i, j, curr_point.x, curr_point.y, curr_point.z);
			AddShapeCopy(0, -1, TRIANGLES);
			shapes.back()->MyTranslate(glm::vec3(curr_point.x, curr_point.y, curr_point.z), 0);
			shapes.back()->MyScale(glm::vec3(octahedron_scale, octahedron_scale, octahedron_scale));
		}
	}
	shapes.push_back(bezier);
	AddShape(Cube, -1, TRIANGLES);

	
	pickedShape = 0;
	
	SetShapeTex(0,0);
	MoveCamera(0,zTranslate,35);
	MoveCamera(0,xTranslate,9);

	pickedShape = -1;
	
	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::MyKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	
	if(action == GLFW_PRESS)
	{
		Shape* cube;
		switch (key)
		{			
			case GLFW_KEY_UP:
				break;

			case GLFW_KEY_C:
				break;

			case GLFW_KEY_SPACE:
				// printf("isActive before = %i\n", isActive);
				if (scn->isActive) {
					scn->Deactivate();
				} else {
					scn->Activate();
				}
				// printf("isActive after = %i\n", isActive);
				break;
			
		default:
			break;
		}
	}
}

void Game::MyMouseCallback(GLFWwindow* window,int button, int action, int mods) 
{
	if(action == GLFW_PRESS )
	{
		Game *scn = (Game*)glfwGetWindowUserPointer(window);
		double x2,y2;
		glfwGetCursorPos(window,&x2,&y2);
		scn->Picking((int)x2,(int)y2);
	}
}

void Game::MyCursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	scn->UpdatePosition((float)xpos,(float)ypos);

	// moving curve
	if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		scn->MouseProccessing(GLFW_MOUSE_BUTTON_RIGHT);
	}

	// rotating curve
	else if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		scn->MouseProccessing(GLFW_MOUSE_BUTTON_LEFT);
	}

}

void Game::MyScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Game *scn = (Game*)glfwGetWindowUserPointer(window);
	scn->MoveCamera(0, zTranslate, yoffset);
	
}

void Game::Update(const glm::mat4 &MVP,const glm::mat4 &Model,const int  shaderIndx)
{
	Shader *s = shaders[shaderIndx];
	int r = ((pickedShape+1) & 0x000000FF) >>  0;
	int g = ((pickedShape+1) & 0x0000FF00) >>  8;
	int b = ((pickedShape+1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal",Model);
	s->SetUniform4f("lightDirection", 0.0f , 0.0f, -1.0f, 0.0f);
	if(shaderIndx == 0)
		s->SetUniform4f("lightColor",r/255.0f, g/255.0f, b/255.0f,1.0f);
	else 
		s->SetUniform4f("lightColor",0.7f,0.8f,0.1f,1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	// Bezier1D *b = (Bezier1D*) shapes[0];
	Shape *cube = shapes.back();
	int segNum = b->GetSegmentsNum();
	if(isActive)
	{
		t += t_increment;
		if (t_increment > 0 && t >= 1) {
			t = 0;
			seg += 1;
		} else if (t_increment < 0 && t <= 0) {
			t = 1;
			seg -= 1;
		}
		
		if (seg == segNum) {
			seg -= 1;
			t = 1;
			t_increment *= -1;
		}

		if (seg < 0) {
			seg = 0;
			t = 0;
			t_increment *= -1;
		}

		glm::vec4 point = b->GetPointOnCurve(seg, t);
		cube->MyTranslate(glm::vec3(), 1);
		cube->MyTranslate(glm::vec3(point.x, point.y, point.z), 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

Game::~Game(void)
{
}
