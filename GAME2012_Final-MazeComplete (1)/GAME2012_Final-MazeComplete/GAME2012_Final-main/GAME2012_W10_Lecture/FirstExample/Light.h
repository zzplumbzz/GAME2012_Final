#pragma once
#include "glm/glm.hpp"
#include <GL\glew.h>

struct Light
{
	glm::vec3 diffuseColour;
	GLfloat diffuseStrength;
	Light(glm::vec3 dCol, GLfloat dStr)
	{
		diffuseColour = dCol;
		diffuseStrength = dStr;
	}
};

struct AmbientLight
{
	glm::vec3 ambientColour;
	GLfloat ambientStrength;
	AmbientLight(glm::vec3 aCol, GLfloat aStr)
	{
		ambientColour = aCol;
		ambientStrength = aStr;
	}
};

struct PointLight : public Light
{
	glm::vec3 position;
	GLfloat constant, linear, exponent, range;
	PointLight(glm::vec3 pos, GLfloat range,
		glm::vec3 dCol, GLfloat dStr) : Light(dCol, dStr)
	{
		position = pos;
		constant = 1.0f;
		linear = 4.5f / range;
		exponent = 75.0f / (range * range);
	}
};

struct Material
{
	GLfloat specularStrength;
	GLfloat shininess;
};