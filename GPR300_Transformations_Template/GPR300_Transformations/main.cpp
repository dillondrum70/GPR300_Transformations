#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

#include <time.h>

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

float orbitRadius = 3;
float orbitSpeed = 2;
float fov = 1;
float orthographicSize = 1;
bool orthographic = false;


namespace Math3D
{
	glm::mat4 Scale(const glm::vec3& s)
	{
		glm::mat4 model = glm::mat4(1);
		model[0][0] = s.x;
		model[1][1] = s.y;
		model[2][2] = s.z;
		return model;
	}

	glm::mat4 Rotate(const glm::vec3& e)
	{
		glm::mat4 pitch = glm::mat4(1);
		pitch[1][1] = cos(e.x);
		pitch[2][1] = -sin(e.x);
		pitch[1][2] = sin(e.x);
		pitch[2][2] = cos(e.x);

		glm::mat4 yaw = glm::mat4(1);
		yaw[0][0] = cos(e.y);
		yaw[2][0] = sin(e.y);
		yaw[0][2] = -sin(e.y);
		yaw[2][2] = cos(e.y);

		glm::mat4 roll = glm::mat4(1);
		roll[0][0] = cos(e.z);
		roll[1][0] = -sin(e.z);
		roll[0][1] = sin(e.z);
		roll[1][1] = cos(e.z);

		return pitch * yaw * roll;
	}

	glm::mat4 Translate(const glm::vec3& p)
	{
		glm::mat4 model = glm::mat4(1);
		model[3][0] = p.x;
		model[3][1] = p.y;
		model[3][2] = p.z;
		return model;
	}
}

struct Transform 
{
	glm::vec3 position;
	glm::vec3 rotation;	//Euler Angles
	glm::vec3 scale;

	glm::mat4 GetModelMatrix()
	{
		glm::mat4 modelMatrix = glm::mat4(1);

		//TODO
		//Apply transformations
		modelMatrix = Math3D::Scale(scale) * Math3D::Rotate(rotation) * Math3D::Translate(position) * modelMatrix;

		return modelMatrix;
	}
};

struct Camera
{
	glm::vec3 eye = glm::vec3(0, 0, 1);
	glm::vec3 target = glm::vec3(0, 0, 0);
	glm::vec3 worldUp = glm::vec3(0, 1, 0);

	float fov;
	float orthographicSize;
	bool orthographic;

	void UpdateEye(float orbitSpeed, float orbitRadius)
	{
		glm::vec3 forward = glm::normalize(target - eye);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));

		eye = glm::normalize(eye) + (right * orbitSpeed * deltaTime);
		glm::vec3 diff = glm::normalize(glm::vec3(eye - target)) * orbitRadius;
		eye = target + diff;
	}

	glm::mat4 GetViewMatrix()
	{
		glm::mat4 rotation = glm::mat4(1);

		glm::vec3 forward = glm::normalize(target - eye);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));
		
		rotation[0] = glm::vec4(right, 0);
		rotation[1] = glm::vec4(up, 0);
		rotation[2] = glm::vec4(-forward, 0);

		glm::mat4 translation = glm::mat4(1);

		translation[3] = glm::vec4(eye, 1);

		return glm::inverse(rotation) * glm::inverse(translation);
	}

	glm::mat4 GetProjectionMatrixPerspective(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		float c = glm::tan(fov / 2);

		glm::mat4 proj = glm::mat4(1);

		proj[0][0] = 1 / (aspectRatio * c);
		proj[1][1] = 1 / c;
		proj[2][2] = -((farPlane + nearPlane) / (farPlane - nearPlane));
		proj[3][2] = -((2 * farPlane * nearPlane) / (farPlane - nearPlane));
		proj[2][3] = -1;

		return proj;
	}

	glm::mat4 GetProjectionMatrixOrtho(float height, float aspectRatio, float nearPlane, float farPlane)
	{
		float t = height / 2;
		float b = -t;
		float l = b * aspectRatio;
		float r = t * aspectRatio;

		glm::mat4 proj = glm::mat4(1);

		proj[0][0] = 2 / (r - l);
		proj[1][1] = 2 / (t - b);
		proj[2][2] = -2 / (farPlane - nearPlane);
		proj[3][0] = -(r + l) / (r - l);
		proj[3][1] = -(t + b) / (t - b);
		proj[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);

		return proj;
	}
};

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

	srand(time(nullptr));

	for (size_t i = 0; i < NUM_CUBES; i++)
	{
		Transform& trans = transforms[i];
		trans.position = glm::vec3((rand() % 2) - 1, (rand() % 2) - 1, (rand() % 2) - 1);
		trans.rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);
		trans.scale = glm::vec3((rand() % 3) + .2f);
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
		cam.UpdateEye(orbitSpeed, orbitRadius);

		int width;
		int height;
		glfwGetWindowSize(window, &width, &height);
		float aspectRatio = width / height;

		//cache values so they are only calculated once per frame
		glm::mat4 view = cam.GetViewMatrix();
		glm::mat4 ortho = cam.GetProjectionMatrixOrtho(orthographicSize, aspectRatio, .001f, 100.0f);
		glm::mat4 perspective = cam.GetProjectionMatrixPerspective((double)fov, aspectRatio, .001f, 100.f);

		//Draw
		shader.use();

		for (size_t i = 0; i < NUM_CUBES; i++)
		{
			shader.setMat4("_Model", transforms[i].GetModelMatrix());
			shader.setMat4("_View", view);

			if (orthographic)
			{
				shader.setMat4("_Project", ortho);
			}
			else
			{
				shader.setMat4("_Project", perspective);
			}
			
			cubeMesh.draw();
		}

		//Draw UI
		ImGui::SetNextWindowSize(ImVec2(0, 0));	//Size to fit content
		ImGui::Begin("Settings");
		ImGui::SliderFloat("Orbit Radius", &orbitRadius, 1.0f, 50.0f);
		ImGui::SliderFloat("Orbit Speed", &orbitSpeed, 0.0f, 50.0f);
		ImGui::SliderFloat("Field of View", &fov, .5f, 3.0f);
		ImGui::SliderFloat("Orthographic Size", &orthographicSize, 1.0f, 100.0f);
		ImGui::Checkbox("Orthographic", &orthographic);
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