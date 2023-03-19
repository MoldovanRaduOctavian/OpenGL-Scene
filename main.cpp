#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Skybox.h"


#include <iostream>

#define MAX(a, b) ((a > b)?(a):(b))
#define MIN(a, b) ((a < b)?(a):(b))
#define CLAMP(x, a, b) (MAX(a, MIN(x, b)))

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat3 lightDirMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
GLuint lightDirMatrixLoc;

// camera
gps::Camera myCamera(
    glm::vec3(50.0f, 10.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.8f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D tree;
GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader mySkyboxShader;
gps::Shader myDepthMapShader;

GLuint shadowMapFBO;
GLuint shadowMapTreeFBO;
GLuint depthMapTexture;
GLuint depthMapTreeTexture;
GLfloat lightAngle;

float directionalLightAngle = 0.f;

const int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

int mouseOk = 0;
int fogEnable = 0, fogButton = 0;
int directionalLightAndShadowsEnable = 1, lightAndShadowsButton = 0;
int treeFalling = 0;

float treeAngle = 0.f;

int displayMode = 1;
int cameraAnimation, cameraAnimationButton;
float cameraAngle = 0.f;

Skybox sky;

void initShadowFBO()
{
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initShadowTreeFBO()
{
    glGenFramebuffers(1, &shadowMapTreeFBO);

    glGenTextures(1, &depthMapTreeTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTreeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapTreeFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTreeTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 getLightSpaceMatrix()
{
    // Asta poate trebuie modificata
    glm::mat4 lightProjectionMatrix = glm::ortho(-400.f, 400.f, -400.f, 400.f, 0.f, 700.f);

    glm::vec3 lightDirTransform = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(directionalLightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(8.21f, 96.977f, 156.382f, 1.0f));
    //glm::mat4 lightView = glm::lookAt(lightDirTransform, myCamera.cameraTarget, glm::vec3(0.f, 1.f, 0.f));

    /*glm::mat4 lightView = glm::lookAt(myCamera.cameraPosition,
        myCamera.cameraFrontDirection + myCamera.cameraPosition,
        glm::vec3(0.0f, 1.0f, 0.0f));*/

    glm::mat4 lightView = glm::lookAt(lightDirTransform, glm::vec3(5.55f, 46.94f, 21.48f), glm::vec3(0.f, 1.f, 0.f));

    return lightProjectionMatrix * lightView;
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO

    //std::cout << "milsugi\n";

    static double lastx = 0.f;
    static double lasty = 0.f;

    if (!mouseOk)
    {
        lastx = xpos;
        lasty = ypos;
        mouseOk = 1;
    }

    //std::cout << "x: " << lastx << " y: " << lasty << '\n';

    double newYaw, newPitch;
    double xdiff, ydiff;
    xdiff = xpos - lastx;
    ydiff = ypos - lasty;

    lastx = xpos;
    lasty = ypos;

    newYaw = myCamera.yaw + xdiff * 0.1f;
    newPitch = myCamera.pitch + ydiff * 0.1f;

    newPitch = CLAMP(newPitch, -89.f, 89.f);

    myCamera.rotate(newPitch, newYaw);
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        /*view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));*/
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        /*/view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));*/
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        /*view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));*/
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        /*view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));*/
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        directionalLightAngle += 0.4f;
        // update model matrix for teapot 
    }

    if (pressedKeys[GLFW_KEY_E]) {
        directionalLightAngle -= 0.4f;
        // update model matrix for teapot
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_B) == GLFW_PRESS)
        cameraAnimation = 1;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_N) == GLFW_PRESS)
        cameraAnimation = 0;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_I) == GLFW_PRESS)
        displayMode = 1;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_O) == GLFW_PRESS)
        displayMode = 2;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_P) == GLFW_PRESS)
        displayMode = 3;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_F) == GLFW_PRESS && fogButton == 1)
    {
        fogEnable = !fogEnable;
        fogButton = 0;
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_F) == GLFW_RELEASE)
        fogButton = 1;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_L) == GLFW_PRESS && lightAndShadowsButton == 1)
    {
        directionalLightAndShadowsEnable = !directionalLightAndShadowsEnable;
        lightAndShadowsButton = 0;
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_L) == GLFW_RELEASE)
        lightAndShadowsButton = 1;

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_T) == GLFW_PRESS)
    {
        treeFalling = 1;
        treeAngle = 0.f;
    }

    if (glfwGetKey(myWindow.getWindow(), GLFW_KEY_Y) == GLFW_PRESS)
        treeFalling = 0;
}

