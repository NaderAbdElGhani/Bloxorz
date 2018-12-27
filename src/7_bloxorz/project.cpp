#include <application.hpp>
#include "7_bloxorz/project.hpp"

#include <mesh/mesh_utils.hpp>
#include <textures/texture_utils.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace std;

//bool CheckInBorders(glm::vec3 position);
bool operator <= (glm::vec3 p1, glm::vec3 p2);
bool operator >= (glm::vec3 p1, glm::vec3 p2);
/*
*
* The goal of this scene is just to showcase simple static model loading
* Also to show Anisotropic filtering
*
*/

void BloxorzScene::Initialize() {

	

	shader = new Shader();
	shader->attach("assets/shaders/texture.vert", GL_VERTEX_SHADER);
	shader->attach("assets/shaders/texture.frag", GL_FRAGMENT_SHADER);
	shader->link();
	
	//-----
	VignetteShader = new Shader();
	VignetteShader->attach("assets/shaders/Vignette.vert", GL_VERTEX_SHADER);
	VignetteShader->attach("assets/shaders/Vignette.frag", GL_FRAGMENT_SHADER);
	VignetteShader->link(); 


	//---------color transformation-----------------------
	color = { {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1} };
	clr = true;
	VignetteShader->use();
	VignetteShader->set("colortransform", color);
	shader->use();
	shader->set("colortransform", color);
	//---------------------------------------------------------

	//-------
	// Create quad to render fboTex on
	quad = new Mesh();
	quad->setup<Vertex>({
		{ { -1, -1,  0 },{ 1, 1, 1, 1 },{ 0, 0 },{ 0,0,1 } },
		{ { 1, -1,  0 },{ 1, 1, 1, 1 },{ 1, 0 },{ 0,0,1 } },
		{ { 1,  1,  0 },{ 1, 1, 1, 1 },{ 1, 1 },{ 0,0,1 } },
		{ { -1,  1,  0 },{ 1, 1, 1, 1 },{ 0, 1 },{ 0,0,1 } },
		}, {
			0, 1, 2, 2, 3, 0
		});
	Vignette = true;  //-----
	

	mvpLoc = glGetUniformLocation(shader->getID(), "MVP");
	texLoc = glGetUniformLocation(shader->getID(), "tex");
	mLoc = glGetUniformLocation(shader->getID(), "M");
	mitLoc = glGetUniformLocation(shader->getID(), "M_it");
	vpLoc = glGetUniformLocation(shader->getID(), "VP");
	camPosLoc = glGetUniformLocation(shader->getID(), "cam_pos");

	materialVars.diffuse = glGetUniformLocation(shader->getID(), "material.diffuse");
	materialVars.specular = glGetUniformLocation(shader->getID(), "material.specular");
	materialVars.ambient = glGetUniformLocation(shader->getID(), "material.ambient");
	materialVars.shininess = glGetUniformLocation(shader->getID(), "material.shininess");

	lightVars.diffuse = glGetUniformLocation(shader->getID(), "light.diffuse");
	lightVars.specular = glGetUniformLocation(shader->getID(), "light.specular");
	lightVars.ambient = glGetUniformLocation(shader->getID(), "light.ambient");
	lightVars.position = glGetUniformLocation(shader->getID(), "light.position");
	lightVars.direction = glGetUniformLocation(shader->getID(), "light.direction");


	
	planeMesh = MeshUtils::LoadObj("assets/models/bloxorz/bloxorz.obj");
	 cubeMesh = MeshUtils::LoadObj("assets/models/bloxorz/boxbloxroz.obj");


	planeTex = TextureUtils::Load2DTextureFromFile("assets/models/bloxorz/tiles.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	cubeTex = TextureUtils::Load2DTextureFromFile("assets/models/bloxorz/cube.jpg");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	

	
	camera = new Camera();
	glm::ivec2 windowSize = getApplication()->getWindowSize();
	camera->setupPerspective(glm::pi<float>() / 2, (float)windowSize.x / windowSize.y, 0.1f, 1000.0f);
	camera->setUp({ 0,1, 0 });
	

	controller = new FlyCameraController(this, camera);
	controller->setYaw(-3);
	controller->setPitch(-1);
	controller->setPosition({ 2, 4, 1});
	
	lightPosition = { 3, 5, 5};
	lightYaw = lightPitch = glm::quarter_pi<float>();

	//-----from here

	// Create our additional frame buffer
	fbo = new FrameBuffer();

	unsigned int width = getApplication()->getWindowSize().x;
	unsigned int height = getApplication()->getWindowSize().y;

	// Create our render target
	fboTex = new Texture2D();
	fboTex->bind();
	fboTex->setup(GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	fbo->attach(fboTex, GL_COLOR_ATTACHMENT0);

	fboDepthTex = new Texture2D();
	fboDepthTex->bind();
	fboDepthTex->setup(GL_DEPTH24_STENCIL8, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	fbo->attach(fboDepthTex, GL_DEPTH_STENCIL_ATTACHMENT);

	if (fbo->isComplete())
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.6f, 0.6f, 0.6f, 0.0f);

	boxPosition = { -1.6,0.775,3.45 };
	/*boxPositionFINAL = { -1.6,0.775,3.45 };*/
	boxInitialPosition = { -1.6,0.775,3.45 };
	holePositionLeft = { -2.9,0.7, -4.8 };
	holePositionRight = { -2.8,0.8, -4.7 };
	horizontalX = false;
	horizontalZ = false;
	GameWon = false;
	boxRotation = 0;
	boxRotationAxis = { 1,0,0 };
}

void BloxorzScene::Update(double delta_time) {

	controller->update(delta_time);
	Keyboard* kb = getKeyboard();

	float pitch_speed = 1.0f, yaw_speed = 1.0f;

	if (kb->isPressed(GLFW_KEY_I)) lightPitch += (float)delta_time * pitch_speed;
	if (kb->isPressed(GLFW_KEY_K)) lightPitch -= (float)delta_time * pitch_speed;
	if (kb->isPressed(GLFW_KEY_L)) lightYaw += (float)delta_time * yaw_speed;
	if (kb->isPressed(GLFW_KEY_J)) lightYaw -= (float)delta_time * yaw_speed;

	if (lightPitch < -glm::half_pi<float>()) lightPitch = -glm::half_pi<float>();
	if (lightPitch > glm::half_pi<float>()) lightPitch = glm::half_pi<float>();
	lightYaw = glm::wrapAngle(lightYaw);


	
	if (kb->justPressed(GLFW_KEY_N))
	{
		clr = !clr;
		if (clr)
		{
			color = { {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1} };
		}
		else
		{
			color = { {0.393, 0.349, 0.272,0},{0.769, 0.686, 0.534,0},{0.189, 0.168, 0.131,0},{0,0,0,1} };
		}
		VignetteShader->use();
		VignetteShader->set("colortransform", color);
		shader->use();
		shader->set("colortransform",color);

	}
	if (GameWon) {
		if (kb->isPressed(GLFW_KEY_ENTER)) restartGame();

	}
	if (kb->isPressed(GLFW_KEY_LEFT)&&!GameWon) {
		boxRotation += glm::half_pi<float>();
		boxRotationAxis = { 1,0,0 };

		if (horizontalZ) {
			boxPosition.z += 0.942;
			boxPosition.y = boxInitialPosition.y;
			horizontalZ = false;
			goto left_sleep_label;
		}
		if (horizontalX) {
			boxRotation = 0;
			boxRotation += glm::half_pi<float>();
			boxRotationAxis = { 0,0,1 };
			boxPosition.z += 0.63;
		}
		if ((boxPosition.y) == boxInitialPosition.y) {
			boxPosition.z += 0.942;
			boxPosition.y -= 0.3;
			horizontalZ = true;
		}
	left_sleep_label:
		cout << "x = " << boxPosition.x << ", y = " << boxPosition.y << ", z = " << boxPosition.z << endl;
		Sleep(200);
	}
	if (kb->isPressed(GLFW_KEY_RIGHT) && !GameWon) {
		boxRotation -= glm::half_pi<float>();
		boxRotationAxis = { 1,0,0 };
		if (horizontalZ) {
			boxPosition.z -= 0.942;
			boxPosition.y = boxInitialPosition.y;
			horizontalZ = false;
			goto right_sleep_label;
		}
		if (horizontalX) {
			boxRotation = 0;
			boxRotation -= glm::half_pi<float>();
			boxRotationAxis = { 0,0,1 };
			boxPosition.z -= 0.63;
		}
		if ((boxPosition.y) == boxInitialPosition.y) {
			boxPosition.z -= 0.942;
			boxPosition.y -= 0.3;
			horizontalZ = true;
		}
	right_sleep_label:
		cout << "x = " << boxPosition.x << ", y = " << boxPosition.y << ", z = " << boxPosition.z << endl;
		Sleep(200);
	}
	if (kb->isPressed(GLFW_KEY_UP) && !GameWon) {
		boxRotation += glm::half_pi<float>();
		boxRotationAxis = { 0,0,1 };
		if (horizontalX) {
			boxPosition.x -= 0.94;
			boxPosition.y = boxInitialPosition.y;
			horizontalX = false;
			goto up_sleep_label;
		}
		if (horizontalZ) {
			boxRotation = 0;
			boxRotation += glm::half_pi<float>();
			boxRotationAxis = { 1,0,0 };
			boxPosition.x -= 0.63;
		}
		if ((boxPosition.y) == boxInitialPosition.y) {
			boxPosition.x -= 0.94;
			boxPosition.y -= 0.3;
			horizontalX = true;
		}
	up_sleep_label:
		cout << "x = " << boxPosition.x << ", y = " << boxPosition.y << ", z = " << boxPosition.z << endl;
		Sleep(200);
	}
	if (kb->isPressed(GLFW_KEY_DOWN) && !GameWon) {
		boxRotation -= glm::half_pi<float>();
		boxRotationAxis = { 0,0,1 };
		if (horizontalX) {
			boxPosition.x += 0.94;
			boxPosition.y = boxInitialPosition.y;
			horizontalX = false;
			goto down_sleep_label;
		}
		if (horizontalZ) {
			boxRotation = 0;
			boxRotation -= glm::half_pi<float>();
			boxRotationAxis = { 1,0,0 };
			boxPosition.x += 0.63;
		}
		if ((boxPosition.y) == boxInitialPosition.y) {
			boxPosition.x += 0.94;
			boxPosition.y -= 0.3;
			horizontalX = true;
		}
	down_sleep_label:		cout << "x = " << boxPosition.x << ", y = " << boxPosition.y << ", z = " << boxPosition.z << endl;
		Sleep(200);
	}
	boxRotation = glm::wrapAngle(boxRotation);
	if (boxPosition >= holePositionLeft && boxPosition <= holePositionRight && !GameWon) {
		boxPosition.y -= 2;
		GameWon = true;
	}
	/*Check = CheckInBorders(boxPosition);
	if (Check) {
		boxPositionFINAL = boxPosition;
	}
	else {
		boxPosition = boxPositionFINAL;
	}*/
}

void BloxorzScene::Draw() {

	if (Vignette) fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear colors and depth

	glm::mat4 VP = camera->getVPMatrix();
	glm::vec3 cam_pos = camera->getPosition();
	glm::vec3 light_dir =
		-glm::vec3(glm::cos(lightYaw), 0, glm::sin(lightYaw)) * glm::cos(lightPitch) + glm::vec3(0, glm::sin(lightPitch), 0);;
		
	shader->use();
	glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(VP));
	glUniform3f(camPosLoc, cam_pos.x, cam_pos.y, cam_pos.z);
	glUniform3f(lightVars.diffuse, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightVars.specular, 0.5f, 0.5f, 0.5f);
	glUniform3f(lightVars.ambient, 0.5f, 0.5f, 0.5f);
	glUniform3f(lightVars.direction, light_dir.x,light_dir.y,light_dir.z);
	
	glActiveTexture(GL_TEXTURE0);
	glUniform3f(materialVars.diffuse, 0.2f, 1.0f, 0.4f);
	glUniform3f(materialVars.specular, 0.2f, 0.2f, 0.2f);
	glUniform3f(materialVars.ambient, 0.5f, 0.5f, 0.5f);
	glUniform1f(materialVars.shininess, 200);
	glUniform1i(texLoc, 0);

	

	planeTex->bind();
	glm::mat4 plane_mat = glm::translate(glm::mat4(), { 0, 0, 0 })* glm::scale(glm::mat4(), { 1.1,1,1 });;
	shader->set("M", plane_mat);
	shader->set("M_it", glm::transpose(glm::inverse(plane_mat)));

	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(VP * plane_mat));
	
	planeMesh->draw();

	cubeTex->bind();
	glm::mat4 cube_mat;
	/*if (!Check) {
		boxRotation = 0;
	}
	cube_mat = glm::translate(glm::mat4(), boxPositionFINAL) * glm::rotate(glm::mat4(), boxRotation, boxRotationAxis) * glm::translate(glm::mat4(), { 0.055,0.1,0 });*/
	cube_mat = glm::translate(glm::mat4(), boxPosition) * glm::rotate(glm::mat4(), boxRotation, boxRotationAxis) * glm::translate(glm::mat4(), { 0.055,0.1,0 });
	shader->set("M", cube_mat);
	shader->set("M_it", glm::transpose(glm::inverse(cube_mat)));
 glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(VP * cube_mat));
 glUniform3f(lightVars.specular, 0.9f, 2.0f, 0.6f);
 if (!GameWon) {
	 cubeMesh->draw();
 }
	

	if (Vignette) {
		// Switch back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VignetteShader->use();
		fboTex->bind();
		quad->draw();
	}

}

