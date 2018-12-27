#pragma once

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>

#include <scene.hpp>
#include <shader.hpp>
#include <mesh/mesh.hpp>
#include <textures/texture2d.hpp>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>
#include <framebuffer.hpp>

class BloxorzScene : public Scene {
private:
	Shader* shader, *VignetteShader;  //-----
	Mesh* planeMesh, *cubeMesh, *quad;  //-----
	Camera* camera;
	FlyCameraController* controller;

	Texture2D* planeTex ,*cubeTex , *fboTex, *fboDepthTex;  //-----

	GLuint mvpLoc, texLoc;
	glm::vec3 lightPosition;
	float lightYaw, lightPitch;
	GLuint mLoc, mitLoc, vpLoc, camPosLoc;
	FrameBuffer* fbo;  //-----
	bool Vignette;     //-----
	bool clr;
	glm::mat4 color;

	struct {
		GLuint diffuse, specular, ambient, shininess;
	} materialVars;
	struct {
		GLuint diffuse, specular, ambient, position,direction;
		GLuint attenuation_quadratic, attenuation_linear, attenuation_constant;
	} lightVars;
	glm::vec3 boxPosition;
	//glm::vec3 boxPositionFINAL;
	glm::vec3 boxInitialPosition;
	glm::vec3 holePositionLeft;
	glm::vec3 holePositionRight;
	bool horizontalX;
	bool horizontalZ;
	bool GameWon;
	//bool Check;
	glm::vec3 boxRotationAxis;
	float boxRotation;
public:
	BloxorzScene(Application* app) : Scene(app) {}

	void Initialize() override;
	void Update(double delta_time) override;
	void Draw() override;
	void Finalize() override;
	void restartGame();
};