void animateCamera()
{
    cameraAngle += 0.01f;
    myCamera.cameraPosition = glm::vec3(150.f * cos(cameraAngle), 80.f, 150.f * sin(cameraAngle));
    myCamera.cameraTarget = glm::vec3(7.493f, 30.4885f, 24.3641);
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, 1920, 1080);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/sceneTest.obj");
    tree.LoadModel("models/customTree.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    mySkyboxShader.loadShader(
        "shaders/skybox.vert",
        "shaders/skybox.frag"
    );

    myDepthMapShader.loadShader(
        "shaders/depthMap.vert",
        "shaders/depthMap.frag"
    );
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 20.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.f, 0.89f, 0.28f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void initSkybox()
{
    std::vector<std::string> faces =
    {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };

    sky = Skybox(faces);
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix()));

    glm::mat4 model = glm::mat4(1.f);
    glm::vec3 cameraDir = (cameraAnimation == 1)?myCamera.cameraTarget:(myCamera.cameraPosition + myCamera.cameraFrontDirection);

    glm::mat4 view = glm::lookAt(myCamera.cameraPosition, cameraDir, myCamera.cameraUpDirection);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(80.f), 1920.f / 1080.f, 0.1f, 1000.f);

    shader.setMat4f("model", model);
    shader.setMat4f("view", view);
    shader.setMat4f("projection", projection);
    shader.setInt("fogEnable", fogEnable);
    shader.setInt("directionalLightAndShadowsEnable", directionalLightAndShadowsEnable);

    //send teapot model matrix data to shader

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    glViewport(0, 0, 1920, 1080);
    shader.useShaderProgram();

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "shadowMap"), 3);

    glm::vec3 lightDirTransform = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(directionalLightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(8.21f, 96.977f, 156.382f, 1.0f));
    shader.setVec3f("lightDir", lightDirTransform);

    shader.setVec3f("pointLight.position", myCamera.cameraPosition);
    shader.setVec3f("pointLight.ambient", glm::vec3(1.f, 0.f, 0.f));
    shader.setVec3f("pointLight.diffuse", glm::vec3(1.f, 0.f, 0.f));
    shader.setVec3f("pointLight.specular", glm::vec3(1.f, 0.f, 0.f));
    /*shader.setFloat("pointLight.constant", 1.f);
    shader.setFloat("pointLight.linear", 0.09f);
    shader.setFloat("pointLight.quadratic", 0.032);*/

    shader.setFloat("pointLight.constant", 0.6f);
    shader.setFloat("pointLight.linear", 0.039f);
    shader.setFloat("pointLight.quadratic", 0.032f);

    shader.setVec3f("fixedLight1.position", glm::vec3(48.88f, 6.04f, 11.34f));
    shader.setVec3f("fixedLight1.ambient", glm::vec3(0.f, 1.f, 0.f));
    shader.setVec3f("fixedLight1.diffuse", glm::vec3(0.f, 1.f, 0.f));
    shader.setVec3f("fixedLight1.specular", glm::vec3(0.f, 1.f, 0.f));

    shader.setFloat("fixedLight1.constant", 0.6f);
    shader.setFloat("fixedLight1.linear", 0.039f);
    shader.setFloat("fixedLight1.quadratic", 0.032f);

    shader.setVec3f("fixedLight2.position", glm::vec3(18.058f, 5.28f, -11.991f));
    shader.setVec3f("fixedLight2.ambient", glm::vec3(0.f, 0.f, 1.f));
    shader.setVec3f("fixedLight2.diffuse", glm::vec3(0.f, 0.f, 1.f));
    shader.setVec3f("fixedLight2.specular", glm::vec3(0.f, 0.f, 1.f));

    shader.setFloat("fixedLight2.constant", 0.6f);
    shader.setFloat("fixedLight2.linear", 0.039f);
    shader.setFloat("fixedLight2.quadratic", 0.032f);

    model = glm::mat4(1.f);
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);

}

