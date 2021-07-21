#include <cstdio>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <time.h>

#include "application.hpp"
#include "shader_manager.hpp"
#include "utility.hpp"

Application::Application() {
	initialiseGLFW();
	initialiseOpenGL();
	initialiseOpenGLShaders();
	srand(time(NULL));
	initialiseScene();
}
Application::~Application() {
	if(window) glfwDestroyWindow(window);
	glfwTerminate();
}
void handleInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void Application::initialiseGLFW() {
	printf("Initialising GLFW\n");
	assertFatal(glfwInit(), "Could not initialise GLFW\n");
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_SIZE[0], WINDOW_SIZE[1], WINDOW_TITLE, NULL, NULL);
	assertFatal(window != NULL, "Could not create window\n");
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, handleInput);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
void Application::initialiseOpenGL() {
	printf("Initialising OpenGL\n");
	assertFatal(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Could not load OpenGL\n");
	glfwSwapInterval(0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
}
void Application::initialiseOpenGLShaders() {
	printf("Loading OpenGL shaders\n");
	shaderMan = new ShaderManager();
	shaderMan->appendShader("basicVert", GL_VERTEX_SHADER, "resources/shaders/basic.vert");
	shaderMan->appendShader("basicFrag", GL_FRAGMENT_SHADER, "resources/shaders/basic.frag");
	shaderMan->createProgram("basic", { "basicVert", "basicFrag" });
}
void Application::initialiseScene() {
	float const ASPECT_RATIO =
		((float)Application::WINDOW_SIZE[0]) / Application::WINDOW_SIZE[1];
	glm::mat4x4 const projection =
		glm::perspective(45.0f, ASPECT_RATIO, 0.1f, 1000.0f);
	camera = new Camera(projection, glm::vec2(glm::radians(-10.0f), 0));
	grid = new Grid(100, 100, 0.5f);
	xAxis = new Arrow(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2.5f);
	yAxis = new Arrow(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2.5f);
	zAxis = new Arrow(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2.5f);
	clusters = new Cluster[5];
	clusters[0].setPosition(glm::vec3(0, 0, 0));
	wall = new Wall(&clusters[0]);
	clusters[1].setPosition(glm::vec3(25.0f, 0, 25.0f));
	clusters[2].setPosition(glm::vec3(-25.0f, 0, 25.0f));
	clusters[3].setPosition(glm::vec3(25.0f, 0, -25.0f));
	clusters[4].setPosition(glm::vec3(-25.0f, 0, -25.0f));
}
void handleInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Application* application = (Application*) glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		application->worldRotation.x += glm::radians(10.0f);
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		application->worldRotation.x -= glm::radians(10.0f);
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		application->worldRotation.y += glm::radians(10.0f);
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		application->worldRotation.y -= glm::radians(10.0f);
	}
	if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
		application->worldRotation = glm::vec2(0, 0);
	}
	if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].scale += 0.1f;
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].scale -= 0.1f;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].position.z += 1.0f;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		if (mods & GLFW_MOD_CAPS_LOCK) {
			application->clusters[application->currentCluster].position.x -= 1.0f;
		}
		else {
			application->clusters[application->currentCluster].rotation -= glm::radians(5.0f);
		}
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].position.z -= 1.0f;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		if (mods & GLFW_MOD_CAPS_LOCK) {
			application->clusters[application->currentCluster].position.x += 1.0f;
		}
		else {
			application->clusters[application->currentCluster].rotation += glm::radians(5.0f);
		}
	}
	if (key > GLFW_KEY_1 && key < GLFW_KEY_5 && action == GLFW_PRESS) {
		application->currentCluster = key - GLFW_KEY_1;
	}
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		for (int i = 0; i < 5; i += 1) {
			application->clusters[i].cubes.clear();
			application->clusters[i].generateCluster();
		}
		application->wall = new Wall(&application->clusters[0]);
	}
}
void Application::handleMouse() {
	glm::dvec2 mousePos;
	glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
	glm::dvec2 delta = mousePos - lastMousePos;
	double zoomFactor = glm::distance(glm::dvec2(0), mousePos) - glm::distance(glm::dvec2(0), lastMousePos);
	lastMousePos = mousePos;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		camera->position += camera->forward * (float)(zoomFactor * scheduler.deltaTime * camera->ZOOM_SPEED);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
		delta.x = 0;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) != GLFW_PRESS) {
		delta.y = 0;
	}
	camera->rotate(delta);
}
void Application::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderMan->useProgram("basic");
	glEnable(GL_DEPTH_TEST);
	if (camera->dirty) {
		shaderMan->setUniform("viewProjection", "basic", camera->viewProjection);
	}
	glm::mat4 worldRotationMat =
		glm::rotate(glm::mat4(1.0f), worldRotation.x, glm::vec3(0, 1.0f, 0)) *
		glm::rotate(glm::mat4(1.0f), worldRotation.y, glm::vec3(1.0f, 0, 0));
	shaderMan->setUniform("world", "basic", worldRotationMat);
	shaderMan->setUniform("object", "basic", glm::mat4(1.0f));
	grid->render();
	for (int i = 0; i < 5; i += 1) {
		clusters[i].render(shaderMan);
	}

	shaderMan->setUniform("object", "basic", glm::mat4(1.0f));
	wall->render();
	glDisable(GL_DEPTH_TEST);
	shaderMan->setUniform("object", "basic", glm::mat4(1.0f));
	zAxis->render();
	shaderMan->setUniform("object", "basic", glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1.0f, 0)));
	xAxis->render();
	shaderMan->setUniform("object", "basic", glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0, 0)));
	yAxis->render();
	glfwSwapBuffers(window);
	camera->dirty = false;
}
void Application::update() {
	handleMouse();
	camera->update(scheduler.currentTime);
}
int main(int argc, char const* argv[]) {
	Application* application = new Application();
	while (!glfwWindowShouldClose(application->window)) {
		glfwPollEvents();
		if (application->scheduler.shouldUpdate()) application->update();
		application->render();
		application->scheduler.FPS += 1;
	}
	delete application;
	return 0;
}