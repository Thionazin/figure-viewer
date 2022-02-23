#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL/GLSL.h"
#include "MatrixStack/MatrixStack.h"
#include "Program/Program.h"
#include "Shape/Shape.h"
#include "RobotComponent/RobotComponent.h"

using namespace std;

using glm::vec3;

GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Program> progIM; // immediate mode
shared_ptr<Shape> cube;
shared_ptr<Shape> teapot;
shared_ptr<Shape> sphere;
shared_ptr<Shape> slippers;
shared_ptr<RobotComponent> root;
shared_ptr<RobotComponent> selected;
vector<shared_ptr<RobotComponent>> dfs_traversal;
int dfs_index = 0;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if(codepoint == 88) {
		selected->updateX(0.05);
	} else if(codepoint == 120) {
		selected->updateX(-0.05);
	} else if(codepoint == 89) {
		selected->updateY(0.05);
	} else if(codepoint == 121) {
		selected->updateY(-0.05);
	} else if(codepoint == 90) {
		selected->updateZ(0.05);
	} else if(codepoint == 122) {
		selected->updateZ(-0.05);
	} else if(codepoint == 46) {
		if(dfs_index != ((int)dfs_traversal.size())-1) {
			selected->setSelected(false);
			dfs_index++;
			selected = dfs_traversal[dfs_index];
			selected->setSelected(true);
		}
	} else if(codepoint == 44) {
		if(dfs_index != 0) {
			selected->setSelected(false);
			dfs_index--;
			selected = dfs_traversal[dfs_index];
			selected->setSelected(true);
		}

	}
}

// pre computes the dfs traversal only once in the init to increase performance at a slight memory cost.
static void precompute_dfs_traversal()
{
	stack<shared_ptr<RobotComponent>> dfs_store;
	dfs_store.push(root);
	while(!dfs_store.empty()) {
		shared_ptr<RobotComponent> temp = dfs_store.top();
		dfs_store.pop();
		dfs_traversal.push_back(temp);
		for(unsigned int i = 0; i < temp->children.size(); i++) {
			dfs_store.push(temp->children[i]);
		}
	}
}

