#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

#include <time.h>

#include "Transform.h"
#include "Camera.h"

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;

float orbitRadius = 10;
float orbitSpeed = 2;

int randPosRange = 10;
int randRotRange = 360;
int randScaleRange = 3;


const int NUM_CUBES = 8;
Transform transforms[NUM_CUBES];

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Seeding random number generator9
	srand(time(nullptr));

	for (size_t i = 0; i < NUM_CUBES; i++)
	{
		Transform& trans = transforms[i];
		trans.position = glm::vec3((rand() % randPosRange) - (randPosRange / 2), (rand() % randPosRange) - (randPosRange / 2), (rand() % randPosRange) - (randPosRange / 2));
		trans.rotation = glm::vec3(rand() % randRotRange, rand() % randRotRange, rand() % randRotRange);
		trans.scale = glm::vec3((rand() % randScaleRange) + .2f);
	}

	Camera cam;

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Rotate camera
		cam.UpdateEye(orbitSpeed, orbitRadius, deltaTime);

		int width;
		int height;
		glfwGetWindowSize(window, &width, &height);
		float aspectRatio = width / height;

		//cache values so they are only calculated once per frame
		glm::mat4 view = cam.GetViewMatrix();

		glm::mat4 project = cam.GetProjectionMatrix(aspectRatio, .001f, 100.0f);

		//Draw
		shader.use();

		for (size_t i = 0; i < NUM_CUBES; i++)
		{
			shader.setMat4("_Model", transforms[i].GetModelMatrix());
			shader.setMat4("_View", view);

			shader.setMat4("_Project", project);
			
			cubeMesh.draw();
		}

		//Draw UI
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Orbit Radius", &orbitRadius, 1.0f, 50.0f);
		ImGui::SliderFloat("Orbit Speed", &orbitSpeed, 0.0f, 50.0f);
		ImGui::SliderFloat("Field of View", &cam.fov, .5f, 3.0f);
		ImGui::SliderFloat("Orthographic Size", &cam.orthographicSize, 1.0f, 100.0f);
		ImGui::Checkbox("Orthographic", &cam.orthographic);
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Transforms");
		for (size_t i = 0; i < NUM_CUBES; i++)
		{
			ImGui::Text(("Cube " + std::to_string(i)).c_str());
			transforms[i].CreateImGui(i);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}