#pragma once

#include <GLFW/glfw3.h>
#include <irrKlang/irrKlang.h>

#include "arrow.hpp"
#include "camera.hpp"
#include "cluster.hpp"
#include "cube.hpp"
#include "grid.hpp"
#include "model.hpp"
#include "number.hpp"
#include "scheduler.hpp"
#include "shader_manager.hpp"
#include "texture_manager.hpp"
#include "quad.hpp"
#include "wall.hpp"

enum RotationMode {
	ROTATE_X,
	ROTATE_Y,
	ROTATE_Z
};
enum ScreenState {
	TITLE_SCREEN,
	GAME_SCREEN,
	PAUSE_SCREEN,
	LOSS_SCREEN
};
class Application {
public:
	double const UPDATE_INTERVAL = 1.0 / 30.0;
	char const* WINDOW_TITLE = "COMP371	Assignment #1";
	glm::vec3 INITIAL_CLUSTER_POSITIONS[5] = {
		glm::vec3(0, 0, 0),
		glm::vec3(25.0f, 0, 25.0f),
		glm::vec3(-25.0f, 0, 25.0f),
		glm::vec3(25.0f, 0, -25.0f),
		glm::vec3(-25.0f, 0, -25.0f)
	};
	glm::vec3 INITIAL_WALL_POSITIONS[5] = {
		glm::vec3(0, 0, -7.0f),
		glm::vec3(25.0f, 0, 23.0f),
		glm::vec3(-25.0f, 0, 23.0f),
		glm::vec3(25.0f, 0, -27.0f),
		glm::vec3(-25.0f, 0, -27.0f)
	};
	glm::vec3 LIGHT_POSITION = glm::vec3(0, 15.0f, 0);
	GLfloat SHADOWMAP_FAR_PLANE = 25.0f;
public:
	irrklang::ISoundEngine* engine;
	Scheduler scheduler = Scheduler(UPDATE_INTERVAL);
	GLFWwindow* window = nullptr;
	glm::dvec2 lastMousePos = glm::dvec2(0, 0);
	int windowSize[2] = { 1024, 768 };
	ScreenState state = TITLE_SCREEN;
	glm::vec3 clusterRotationVector;
public:
	Camera* camera;
	Grid* grid;
	ShaderManager *shaderMan;
	TextureManager* textureMan;
	Arrow *xAxis, *yAxis, *zAxis;
	Cube* lightCube;
	Model* model;
public:
	glm::vec2 worldRotation = glm::vec2(0, 0);
	Cluster *clusters;
	float clusterSpeed = 120.0f;
	int currentCluster = 0;
	Wall **walls;
	GLuint depthFBO;
	std::vector<glm::mat4> shadowTransforms;
	RotationMode rotationMode;
public:
	bool enableTextures = true;
	bool enableShadows = true;
	bool enableSmoothMoves = false;
	bool clusterPassedWall = true;
	bool paused = false;
public:
	Quad* hyperCube;
	Quad* gameOver;
	Quad* getStarted;
	Quad* pause;
	Quad* _score;
	Quad* _timer;
	Quad* instructions;
	Quad* digits[10];
public:
	Number* score;
	Number* timer;
public:
	irrklang::ISound* music;
	irrklang::ISoundSource* welcomeSound;
	irrklang::ISoundSource* gameOverSound;
	irrklang::ISoundSource* hitSounds[6];
	irrklang::ISoundSource* passSounds[6];
public: 
	Application();
	~Application();
public:
	void handleMouse();
	void handleKeyboard();
	void render();
	void renderShadowMap();
	void update();
public:
	void resetCluster();
	void moveCluster();
private:
	void initialiseGLFW();
	void initialiseOpenGL();
	void initialiseOpenGLShaders();
	void initialiseScene();
	void initialiseSound();
	void initialiseTextures();
};