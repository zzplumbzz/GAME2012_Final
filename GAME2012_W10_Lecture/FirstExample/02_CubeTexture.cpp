﻿///////////////////////////////////////////////////////////////////////
//
// 01_CubeTexture.cpp
//
///////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdlib>
#include <ctime>
#include "vgl.h"
#include "LoadShaders.h"
#include "Light.h"
#include "Shape.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FPS 60
#define MOVESPEED 0.1f
#define TURNSPEED 0.05f
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,1,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)

enum keyMasks {
	KEY_FORWARD = 0b00000001,		// 0x01 or 1 or 01
	KEY_BACKWARD = 0b00000010,		// 0x02 or 2 or 02
	KEY_LEFT = 0b00000100,
	KEY_RIGHT = 0b00001000,
	KEY_UP = 0b00010000,
	KEY_DOWN = 0b00100000,
	KEY_MOUSECLICKED = 0b01000000
	// Any other keys you want to add.
};

// IDs.
GLuint vao, ibo, points_vbo, colors_vbo, uv_vbo, modelID, viewID, projID;// mvp_ID;

// Matrices.
glm::mat4 MVP, View, Projection;

// Our bitflags. 1 byte for up to 8 keys.
unsigned char keys = 0; // Initialized to 0 or 0b00000000.

// Camera and transform variables.
float scale = 1.0f, angle = 0.0f;
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function
GLfloat pitch, yaw;
int lastX, lastY;

// Texture variables.
GLuint brickTx, blankTx, grassTx, hedgeTx, gateTx, castleTx;
GLint width, height, bitDepth;

//Light variables			Ambient colour		Ambient strength
AmbientLight aLight(glm::vec3(1.0f, 1.0f, 1.0f), 0.5f);

PointLight pLights[2] = { { glm::vec3(7.5f, 1.0f, -10.0f), 10.0f, glm::vec3(1.0f, 1.0f, 0.0f), 10.0f }, //Yellow
						  { glm::vec3(-1.5f, 1.0f, -5.0f), 10.0f, glm::vec3(0.0f, 0.0f, 5.0f), 10.0f } }; //Blue
						 /* { glm::vec3(1.5f, 2.0f, -5.0f), 10.0f, glm::vec3(1.0f, 1.0f, 0.0f), 10.0f },
						  { glm::vec3(8.0f, 2.0f, -1.0f), 10.0f, glm::vec3(1.0f, 0.5f, 0.0f), 10.0f } };*/

void timer(int);

void resetView()
{
	position = glm::vec3(5.0f, 3.0f, 10.0f);
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	pitch = 0.0f;
	yaw = -90.0f;
	// View will now get set only in transformObject
}

// Shapes. Recommend putting in a map
//ground
Grid g_grid(10);
Plane g_plane;
//castle walls
RightWall RWall;
LeftWall LWall;
BackWall BWall;
FrontWallR FWallR;
FrontWallL FWallL;
FrontWallM FWallM;
// gate
Gate gate;
//outer hedge maze
OHedgeMazeF OHMF;
OHedgeMazeR OHMR;
OHedgeMazeL OHML;
OHedgeMazeB OHMB;
// inner hedge maze
IHedgeMaze1 IHM1;
IHedgeMaze2 IHM2;
IHedgeMaze3 IHM3;
IHedgeMaze4 IHM4;
IHedgeMaze5 IHM5;

//Back right prism
TowerPrism BRP(12);
//Front right prism
TowerPrism FRP(12);
//Front left prism
TowerPrism FLP(12);
//Back left prism
TowerPrism BLP(12);

//Back right cone
TowerCone BRC(12);
//Front right cone
TowerCone FRC(12);
//Back left cone
TowerCone BLC(12);
//Front left cone
TowerCone FLC(12);

//Prism g_prism(7);