static void init()
{
	GLSL::checkVersion();

	// Check how many texture units are supported in the vertex shader
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << endl;

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize cube.
	cube = make_shared<Shape>();
	cube->loadMesh(RES_DIR + "cube.obj");
	cube->init();

	// Initialize teapot.
	teapot = make_shared<Shape>();
	teapot->loadMesh(RES_DIR + "teapot.obj");
	teapot->init();

	// Initialize sphere.
	sphere = make_shared<Shape>();
	sphere->loadMesh(RES_DIR + "sphere.obj");
	sphere->init();

	slippers = make_shared<Shape>();
	slippers->loadMesh(RES_DIR + "bunny.obj");
	slippers->init();
	
	// Initialize the GLSL programs.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "nor_vert.glsl", RES_DIR + "nor_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);


	// root vectors
	vec3 trans_1(0.0, 0.0, 0.0);
	vec3 angs(0.0, 0.0, 0.0);
	vec3 trans_2(0.0, 0.0, 0.0);
	vec3 scales(1.8, 3.0, 1.0);
	// root object, the body component
	root = make_shared<RobotComponent>(trans_1, angs, trans_2, scales, cube, prog, false);

	// by default, the selector is on root
	selected = root;
	selected->setSelected(true);

	// head vectors
	vec3 head_trans(0.0, 1.5, 0.0);
	vec3 head_ang(0.0, -1.58, 0.0);
	vec3 head_mesh_trans(0.0, 0.0, 0.0);
	vec3 head_scale(1.0, 1.0, 1.0);
	// head object
	auto rob_head = make_shared<RobotComponent>(head_trans, head_ang, head_mesh_trans, head_scale, teapot, prog, true);
	// add head to root's children
	root->children.push_back(rob_head);

	// left arm, our right
	vec3 left_arm_trans(0.9, 0.8, 0.0);
	vec3 left_arm_ang(0.0, 0.0, 0.0);
	vec3 left_arm_mesh_trans(1.0, 0.0, 0.0);
	vec3 left_arm_scale(2.0, 0.5, 0.5);
	// left arm object
	auto left_arm = make_shared<RobotComponent>(left_arm_trans, left_arm_ang, left_arm_mesh_trans, left_arm_scale, cube, prog, true);
	left_arm->make_rotate(true);
	// add left arm to root's children
	root->children.push_back(left_arm);


	// left forearm, our right
	vec3 left_for_trans(2.0, 0.0, 0.0);
	vec3 left_for_ang(0.0, 0.0, 0.0);
	vec3 left_for_mesh_trans(1.0, 0.0, 0.0);
	vec3 left_for_scale(2.0, 0.5, 0.5);
	// left forearm object
	auto left_for = make_shared<RobotComponent>(left_for_trans, left_for_ang, left_for_mesh_trans, left_for_scale, cube, prog, true);
	// add left forearm to root's children
	left_arm->children.push_back(left_for);

	// right arm, our left
	vec3 right_arm_trans(-0.9, 0.8, 0.0);
	vec3 right_arm_ang(0.0, 0.0, 0.0);
	vec3 right_arm_mesh_trans(-1.0, 0.0, 0.0);
	vec3 right_arm_scale(2.0, 0.5, 0.5);
	// right arm object
	auto right_arm = make_shared<RobotComponent>(right_arm_trans, right_arm_ang, right_arm_mesh_trans, right_arm_scale, cube, prog, true);
	right_arm->make_rotate(true);
	// add right arm to root's children
	root->children.push_back(right_arm);

	// right forearm, our left
	vec3 right_for_trans(-2.0, 0.0, 0.0);
	vec3 right_for_ang(0.0, 0.0, 0.0);
	vec3 right_for_mesh_trans(-1.0, 0.0, 0.0);
	vec3 right_for_scale(2.0, 0.5, 0.5);
	// right forearm object
	auto right_for = make_shared<RobotComponent>(right_for_trans, right_for_ang, right_for_mesh_trans, right_for_scale, cube, prog, true);
	// add right forearm to root's children
	right_arm->children.push_back(right_for);
	
	// left leg, our right
	vec3 left_leg_trans(0.5, -1.5, 0.0);
	vec3 left_leg_ang(0.0, 0.0, 0.0);
	vec3 left_leg_mesh_trans(0.0, -1.0, 0.0);
	vec3 left_leg_scale(0.5, 2.0, 0.5);
	// left leg object
	auto left_leg = make_shared<RobotComponent>(left_leg_trans, left_leg_ang, left_leg_mesh_trans, left_leg_scale, cube, prog, true);
	// add left leg to root's children
	root->children.push_back(left_leg);

	// left calf, our right
	vec3 left_cav_trans(0.0, -2.0, 0.0);
	vec3 left_cav_ang(0.0, 0.0, 0.0);
	vec3 left_cav_mesh_trans(0.0, -1.0, 0.0);
	vec3 left_cav_scale(0.5, 2.0, 0.5);
	// left calf object
	auto left_cav = make_shared<RobotComponent>(left_cav_trans, left_cav_ang, left_cav_mesh_trans, left_cav_scale, cube, prog, true);
	// add left calf to root's children
	left_leg->children.push_back(left_cav);

	// right leg, our right
	vec3 right_leg_trans(-0.5, -1.5, 0.0);
	vec3 right_leg_ang(0.0, 0.0, 0.0);
	vec3 right_leg_mesh_trans(0.0, -1.0, 0.0);
	vec3 right_leg_scale(0.5, 2.0, 0.5);
	// right leg object
	auto right_leg = make_shared<RobotComponent>(right_leg_trans, right_leg_ang, right_leg_mesh_trans, right_leg_scale, cube, prog, true);
	// add right leg to root's children
	root->children.push_back(right_leg);

	// right calf, our right
	vec3 right_cav_trans(0.0, -2.0, 0.0);
	vec3 right_cav_ang(0.0, 0.0, 0.0);
	vec3 right_cav_mesh_trans(0.0, -1.0, 0.0);
	vec3 right_cav_scale(0.5, 2.0, 0.5);
	// right calf object
	auto right_cav = make_shared<RobotComponent>(right_cav_trans, right_cav_ang, right_cav_mesh_trans, right_cav_scale, cube, prog, true);
	// add right calf to root's children
	right_leg->children.push_back(right_cav);

	// right bunny slipper
	vec3 right_slip_trans(0.0, -2.0, 0.0);
	vec3 right_slip_ang(0.0, 1.2, 0.0);
	vec3 right_slip_mesh_trans(0.0, -1.0, 0.0);
	vec3 right_slip_scale(1.0, 1.0, 1.0);
	// right slipper object
	auto right_slip = make_shared<RobotComponent>(right_slip_trans, right_slip_ang, right_slip_mesh_trans, right_slip_scale, slippers, prog, true);
	// add right slipper to root's children
	right_cav->children.push_back(right_slip);

	// left bunny slipper
	vec3 left_slip_trans(0.0, -2.0, 0.0);
	vec3 left_slip_ang(0.0, 1.2, 0.0);
	vec3 left_slip_mesh_trans(0.0, -1.0, 0.0);
	vec3 left_slip_scale(1.0, 1.0, 1.0);
	// left slipper object
	auto left_slip = make_shared<RobotComponent>(left_slip_trans, left_slip_ang, left_slip_mesh_trans, left_slip_scale, slippers, prog, true);
	// add left slipper to root's children
	left_cav->children.push_back(left_slip);

	precompute_dfs_traversal();
	
	progIM = make_shared<Program>();
	progIM->setVerbose(true);
	progIM->setShaderNames(RES_DIR + "simple_vert.glsl", RES_DIR + "simple_frag.glsl");
	progIM->init();
	progIM->addUniform("P");
	progIM->addUniform("MV");
	progIM->setVerbose(false);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(glm::vec3(0, 1, -12));

	// Quick calculation for oscillation of scale for selected
	double t = glfwGetTime();
	double scale_mult = 1 + 0.05/2 + (0.05/2)*sin(2 * M_PI * 2 * t);
	glm::vec3 scale_multiple(scale_mult, scale_mult, scale_mult);
	selected->setScaleMult(scale_multiple);
	
	// Draw teapot.
	prog->bind();
	root->draw(MV, sphere);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
	/*
	MV->pushMatrix();
	MV->translate(x_mov, y_mov, z_mov);
	MV->rotate(t, 0.0, 1.0, 0.0);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
	teapot->draw(prog);
		MV->pushMatrix();
		MV->translate(0.0, 0.5, 0.0);
		MV->rotate(t, 0.0, 1.0, 0.0);
		//glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
		cube->draw(prog);
			MV->pushMatrix();
			MV->translate(0.0, -0.5, 0.0);
			// MV->rotate(t, 0.0, 1.0, 0.0);
			//glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
			sphere->draw(prog);
			MV->popMatrix();
		MV->popMatrix();
	MV->popMatrix();
	*/
	prog->unbind();
	// Draw cube.
	// Draw cube.
	
	// Draw lines.
	/*
	progIM->bind();
	MV->pushMatrix();
	glUniformMatrix4fv(progIM->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
	glUniformMatrix4fv(progIM->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_STRIP);
	glVertex3d(-1.0, -1.0, 0.0);
	glVertex3d( 1.0, -1.0, 0.0);
	glVertex3d( 1.0,  1.0, 0.0);
	glVertex3d(-1.0,  1.0, 0.0);
	glVertex3d(-1.0, -1.0, 0.0);
	glEnd();
	MV->popMatrix();
	progIM->unbind();
	*/

	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// https://en.wikipedia.org/wiki/OpenGL
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Senhe Hao", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set character callback
	glfwSetCharCallback(window, character_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