void BloxorzScene::Finalize() {
	delete controller;
	delete camera;
	delete cubeTex;
	delete planeTex;
	delete cubeMesh;
	delete planeMesh;
	delete shader;
}

//bool CheckInBorders(glm::vec3 position) {
//
//	/*glm::vec3 section1Left = { -2.8,0,3.4 };
//	glm::vec3 section1Right = { -0.3,0,1.5 };
//
//	glm::vec3 section2Left = { -0.9,0,0.9 };
//	glm::vec3 section2Right = { -0.9,0,0.3 };
//
//	glm::vec3 section3Left = { -3.4,0,-0.3};
//	glm::vec3 section3Right = { -0.3,0,-2.2 };
//
//	glm::vec3 section4Left = { -0.9,0,-2.8 };
//	glm::vec3 section4Right = { -0.9,0,-3.4 };
//
//	glm::vec3 section5Left = { -3.4,0,-4.1 };
//	glm::vec3 section5Right = { -0.9,0,-5.3 };*/
//
//	//Section 1
//	if ((float)position.z < 3.5 && (float)position.z >1.4) {
//		if ((float)position.x > -2.9 && (float)position.x < -0.2) {
//			return true;
//		}
//		return false;
//	}
//	//Section 2
//	if ((float)position.z < 1.1 && (float)position.z >0.2) {
//		if ((float)position.x == -0.9) {
//			return true;
//		}
//		return false;
//	}
//	//Section 3
//	if ((float)position.z < -0.2 && (float)position.z > -2.3) {
//		if ((float)position.x > -3.5 && (float)position.x < -0.2) {
//			return true;
//		}
//		return false;
//	}
//	//Section 4
//	if ((float)position.z <-2.7 && (float)position.z >-3.5) {
//		if ((float)position.x == -0.9) {
//			return true;
//		}
//		return false;
//	}
//	//Section 5
//	if ((float)position.z < -4.0 && (float)position.z >-5.4) {
//		if ((float)position.x > -3.5 && (float)position.x < -0.8) {
//			return true;
//		}
//		return false;
//	}
//	return false;
//}

bool operator >= (glm::vec3 p1,glm::vec3 p2) {
	if (p1.x >= p2.x && p1.y >= p2.y && p1.z >= p2.z) {
		return true;
	}
	return false;
}

bool operator <= (glm::vec3 p1, glm::vec3 p2) {
	if (p1.x <= p2.x && p1.y <= p2.y && p1.z <= p2.z) {
		return true;
	}
	return false;
}

void BloxorzScene::restartGame() {
	boxPosition = { -1.6,0.775,3.45 };
	/*boxPositionFINAL = { -1.6,0.775,3.45 };*/
	boxInitialPosition = { -1.6,0.775,3.45 };
	holePositionLeft = { -2.9,0.7, -4.8 };
	holePositionRight = { -2.8,0.8, -4.7 };
	horizontalX = false;
	horizontalZ = false;
	GameWon = false;
	boxRotation = 0;
	boxRotationAxis = { 1,0,0 };
}