void init(void)
{
	srand((unsigned)time(NULL));
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	//Loading and compiling shaders
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	//mvp_ID = glGetUniformLocation(program, "MVP");
	modelID = glGetUniformLocation(program, "model");
	projID = glGetUniformLocation(program, "projection");
	viewID = glGetUniformLocation(program, "view");

	// Projection matrix : 45∞ Field of View, aspect ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	resetView();

	// Image loading.
	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load("brick.jpg", &width, &height, &bitDepth, 0);
	if (!image) cout << "Unable to load file!" << endl;

	glGenTextures(1, &brickTx);
	glBindTexture(GL_TEXTURE_2D, brickTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);

	// Second texture. Blank one.

	unsigned char* image2 = stbi_load("blank.jpg", &width, &height, &bitDepth, 0);
	if (!image2) cout << "Unable to load file!" << endl;

	glGenTextures(1, &blankTx);
	glBindTexture(GL_TEXTURE_2D, blankTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image2);

	unsigned char* image3 = stbi_load("grass.png", &width, &height, &bitDepth, 0);
	if (!image3) cout << "Unable to load file!" << endl;

	glGenTextures(1, &grassTx);
	glBindTexture(GL_TEXTURE_2D, grassTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image3);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image3);

	unsigned char* image4 = stbi_load("hedge.png", &width, &height, &bitDepth, 0);
	if (!image4) cout << "Unable to load file!" << endl;

	glGenTextures(1, &hedgeTx);
	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image4);

	unsigned char* image5 = stbi_load("gate.jpg", &width, &height, &bitDepth, 0);
	if (!image5) cout << "Unable to load file!" << endl;

	glGenTextures(1, &gateTx);
	glBindTexture(GL_TEXTURE_2D, gateTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image5);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image5);

	unsigned char* image6 = stbi_load("castle.jpg", &width, &height, &bitDepth, 0);
	if (!image6) cout << "Unable to load file!" << endl;

	glGenTextures(1, &castleTx);
	glBindTexture(GL_TEXTURE_2D, castleTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image6);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image6);

	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

	// Setting ambient Light.
	glUniform3f(glGetUniformLocation(program, "aLight.ambientColour"), aLight.ambientColour.x, aLight.ambientColour.y, aLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.ambientStrength"), aLight.ambientStrength);

	// Setting point lights.
	glUniform3f(glGetUniformLocation(program, "pLights[0].base.diffuseColour"), pLights[0].diffuseColour.x, pLights[0].diffuseColour.y, pLights[0].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].base.diffuseStrength"), pLights[0].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[0].position"), pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].constant"), pLights[0].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[0].linear"), pLights[0].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[0].exponent"), pLights[0].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[1].base.diffuseColour"), pLights[1].diffuseColour.x, pLights[1].diffuseColour.y, pLights[1].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].base.diffuseStrength"), pLights[1].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[1].position"), pLights[1].position.x, pLights[1].position.y, pLights[1].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].constant"), pLights[1].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[1].linear"), pLights[1].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[1].exponent"), pLights[1].exponent);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	ibo = 0;
	glGenBuffers(1, &ibo);

	points_vbo = 0;
	glGenBuffers(1, &points_vbo);

	colors_vbo = 0;
	glGenBuffers(1, &colors_vbo);

	uv_vbo = 0;
	glGenBuffers(1, &uv_vbo);

	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	// Enable depth test.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);

	timer(0);
}

//---------------------------------------------------------------------
//
// calculateView
//
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	View = glm::lookAt(
		position, // Camera position
		position + frontVec, // Look target
		upVec); // Up vector
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);

	calculateView();

	MVP = Projection * View * Model;
	/*glUniformMatrix4fv(mvp_ID, 1, GL_FALSE, &MVP[0][0]);*/
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClearColor(0.3, 0.8, 1.0, 1.0);

	glBindVertexArray(vao);
	// Draw all shapes.

	glBindTexture(GL_TEXTURE_2D, grassTx);
	g_grid.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_LINE_STRIP, g_grid.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, grassTx);
	g_plane.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(10.0f, 10.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, g_plane.NumIndices(), GL_UNSIGNED_SHORT, 0);
	//grid and plane/ ground^
	/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//castse walls
	glBindTexture(GL_TEXTURE_2D, brickTx);
	LWall.ColorShape(1.0f, 0.9f, 0.65f);
	LWall.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, LWall.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	RWall.ColorShape(1.0f, 0.9f, 0.65f);
	RWall.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, RWall.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	BWall.ColorShape(1.0f, 0.9f, 0.65f);
	BWall.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, BWall.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	FWallR.ColorShape(1.0f, 0.9f, 0.65f);
	FWallR.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, FWallR.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	FWallM.ColorShape(1.0f, 0.9f, 0.65f);
	FWallM.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, FWallM.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	FWallL.ColorShape(1.0f, 0.9f, 0.65f);
	FWallL.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, FWallL.NumIndices(), GL_UNSIGNED_SHORT, 0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D, gateTx);
	gate.ColorShape(1.0f, 0.9f, 0.65f);
	gate.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, gate.NumIndices(), GL_UNSIGNED_SHORT, 0);
	//gate^
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//outer hedge maze below
	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	OHMF.ColorShape(1.0f, 0.9f, 0.65f);
	OHMF.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, OHMF.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	OHMR.ColorShape(1.0f, 0.9f, 0.65f);
	OHMR.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, OHMR.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	OHML.ColorShape(1.0f, 0.9f, 0.65f);
	OHML.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, OHML.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	OHMB.ColorShape(1.0f, 0.9f, 0.65f);
	OHMB.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, OHMB.NumIndices(), GL_UNSIGNED_SHORT, 0);
	/// outer hedge maze^
	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// inner hedge maze
	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	IHM1.ColorShape(1.0f, 0.9f, 0.65f);
	IHM1.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, IHM1.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	IHM2.ColorShape(1.0f, 0.9f, 0.65f);
	IHM2.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, IHM2.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	IHM3.ColorShape(1.0f, 0.9f, 0.65f);
	IHM3.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, IHM3.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	IHM4.ColorShape(1.0f, 0.9f, 0.65f);
	IHM4.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, IHM4.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	IHM5.ColorShape(1.0f, 0.9f, 0.65f);
	IHM5.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(5.0f, 2.0f, 2.0f), X_AXIS, 0.0f, glm::vec3(2.5f, 0.0f, -3.5f));
	glDrawElements(GL_TRIANGLES, IHM5.NumIndices(), GL_UNSIGNED_SHORT, 0);

	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Tower prism
	glBindTexture(GL_TEXTURE_2D, brickTx);
	//BRP.ColorShape(1.0f, 0.9f, 0.65f);
	BRP.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.0f, 2.5f, 1.0f), X_AXIS, 0.0f, glm::vec3(9.7f, 0.0f, -10.9f));
	glDrawElements(GL_TRIANGLES, BRP.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	FRP.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.0f, 2.5f, 1.0f), X_AXIS, 0.0f, glm::vec3(9.7f, 0.0f, -0.2f));
	glDrawElements(GL_TRIANGLES, FRP.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	FLP.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.0f, 2.5f, 1.0f), X_AXIS, 0.0f, glm::vec3(-0.8f, 0.0f, -0.2f));
	glDrawElements(GL_TRIANGLES, FLP.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	BLP.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.0f, 2.5f, 1.0f), X_AXIS, 0.0f, glm::vec3(-0.8f, 0.0f, -10.9f));
	glDrawElements(GL_TRIANGLES, BLP.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, castleTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	BRC.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.5f, 1.0f, 1.5f), X_AXIS, 0.0f, glm::vec3(9.45f, 2.5f, -11.15f));
	glDrawElements(GL_TRIANGLES, BRC.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, castleTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	FRC.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.5f, 1.0f, 1.5f), X_AXIS, 0.0f, glm::vec3(9.45f, 2.5f, -0.45f));
	glDrawElements(GL_TRIANGLES, FRC.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, castleTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	BLC.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.5f, 1.0f, 1.5f), X_AXIS, 0.0f, glm::vec3(-1.05f, 2.5f, -11.15f));
	glDrawElements(GL_TRIANGLES, BLC.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, castleTx);
	//.ColorShape(1.0f, 0.9f, 0.65f);
	FLC.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo);
	transformObject(glm::vec3(1.5f, 1.0f, 1.5f), X_AXIS, 0.0f, glm::vec3(-1.05f, 2.5f, -0.45f));
	glDrawElements(GL_TRIANGLES, FLC.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0); // Done writing.
	glutSwapBuffers(); // Now for a potentially smoother render.
}