void renderTree(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix()));

    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(-90.37f, 0.f, -29.34f));
    if (treeFalling == 1)
    {
        model = glm::rotate(model, glm::radians(treeAngle), glm::vec3(0.f, 0.f, 1.f));
        treeAngle = (treeAngle < 80.f) ? (treeAngle + 1.f) : 80.f;
    }

    glm::vec3 cameraDir = (cameraAnimation == 1) ? myCamera.cameraTarget : (myCamera.cameraPosition + myCamera.cameraFrontDirection);

    glm::mat4 view = glm::lookAt(myCamera.cameraPosition, cameraDir, myCamera.cameraUpDirection);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(80.f), 1920.f / 1080.f, 0.1f, 1000.f);

    shader.setMat4f("model", model);
    shader.setMat4f("view", view);
    shader.setMat4f("projection", projection);
    shader.setInt("fogEnable", fogEnable);
    shader.setInt("directionalLightAndShadowsEnable", directionalLightAndShadowsEnable);

    //send teapot model matrix data to shader

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    glViewport(0, 0, 1920, 1080);
    shader.useShaderProgram();

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "shadowMap"), 3);


    glm::vec3 lightDirTransform = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(directionalLightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(8.21f, 96.977f, 156.382f, 1.0f));
    shader.setVec3f("lightDir", lightDirTransform);

    shader.setVec3f("pointLight.position", myCamera.cameraPosition);
    shader.setVec3f("pointLight.ambient", glm::vec3(1.f, 0.f, 0.f));
    shader.setVec3f("pointLight.diffuse", glm::vec3(1.f, 0.f, 0.f));
    shader.setVec3f("pointLight.specular", glm::vec3(1.f, 0.f, 0.f));
    /*shader.setFloat("pointLight.constant", 1.f);
    shader.setFloat("pointLight.linear", 0.09f);
    shader.setFloat("pointLight.quadratic", 0.032);*/

    shader.setFloat("pointLight.constant", 0.2f);
    shader.setFloat("pointLight.linear", 0.009f);
    shader.setFloat("pointLight.quadratic", 0.032f);

    shader.setVec3f("fixedLight1.position", glm::vec3(48.88f, 6.04f, 11.34f));
    shader.setVec3f("fixedLight1.ambient", glm::vec3(0.f, 1.f, 0.f));
    shader.setVec3f("fixedLight1.diffuse", glm::vec3(0.f, 1.f, 0.f));
    shader.setVec3f("fixedLight1.specular", glm::vec3(0.f, 1.f, 0.f));

    shader.setFloat("fixedLight1.constant", 0.6f);
    shader.setFloat("fixedLight1.linear", 0.039f);
    shader.setFloat("fixedLight1.quadratic", 0.032f);

    shader.setVec3f("fixedLight2.position", glm::vec3(18.058f, 5.28f, -11.991f));
    shader.setVec3f("fixedLight2.ambient", glm::vec3(0.f, 0.f, 1.f));
    shader.setVec3f("fixedLight2.diffuse", glm::vec3(0.f, 0.f, 1.f));
    shader.setVec3f("fixedLight2.specular", glm::vec3(0.f, 0.f, 1.f));

    shader.setFloat("fixedLight2.constant", 0.6f);
    shader.setFloat("fixedLight2.linear", 0.039f);
    shader.setFloat("fixedLight2.quadratic", 0.032f);

    model = glm::mat4(1.f);
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    tree.Draw(shader);

}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene
    /*myDepthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.f);
    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));
    
    teapot.Draw(myDepthMapShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderTeapot(myBasicShader);*/

    myDepthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.f);
    model = glm::translate(model, glm::vec3(-90.37f, 0.f, -29.34f));
    if (treeFalling == 1)
    {
        model = glm::rotate(model, glm::radians(treeAngle), glm::vec3(0.f, 0.f, 1.f));
        treeAngle = (treeAngle < 80.f) ? (treeAngle + 1.f) : 80.f;
    }
    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    tree.Draw(myDepthMapShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    renderTree(myBasicShader);

	// render the teapot
    myDepthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "lightSpaceMatrix"),
        1, GL_FALSE, glm::value_ptr(getLightSpaceMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    //glClear(GL_DEPTH_BUFFER_BIT);

    model = glm::mat4(1.f);
    glUniformMatrix4fv(glGetUniformLocation(myDepthMapShader.shaderProgram, "model"),
        1, GL_FALSE, glm::value_ptr(model));

    teapot.Draw(myDepthMapShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderTeapot(myBasicShader);


    
    glCheckError();

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
    initShadowFBO();
    //initShadowTreeFBO();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    
    std::vector<std::string> faces =
    {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };

    Skybox sky1 = Skybox(faces);
    sky1.load();

	
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (displayMode)
        {
        case 1: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
        case 2: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case 3: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        }

        processMovement();
        if (cameraAnimation == 1) animateCamera();

        renderScene();

        sky1.draw(mySkyboxShader, myCamera.cameraPosition, (cameraAnimation == 1)?(myCamera.cameraTarget - myCamera.cameraPosition):myCamera.cameraFrontDirection, myCamera.cameraUpDirection, 0);

        /*glm::vec3 target = myCamera.cameraPosition + myCamera.cameraFrontDirection;
        std::cout << "Camera position: " << myCamera.cameraPosition.x << " " << myCamera.cameraPosition.y << " " << myCamera.cameraPosition.z << " " << "\n";
        std::cout << "Camera targert: " << target.x << " " << target.y << " " << target.z << " " << "\n";*/

        glCheckError();

        glfwSwapBuffers(myWindow.getWindow());
        glfwPollEvents();
	}

	cleanup();

    return EXIT_SUCCESS;
}
