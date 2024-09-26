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
	initialiseTextures();
	initialiseScene();
	initialiseSound();
}
Application::~Application() {
	if (window) glfwDestroyWindow(window);
	glfwTerminate();
}
void handleInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void handleWindowResize(GLFWwindow* window, int width, int height);
void Application::initialiseGLFW() {
	printf("Initialising GLFW\n");
	assertFatal(glfwInit(), "Could not initialise GLFW\n");
	// Hint the OpenGL version to the window.
	glfwWindowHint(GLFW_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(windowSize[0], windowSize[1], WINDOW_TITLE, NULL, NULL);
	assertFatal(window != NULL, "Could not create window\n");
	glfwMakeContextCurrent(window);
	// Let the modifier bitfield mods variable in the key callback to store the state of the CAPS LOCK key and others.
	glfwSetInputMode(window, GLFW_LOCK_KEY_MODS, GLFW_TRUE);
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, handleInput);
	glfwSetWindowSizeCallback(window, handleWindowResize);
	// Hide the cursor.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
void Application::initialiseOpenGL() {
	printf("Initialising OpenGL\n");
	assertFatal(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Could not load OpenGL\n");\
		// Disable VSync to allow for infinite framerate.
		glfwSwapInterval(0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	// Enable transparency.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Enable back-face culling.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
}
void Application::initialiseOpenGLShaders() {
	printf("Loading OpenGL shaders\n");
	// The shader manager creates, handles and deletes every shader and program
	// and also, provides a simple interface for setting uniforms.
	shaderMan = new ShaderManager();
	shaderMan->appendShader("basicVert", GL_VERTEX_SHADER, "resources/shaders/basic.vert");
	shaderMan->appendShader("basicFrag", GL_FRAGMENT_SHADER, "resources/shaders/basic.frag");
	shaderMan->createProgram("basic", { "basicVert", "basicFrag" });
	shaderMan->appendShader("textureVert", GL_VERTEX_SHADER, "resources/shaders/texture.vert");
	shaderMan->appendShader("textureFrag", GL_FRAGMENT_SHADER, "resources/shaders/texture.frag");
	shaderMan->createProgram("texture", { "textureVert", "textureFrag" });
	shaderMan->appendShader("shadowVert", GL_VERTEX_SHADER, "resources/shaders/shadow.vert");
	shaderMan->appendShader("shadowFrag", GL_FRAGMENT_SHADER, "resources/shaders/shadow.frag");
	shaderMan->appendShader("shadowGeom", GL_GEOMETRY_SHADER, "resources/shaders/shadow.geom");
	shaderMan->createProgram("shadow", { "shadowVert", "shadowFrag", "shadowGeom" });
	shaderMan->appendShader("quadVert", GL_VERTEX_SHADER, "resources/shaders/quad.vert");
	shaderMan->appendShader("quadFrag", GL_FRAGMENT_SHADER, "resources/shaders/quad.frag");
	shaderMan->createProgram("quad", { "quadVert", "quadFrag" });
}
void Application::initialiseScene() {
	float const ASPECT_RATIO =
		((float)windowSize[0]) / windowSize[1];
	glm::mat4x4 const projection =
		glm::perspective(glm::radians(90.0f), ASPECT_RATIO, 0.1f, 1000.0f);
	camera = new Camera(projection, glm::vec2(glm::radians(-10.0f), 0));
	grid = new Grid(100, 100, 0.5f);
	lightCube = new Cube(glm::vec4(1, 1, 1, 1), LIGHT_POSITION);
	xAxis = new Arrow(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 2.5f);
	yAxis = new Arrow(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 2.5f);
	zAxis = new Arrow(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 2.5f);
	clusters = new Cluster[1];
	model = new Model("resources/models/model.obj", glm::vec4(1.0f, 0.498f, 0.498f, 1.0f), glm::vec3(5.0f, 0.0f, -5.0f), glm::vec2(0.0f, glm::radians(150.0f)), 2.5f);
	walls = new Wall * [1];
	for (int i = 0; i < 1; i += 1) {
		clusters[i].setPosition(INITIAL_CLUSTER_POSITIONS[i]);
		walls[i] = new Wall(&clusters[i], INITIAL_WALL_POSITIONS[i]);
	}
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, SHADOWMAP_FAR_PLANE);
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(LIGHT_POSITION, LIGHT_POSITION + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	glm::mat4x4 const ortho =
		glm::ortho(windowSize[0] / -2.0f, windowSize[0] / 2.0f, windowSize[1] / -2.0f, windowSize[1] / 2.0f, -1.0f, 1.0f);
	shaderMan->setUniform("projection", "quad", ortho);
	hyperCube = new Quad(glm::vec2(0, 300), glm::vec2(368, 58), glm::vec2(0.0475, 0.05), glm::vec2(0.5075, 0.1225));
	gameOver = new Quad(glm::vec2(0, 0), glm::vec2(313, 50), glm::vec2(0.55875, 0.05125), glm::vec2(0.95, 0.11375));
	//pause = new Quad(glm::vec2(0, 0), glm::vec2(313, 50), glm::vec2(0.55875, 0.05125), glm::vec2(0.95, 0.11375));
	getStarted = new Quad(glm::vec2(0, -300), glm::vec2(411, 30), glm::vec2(0.04375, 0.14125), glm::vec2(0.5575, 0.17875));
	_score = new Quad(glm::vec2(-400, 340), glm::vec2(151, 38), glm::vec2(0.62875, 0.14125), glm::vec2(0.8175, 0.18875));
	_timer = new Quad(glm::vec2(350, 340), glm::vec2(139, 36), glm::vec2(0.73, 0.22875), glm::vec2(0.90375, 0.27375));
	instructions = new Quad(glm::vec2(0, -220), glm::vec2(639, 94), glm::vec2(0.085, 0.3075), glm::vec2(0.88375, 0.425));
	digits[0] = new Quad(glm::vec2(0, 0), glm::vec2(26, 35), glm::vec2(0.07875, 0.21375), glm::vec2(0.11125, 0.2575));
	digits[1] = new Quad(glm::vec2(0, 0), glm::vec2(23, 35), glm::vec2(0.1275, 0.21375), glm::vec2(0.15625, 0.2575));
	digits[2] = new Quad(glm::vec2(0, 0), glm::vec2(23, 35), glm::vec2(0.1725, 0.21375), glm::vec2(0.20125, 0.2575));
	digits[3] = new Quad(glm::vec2(0, 0), glm::vec2(23, 35), glm::vec2(0.21875, 0.21375), glm::vec2(0.2475, 0.2575));
	digits[4] = new Quad(glm::vec2(0, 0), glm::vec2(27, 35), glm::vec2(0.26375, 0.21375), glm::vec2(0.2975, 0.2575));
	digits[5] = new Quad(glm::vec2(0, 0), glm::vec2(24, 35), glm::vec2(0.31125, 0.21375), glm::vec2(0.34125, 0.2575));
	digits[6] = new Quad(glm::vec2(0, 0), glm::vec2(25, 36), glm::vec2(0.3575, 0.21375), glm::vec2(0.38875, 0.25875));
	digits[7] = new Quad(glm::vec2(0, 0), glm::vec2(25, 35), glm::vec2(0.40375, 0.21375), glm::vec2(0.435, 0.2575));
	digits[8] = new Quad(glm::vec2(0, 0), glm::vec2(26, 35), glm::vec2(0.45, 0.21375), glm::vec2(0.4825, 0.2575));
	digits[9] = new Quad(glm::vec2(0, 0), glm::vec2(24, 35), glm::vec2(0.4975, 0.21375), glm::vec2(0.5275, 0.2575));
	score = new Number(glm::vec2(-285, 340), 0, digits);
	timer = new Number(glm::vec2(460, 340), 90, digits);
}
void Application::initialiseSound() {
	engine = irrklang::createIrrKlangDevice();
	assert(engine, "Could not create sound engine");
	music = engine->play2D("resources/sounds/warioware.wav", true, false, true);
	music->setVolume(0.3f);
	welcomeSound = engine->addSoundSourceFromFile("resources/sounds/welcome.wav");
	gameOverSound = engine->addSoundSourceFromFile("resources/sounds/game-over.wav");
	hitSounds[0] = engine->addSoundSourceFromFile("resources/sounds/hit-1.wav");
	hitSounds[1] = engine->addSoundSourceFromFile("resources/sounds/hit-2.wav");
	hitSounds[2] = engine->addSoundSourceFromFile("resources/sounds/hit-3.wav");
	hitSounds[3] = engine->addSoundSourceFromFile("resources/sounds/hit-4.wav");
	hitSounds[4] = engine->addSoundSourceFromFile("resources/sounds/hit-5.wav");
	hitSounds[5] = engine->addSoundSourceFromFile("resources/sounds/hit-6.wav");
	passSounds[0] = engine->addSoundSourceFromFile("resources/sounds/pass-1.wav");
	passSounds[1] = engine->addSoundSourceFromFile("resources/sounds/pass-2.wav");
	passSounds[2] = engine->addSoundSourceFromFile("resources/sounds/pass-3.wav");
	passSounds[3] = engine->addSoundSourceFromFile("resources/sounds/pass-4.wav");
	passSounds[4] = engine->addSoundSourceFromFile("resources/sounds/pass-5.wav");
	passSounds[5] = engine->addSoundSourceFromFile("resources/sounds/pass-6.wav");
	float const fxVolume = 0.6;
	welcomeSound->setDefaultVolume(fxVolume);
	gameOverSound->setDefaultVolume(fxVolume);
	hitSounds[0]->setDefaultVolume(fxVolume);
	hitSounds[1]->setDefaultVolume(fxVolume);
	hitSounds[2]->setDefaultVolume(fxVolume);
	hitSounds[3]->setDefaultVolume(fxVolume);
	hitSounds[4]->setDefaultVolume(fxVolume);
	hitSounds[5]->setDefaultVolume(fxVolume);
	passSounds[0]->setDefaultVolume(fxVolume);
	passSounds[1]->setDefaultVolume(fxVolume);
	passSounds[2]->setDefaultVolume(fxVolume);
	passSounds[3]->setDefaultVolume(fxVolume);
	passSounds[4]->setDefaultVolume(fxVolume);
	passSounds[5]->setDefaultVolume(fxVolume);
}
void Application::initialiseTextures() {
	printf("Loading textures\n");
	textureMan = new TextureManager();
	textureMan->loadTexture("brick", GL_TEXTURE_2D, "resources/textures/brick.png");
	textureMan->bindTextureToUnit(1, "brick");
	textureMan->loadTexture("tile", GL_TEXTURE_2D, "resources/textures/moon.jpg");
	textureMan->bindTextureToUnit(2, "tile");
	textureMan->loadTexture("metal", GL_TEXTURE_2D, "resources/textures/metal.jpg");
	textureMan->bindTextureToUnit(3, "metal");
	textureMan->generateCubemap("shadowMap");
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureMan->getTexture("shadowMap").identifier, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	textureMan->bindTextureToUnit(4, "shadowMap");
	textureMan->loadTexture("atlas", GL_TEXTURE_2D, "resources/textures/atlas.png");
	textureMan->bindTextureToUnit(5, "atlas");
	textureMan->loadTexture("space", GL_TEXTURE_2D, "resources/textures/space.jpg");
	textureMan->bindTextureToUnit(6, "space");
	shaderMan->setUniform("textureSampler", "quad", 4);
}
void handleInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Application* application = (Application*)glfwGetWindowUserPointer(window);
	if (application->state == TITLE_SCREEN) {
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
			application->resetCluster();
			application->state = GAME_SCREEN;
		}
	}
	if (application->state == GAME_SCREEN) {
		const float PI = glm::pi<float>();
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			//application->worldRotation.x += glm::radians(10.0f);
			application->clusterRotationVector.y += glm::radians(90.0f);
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			//application->worldRotation.x -= glm::radians(10.0f);
			application->clusterRotationVector.y += glm::radians(-90.0f);
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			//application->worldRotation.y += glm::radians(10.0f);
			application->clusterRotationVector.x += glm::radians(-90.0f);
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			//application->worldRotation.y -= glm::radians(10.0f);
			application->clusterRotationVector.x += glm::radians(90.0f);
		}
	}

	// Pause screen handler
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE) {
		if (!application->paused) {
			application->state = PAUSE_SCREEN;
			application->paused = true;
		}
		else {
			application->state = GAME_SCREEN;
			application->paused = false;
		}
	}
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		application->rotationMode = ROTATE_X;
	}
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		application->rotationMode = ROTATE_Y;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		application->rotationMode = ROTATE_Z;
	}
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		application->enableTextures = !application->enableTextures;
	}
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		application->enableShadows = !application->enableShadows;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		application->enableSmoothMoves = !application->enableSmoothMoves;
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
	//if (!application->enableSmoothMoves && key == GLFW_KEY_W && action == GLFW_PRESS) {
	//	application->clusters[application->currentCluster].position.z += 1.0f;
	//}
	if (!application->enableSmoothMoves && key == GLFW_KEY_A && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].position.x -= 1.0f;
	}
	//if (!application->enableSmoothMoves && key == GLFW_KEY_S && action == GLFW_PRESS) {
	//	application->clusters[application->currentCluster].position.z -= 1.0f;
	//}
	if (!application->enableSmoothMoves && key == GLFW_KEY_D && action == GLFW_PRESS) {
		application->clusters[application->currentCluster].position.x += 1.0f;
	}
	// Because the numerical keys are defined sequentially, this logic can be greatly simplified.
	/*if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5 && action == GLFW_PRESS) {
		application->currentCluster = key - GLFW_KEY_1;
	}*/
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		for (int i = 0; i < 1; i += 1) {
			application->clusters[i].cubes.clear();
			application->clusters[i].generateCluster();
			application->walls[i] = new Wall(&application->clusters[i], application->INITIAL_WALL_POSITIONS[i]);
		}
	}
}
void Application::handleMouse() {
	glm::dvec2 mousePos;
	glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
	glm::dvec2 delta = mousePos - lastMousePos;
	// The zoom factor is calculated by subtracting the magnitude of the last position and current position vectors.
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
void Application::handleKeyboard() {
	const float speed = 100.0f;
	if (enableSmoothMoves && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		clusters[currentCluster].position.x += speed * scheduler.deltaTime;
	}
	if (enableSmoothMoves && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		clusters[currentCluster].position.x -= speed * scheduler.deltaTime;
	}
	if (enableSmoothMoves && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		clusters[currentCluster].position.z += speed * scheduler.deltaTime;
	}
	if (enableSmoothMoves && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		clusters[currentCluster].position.z -= speed * scheduler.deltaTime;
	}
	clusterSpeed = 120.0f;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		clusterSpeed = 360.0f;
	}
}
void handleWindowResize(GLFWwindow* window, int width, int height) {
	Application* application = (Application*)glfwGetWindowUserPointer(window);
	float const ASPECT_RATIO =
		((float)width) / ((float)height);
	glm::mat4x4 const projection =
		glm::perspective(glm::radians(90.0f), ASPECT_RATIO, 0.1f, 1000.0f);
	application->camera->setProjection(projection);
	glViewport(0, 0, width, height);
	application->windowSize[0] = width;
	application->windowSize[1] = height;
}
void Application::render() {
	glViewport(0, 0, windowSize[0], windowSize[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	if (camera->dirty) {
		shaderMan->setUniform("viewProjection", "texture", camera->viewProjection);
		shaderMan->setUniform("viewProjection", "basic", camera->viewProjection);
	}
	glm::mat4 worldRotationMat =
		glm::rotate(glm::mat4(1.0f), worldRotation.x, glm::vec3(0, 1.0f, 0)) *
		glm::rotate(glm::mat4(1.0f), worldRotation.y, glm::vec3(1.0f, 0, 0));
	shaderMan->setUniform("lightPosition", "texture", LIGHT_POSITION);
	shaderMan->setUniform("world", "basic", worldRotationMat);
	shaderMan->setUniform("cameraPosition", "texture", camera->position * glm::mat3(worldRotationMat));
	shaderMan->useProgram("texture");
	shaderMan->setUniform("enableShadows", "texture", enableShadows);
	shaderMan->setUniform("enableTextures", "texture", enableTextures);
	shaderMan->setUniform("shadowMap", "texture", 3);
	shaderMan->setUniform("farPlane", "texture", SHADOWMAP_FAR_PLANE);
	shaderMan->setUniform("world", "texture", worldRotationMat);
	shaderMan->setUniform("object", "texture", glm::mat4(1.0f));
	shaderMan->setUniform("textureSampler", "texture", 1);
	shaderMan->setUniform("specularStrength", "texture", 0.15f);
	grid->render();
	shaderMan->setUniform("enableShadows", "texture", false);
	shaderMan->setUniform("enableTextures", "texture", false);
	shaderMan->setUniform("specularStrength", "texture", 0.6f);
	glCullFace(GL_FRONT);
	model->render(shaderMan, "texture");
	glCullFace(GL_BACK);
	shaderMan->setUniform("enableShadows", "texture", enableShadows);
	shaderMan->setUniform("enableTextures", "texture", enableTextures);
	for (int i = 0; i < 1; i += 1) {
		shaderMan->setUniform("textureSampler", "texture", 2);
		shaderMan->setUniform("specularStrength", "texture", 0.6f);
		clusters[i].render(shaderMan, "texture");
		glm::mat4 wallTransform = glm::translate(glm::mat4(1.0f), walls[i]->position);
		shaderMan->setUniform("object", "texture", wallTransform);
		shaderMan->setUniform("textureSampler", "texture", 0);
		shaderMan->setUniform("specularStrength", "texture", 0.6f);
		walls[i]->render();
	}

	// Create the sky cube around the grid
	glDisable(GL_CULL_FACE);
	Cube* skyCube = new Cube(glm::vec4(96.0f, 96.0f, 96.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f));
	const glm::mat4 i = glm::mat4(1.0f);
	glm::mat4 transform =
		glm::translate(i, glm::vec3(0.0f, 0.0f, 0.0f)) * 
		glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f)) *
		glm::translate(i, skyCube->position);
	shaderMan->setUniform("object", "texture", transform);
	shaderMan->setUniform("textureSampler", "texture", 6);
	skyCube->render();
	delete skyCube;
	glEnable(GL_CULL_FACE);

	shaderMan->setUniform("object", "basic", glm::translate(glm::mat4(1.0f), lightCube->position));
	//lightCube->render();
	shaderMan->setUniform("object", "basic", glm::mat4(1.0f));
	// The depth test is disabled for the axises to allow them to be drawn ontop of everything else.
	glDisable(GL_DEPTH_TEST);
	shaderMan->setUniform("color", "basic", glm::vec4(0, 0, 1.0f, 1.0f));
	shaderMan->setUniform("object", "basic", glm::mat4(1.0f));
	//zAxis->render();
	shaderMan->setUniform("color", "basic", glm::vec4(1.0f, 0, 0, 1.0f));
	shaderMan->setUniform("object", "basic", glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1.0f, 0)));
	//xAxis->render();
	shaderMan->setUniform("color", "basic", glm::vec4(0, 1.0f, 0, 1.0f));
	shaderMan->setUniform("object", "basic", glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0, 0)));
	//yAxis->render();
	shaderMan->useProgram("quad");
	switch (state) {
	case TITLE_SCREEN:
		hyperCube->render(shaderMan, "quad");
		getStarted->render(shaderMan, "quad");
		instructions->render(shaderMan, "quad");
		break;
	case LOSS_SCREEN:
		gameOver->render(shaderMan, "quad");
	case PAUSE_SCREEN:
		//pause->render(shaderMan, "quad");
	case GAME_SCREEN:
		_score->render(shaderMan, "quad");
		_timer->render(shaderMan, "quad");
		score->render(shaderMan, "quad");
		timer->render(shaderMan, "quad");
		break;
	}
	glfwSwapBuffers(window);
	camera->dirty = false;
}
void Application::renderShadowMap() {
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	shaderMan->useProgram("shadow");
	for (GLuint i = 0; i < 6; i += 1)
		shaderMan->setUniform("shadowMatrices[" + std::to_string(i) + "]", "shadow", shadowTransforms[i]);
	shaderMan->setUniform("farPlane", "shadow", SHADOWMAP_FAR_PLANE);
	shaderMan->setUniform("lightPos", "shadow", LIGHT_POSITION);

	shaderMan->setUniform("object", "shadow", glm::mat4(1.0f));
	grid->render();
	model->render(shaderMan, "shadow");
	for (int i = 0; i < 1; i += 1) {
		clusters[i].render(shaderMan, "shadow");
		glm::mat4 wallTransform = glm::translate(glm::mat4(1.0f), walls[i]->position);
		shaderMan->setUniform("object", "shadow", wallTransform);
		walls[i]->render();
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Application::update() {
	handleMouse();
	handleKeyboard();
	camera->update(scheduler.currentTime);
	if (state == GAME_SCREEN) {
		moveCluster();
		clusters[0].rotation = glm::mix(clusters[0].rotation, clusterRotationVector, scheduler.deltaTime * 200.0f);
		camera->position = clusters[0].position + glm::vec3(0.0f, 4.5f, 10.0f);
		if (clusters[0].position.z < walls[0]->position.z && !clusterPassedWall) {
			glm::vec3 normalisedRotationVector = clusterRotationVector;
			const float PI = glm::pi<float>();
			while (normalisedRotationVector.x > 2 * PI) {
				normalisedRotationVector.x -= 2 * PI;
			}
			while (normalisedRotationVector.x < 0) {
				normalisedRotationVector.x += 2 * PI;
			}
			while (normalisedRotationVector.y > 2 * PI) {
				normalisedRotationVector.y -= 2 * PI;
			}
			while (normalisedRotationVector.y < 0) {
				normalisedRotationVector.y += 2 * PI;
			}
			bool clusterFitsWall = glm::length(normalisedRotationVector) < 0.1f;
			if (clusterFitsWall) {
				score->setValue(score->value + 1);
				int i = rand() % 6;
				engine->play2D(passSounds[i]);
				clusterPassedWall = true;
			} else {
				int i = rand() % 6;
				engine->play2D(hitSounds[i]);
				resetCluster();
			}
		}
	}
}
int main(int argc, char const* argv[]) {
	// The application was encapsulated in a class to have a simple way to
	// keep track of its state and to also use the destructor to free resources
	// incase it fails unexpectingly during its initialisation.
	// Indeed, if the termination functions are defined at the end of the main function
	// then they'll never be executed since the program will exit before they are reached.
	Application* application = new Application();
	application->engine->play2D(application->welcomeSound);
	while (!glfwWindowShouldClose(application->window)) {
		glfwPollEvents();
		// The scheduler locks the application to a fixed update interval but 
		// allows it to run at the maximum possible framerate.
		if (application->scheduler.shouldUpdate()) application->update();
		if (application->scheduler.shouldTickSecond() && application->state == GAME_SCREEN) {
			application->timer->setValue(application->timer->value - 1);
			if (application->timer->value == 0) {
				if (application->state != LOSS_SCREEN) {
					application->engine->play2D(application->gameOverSound);
					application->music->stop();
				}
				application->state = LOSS_SCREEN;
			}
		}

		application->renderShadowMap();
		application->render();
		application->scheduler.FPS += 1;
	}
	delete application;
	return 0;
}
void Application::resetCluster() {
	clusterPassedWall = false;
	clusters[0] = Cluster();
	clusters[0].generateCluster();
	clusters[0].setPosition(glm::vec3(0, 0, 20));
	walls[0] = new Wall(&clusters[0], INITIAL_WALL_POSITIONS[0]);
	clusterSpeed = 10.0f;

	int rotations = rand() % 10;
	for (int i = 0; i < rotations; i += 1) {
		switch (rand() % 4) {
		case 0:
			clusters[0].rotation.x += glm::radians(-90.0f);
			break;
		case 1:
			clusters[0].rotation.x += glm::radians(90.0f);
			break;
		case 2:
			clusters[0].rotation.y += glm::radians(-90.0f);
			break;
		case 3:
			clusters[0].rotation.y += glm::radians(90.0f);
			break;
		}
	}
	clusterRotationVector = clusters[0].rotation;
}
void Application::moveCluster() {
	if (clusters[0].position.z > -15.0f) {
		clusters[0].position += glm::vec3(0.0f, 0.0f, -1.0f * clusterSpeed * scheduler.deltaTime);
	} else {
		resetCluster();
	}
}