void parseKeys()
{
	if (keys & KEY_FORWARD)
		position += frontVec * MOVESPEED;
	else if (keys & KEY_BACKWARD)
		position -= frontVec * MOVESPEED;
	if (keys & KEY_LEFT)
		position -= rightVec * MOVESPEED;
	else if (keys & KEY_RIGHT)
		position += rightVec * MOVESPEED;
	if (keys & KEY_UP)
		position.y += MOVESPEED;
	else if (keys & KEY_DOWN)
		position.y -= MOVESPEED;
}

void timer(int) { // essentially our update()
	parseKeys();
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, timer, 0); // 60 FPS or 16.67ms.
}

//---------------------------------------------------------------------
//
// keyDown
//
void keyDown(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; break;
	case 's':
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD; break;
	case 'a':
		if (!(keys & KEY_LEFT))
			keys |= KEY_LEFT; break;
	case 'd':
		if (!(keys & KEY_RIGHT))
			keys |= KEY_RIGHT; break;
	case 'r':
		if (!(keys & KEY_UP))
			keys |= KEY_UP; break;
	case 'f':
		if (!(keys & KEY_DOWN))
			keys |= KEY_DOWN; break;
	}
}

void keyDownSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
	}
}

void keyUp(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		keys &= ~KEY_FORWARD; break;
	case 's':
		keys &= ~KEY_BACKWARD; break;
	case 'a':
		keys &= ~KEY_LEFT; break;
	case 'd':
		keys &= ~KEY_RIGHT; break;
	case 'r':
		keys &= ~KEY_UP; break;
	case 'f':
		keys &= ~KEY_DOWN; break;
	case ' ':
		resetView();
	}
}

void keyUpSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		keys &= ~KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		keys &= ~KEY_BACKWARD;
	}
}

void mouseMove(int x, int y)
{
	if (keys & KEY_MOUSECLICKED)
	{
		pitch += (GLfloat)((y - lastY) * TURNSPEED);
		yaw -= (GLfloat)((x - lastX) * TURNSPEED);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		keys |= KEY_MOUSECLICKED; // Flip flag to true
		glutSetCursor(GLUT_CURSOR_NONE);
		//cout << "Mouse clicked." << endl;
	}
	else
	{
		keys &= ~KEY_MOUSECLICKED; // Reset flag to false
		glutSetCursor(GLUT_CURSOR_INHERIT);
		//cout << "Mouse released." << endl;
	}
}

void clean()
{
	cout << "Cleaning up!" << endl;
	glDeleteTextures(1, &brickTx);
	glDeleteTextures(1, &blankTx);
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("GAME2012_Final");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutSpecialFunc(keyDownSpec);
	glutKeyboardUpFunc(keyUp); // New function for third example.
	glutSpecialUpFunc(keyUpSpec);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove); // Requires click to register.

	atexit(clean); // This GLUT function calls specified function before terminating program. Useful!

	glutMainLoop();

	return 0;
}