#pragma once

#include <iostream>
#include <vector>
#define PI 3.14159265358979324
using namespace std;

struct Shape
{
	vector<GLshort> shape_indices;
	vector<GLfloat> shape_vertices;
	vector<GLfloat> shape_colors;
	vector<GLfloat> shape_uvs;

	~Shape()
	{
		shape_indices.clear();
		shape_indices.shrink_to_fit();
		shape_vertices.clear();
		shape_vertices.shrink_to_fit();
		shape_colors.clear();
		shape_colors.shrink_to_fit();
		shape_uvs.clear();
		shape_uvs.shrink_to_fit();
	}
	GLsizei NumIndices() { return shape_indices.size(); }
	void BufferShape(GLuint* ibo, GLuint* points_vbo, GLuint* colors_vbo, GLuint* uv_vbo)
	{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices[0]) * shape_indices.size(), &shape_indices.front(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, *points_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices[0]) * shape_vertices.size(), &shape_vertices.front(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(shape_vertices[0]) * 3, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, *colors_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(shape_colors[0]) * shape_colors.size(), &shape_colors.front(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);	

			glBindBuffer(GL_ARRAY_BUFFER, *uv_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(shape_uvs[0]) * shape_uvs.size(), &shape_uvs.front(), GL_STATIC_DRAW);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(2);

			glBindBuffer(GL_ARRAY_BUFFER, 0); 
	}
	void ColorShape(GLfloat r, GLfloat g, GLfloat b)
	{
		shape_colors.clear();
		shape_colors.shrink_to_fit();
		for (int i = 0; i < shape_vertices.size(); i+=3)
		{
			shape_colors.push_back(r);
			shape_colors.push_back(g);
			shape_colors.push_back(b);
		}
		shape_colors.shrink_to_fit(); // Good idea after a bunch of pushes.
	}
};

struct Plane : public Shape // Vertical plane of 1x1 units across.
{
	Plane()
	{
		shape_indices = {
			0, 1, 2,
			2, 3, 0
		};
		shape_vertices = {
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,		
			1.0f, 1.0f, 0.0f,		
			0.0f, 1.0f, 0.0f
		};
		for (int i = 0; i < shape_vertices.size(); i += 3)
		{
			shape_uvs.push_back(shape_vertices[i]);
			shape_uvs.push_back(shape_vertices[i + 1]);
		}
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};

struct Grid : public Shape // Flat grid on ground. Starts at 0,0,0 and increases into positive.
{
	Grid(int quads)
	{
		for (int row = 0; row <= quads; row++)
		{
			for (int col = 0; col <= quads; col++)
			{
				shape_vertices.push_back(col);
				shape_vertices.push_back(row);
				shape_vertices.push_back(0.0f);
			}
		}
		int i = 0;
		for (int row = 0; row < (quads * (quads + 1)); row = i * (quads + 1))
		{
			if (i % 2 == 0) // Even row: l->r
			{
				for (int col = 0; col < quads; col++)
				{
					// Triangle one.
					shape_indices.push_back(row + col);
					shape_indices.push_back(row + (col + 1));
					shape_indices.push_back(row + (quads + 1) + (col + 1));
					// Triangle two.
					shape_indices.push_back(row + (quads + 1) + (col + 1));
					shape_indices.push_back(row + (quads + 1) + col);
					shape_indices.push_back(row + col);
				}
			}
			else // Odd row: r->l
			{
				for (int col = quads - 1; col >= 0; col--)
				{
					// Triangle one.
					shape_indices.push_back(row + col);
					shape_indices.push_back(row + (col + 1));
					shape_indices.push_back(row + (quads + 1) + (col + 1));
					// Triangle two.
					shape_indices.push_back(row + (quads + 1) + (col + 1));
					shape_indices.push_back(row + (quads + 1) + col);
					shape_indices.push_back(row + col);
				}
			}
			i++;
		}
		for (int i = 0; i < shape_vertices.size(); i += 3)
		{
			shape_uvs.push_back(0); // No texture for grid so value doesn't matter.
			shape_uvs.push_back(0);
		}
		ColorShape(1.0f, 0.0f, 1.0f);
	}
};
struct RightWall : public Shape
{
	RightWall()
	{
		
		shape_indices = {
			// Front.
			0, 1, 2,
			2, 3, 0,
			// Right.
			4, 5, 6,
			6, 7, 4,
			// Back.
			8, 9, 10,
			10, 11, 8,
			// Left.
			12, 13, 14,
			14, 15, 12,
			// Top.
			16, 17, 18,
			18, 19, 16,
			// Bottom.
			20, 21, 22,
			22, 23, 20
		};
		shape_vertices = {
			// Front.
			1.45f, 0.0f, 2.0f,		// 0.
			1.5f, 0.0f, 2.0f,		// 1.
			1.5f, 1.0f, 2.0f,		// 2.
			1.45f, 1.0f, 2.0f,		// 3.
			// Right.
			1.5f, 0.0f, 2.0f,		// 1. 4
			1.5f, 0.0f, -3.3f,		// 5. 5
			1.5f, 1.0f, -3.3f,		// 6. 6
			1.5f, 1.0f, 2.0f,		// 2. 7
			// Back.
			1.45f, 0.0f, -3.3f,		// 5. 8
			1.5f, 0.0f, -3.3f,		// 4. 9
			1.5f, 1.0f, -3.3f,		// 7. 10
			1.45f, 1.0f, -3.3f,		// 6. 11
			// Left.
			1.45f, 0.0f, -3.3f,		// 4. 12
			1.45f, 0.0f, 2.0f,		// 0. 13
			1.45f, 1.0f, 2.0f,		// 3. 14
			1.45f, 1.0f, -3.3f,		// 7. 15
			// Top.
			1.45f, 1.0f, -3.3f,		// 7. 16
			1.45f, 1.0f, 2.0f,		// 3. 17
			1.5f, 1.0f, 2.0f,		// 2. 18
			1.5f, 1.0f, -3.3f,		// 6. 19
			// Bottom.
			1.5f, 0.0f, -3.3f,		// 4. 20
			1.5f, 0.0f, 2.0f,		// 5. 21
			1.45f, 0.0f, 2.0f,		// 1. 22
			1.45f, 0.0f, -3.3f		// 0. 23
		};
		shape_uvs = {
			// Front.
			0.0f, 0.0f, 	// 0.
			3.0f, 0.0f, 	// 1.
			3.0f, 1.0f, 	// 2.
			0.0f, 1.0f,		// 3.
			// Right.
			0.0f, 0.0f, 	// 1.
			1.0f, 0.0f, 	// 5.
			1.0f, 1.0f, 	// 6.
			0.0f, 1.0f,		// 2.
			// Back.
			0.0f, 0.0f, 	// 5.
			3.0f, 0.0f, 	// 4.
			3.0f, 1.0f,		// 7.
			0.0f, 1.0f,		// 6.
			// Left.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 0.
			1.0f, 1.0f,		// 3.
			0.0f, 1.0f,		// 7.
			// Top.
			0.0f, 0.0f,		// 7.
			1.0f, 0.0f,		// 3.
			1.0f, 1.0f,		// 2.
			0.0f, 1.0f,		// 6.
			// Bottom.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 5.
			1.0f, 1.0f,		// 1.
			0.0f, 1.0f		// 0.
		};
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};

struct LeftWall : public Shape
{
	LeftWall()
	{
		
		shape_indices = {
			// Front.
			0, 1, 2,
			2, 3, 0,
			// Right.
			4, 5, 6,
			6, 7, 4,
			// Back.
			8, 9, 10,
			10, 11, 8,
			// Left.
			12, 13, 14,
			14, 15, 12,
			// Top.
			16, 17, 18,
			18, 19, 16,
			// Bottom.
			20, 21, 22,
			22, 23, 20
		};
		shape_vertices = {
			// Front.
			-0.5f, 0.0f, 2.0f,		// 0.
			-0.45f, 0.0f, 2.0f,		// 1.
			-0.45f, 1.0f, 2.0f,		// 2.
			-0.5f, 1.0f, 2.0f,		// 3.
			// Right.
			-0.45f, 0.0f, 2.0f,		// 1. 4
			-0.45f, 0.0f, -3.3f,		// 5. 5
			-0.45f, 1.0f, -3.3f,		// 6. 6
			-0.45f, 1.0f, 2.0f,		// 2. 7
			// Back.
			-0.5f, 0.0f, -3.3f,		// 5. 8
			-0.5f, 0.0f, -3.3f,		// 4. 9
			-0.45f, 1.0f, -3.3f,		// 7. 10
			-0.45f, 1.0f, -3.3f,		// 6. 11
			// Left.
			-0.5f, 0.0f, -3.3f,		// 4. 12
			-0.5f, 0.0f, 2.0f,		// 0. 13
			-0.5f, 1.0f, 2.0f,		// 3. 14
			-0.5f, 1.0f, -3.3f,		// 7. 15
			// Top.
			-0.5f, 1.0f, -3.3f,		// 7. 16
			-0.5f, 1.0f, 2.0f,		// 3. 17
			-0.45f, 1.0f, 2.0f,		// 2. 18
			-0.45f, 1.0f, -3.3f,		// 6. 19
			// Bottom.
			-0.5f, 0.0f, -3.3f,		// 4. 20
			-0.5f, 0.0f, 2.0f,		// 5. 21
			-0.45f, 0.0f, 2.0f,		// 1. 22
			-0.45f, 0.0f, -3.3f		// 0. 23
		};
		shape_uvs = {
			// Front.
			0.0f, 0.0f, 	// 0.
			3.0f, 0.0f, 	// 1.
			3.0f, 1.0f, 	// 2.
			0.0f, 1.0f,		// 3.
			// Right.
			0.0f, 0.0f, 	// 1.
			1.0f, 0.0f, 	// 5.
			1.0f, 1.0f, 	// 6.
			0.0f, 1.0f,		// 2.
			// Back.
			0.0f, 0.0f, 	// 5.
			3.0f, 0.0f, 	// 4.
			3.0f, 1.0f,		// 7.
			0.0f, 1.0f,		// 6.
			// Left.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 0.
			1.0f, 1.0f,		// 3.
			0.0f, 1.0f,		// 7.
			// Top.
			0.0f, 0.0f,		// 7.
			1.0f, 0.0f,		// 3.
			1.0f, 1.0f,		// 2.
			0.0f, 1.0f,		// 6.
			// Bottom.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 5.
			1.0f, 1.0f,		// 1.
			0.0f, 1.0f		// 0.
		};
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};

struct BackWall : public Shape
{
	BackWall()
	{
		
		shape_indices = {
			// Front.
			0, 1, 2,
			2, 3, 0,
			// Right.
			4, 5, 6,
			6, 7, 4,
			// Back.
			8, 9, 10,
			10, 11, 8,
			// Left.
			12, 13, 14,
			14, 15, 12,
			// Top.
			16, 17, 18,
			18, 19, 16,
			// Bottom.
			20, 21, 22,
			22, 23, 20
		};
		shape_vertices = {
			// Front.
			-0.5f, 0.0f, -3.25f,		// 0.
			1.5f, 0.0f, -3.25f,		// 1.
			1.5f, 1.0f, -3.25f,		// 2.
			-0.5f, 1.0f, -3.25f,		// 3.
			// Right.
			1.5f, 0.0f, -3.3f,		// 4. 12	Z was 0
			1.5f, 0.0f, -3.25f,		// 0. 13
			1.5f, 1.0f, -3.25f,		// 3. 14
			1.5f, 1.0f, -3.3f,
			// Back.
			1.5f, 0.0f, -3.3f,		// 5. 8
			-0.5f, 0.0f, -3.3f,		// 4. 9
			-0.5f, 1.0f, -3.3f,		// 7. 10
			1.5f, 1.0f, -3.3f,		// 6. 11
			// Left.
			-0.5f, 0.0f, -3.25f,		// 1. 4		
			-0.5f, 0.0f, -3.3f,		// 5. 5		Z was 0
			-0.5f, 1.0f, -3.3f,		// 6. 6		Z was 0
			-0.5f, 1.0f, -3.25f,		// 2. 7		
			// Top.
			-0.5f, 1.0f, -3.3f,		// 7. 16  Z was 0
			-0.5f, 1.0f, -3.25f,		// 3. 17  
			1.5f, 1.0f, -3.25f,		// 2. 18
			1.5f, 1.0f, -3.3f,		// 6. 19	Z was 0
			// Bottom.
			1.5f, 0.0f, -3.3f,		// 4. 20	Z was 0
			-0.5f, 0.0f, -3.3f,		// 5. 21	Z was 0
			-0.5f, 0.0f, -3.25f,		// 1. 22
			1.5f, 0.0f, -3.25f		// 0. 23
		};
		shape_uvs = {
			// Front.
			0.0f, 0.0f, 	// 0.
			3.0f, 0.0f, 	// 1.
			3.0f, 1.0f, 	// 2.
			0.0f, 1.0f,		// 3.
			// Right.
			0.0f, 0.0f, 	// 1.
			1.0f, 0.0f, 	// 5.
			1.0f, 1.0f, 	// 6.
			0.0f, 1.0f,		// 2.
			// Back.
			0.0f, 0.0f, 	// 5.
			3.0f, 0.0f, 	// 4.
			3.0f, 1.0f,		// 7.
			0.0f, 1.0f,		// 6.
			// Left.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 0.
			1.0f, 1.0f,		// 3.
			0.0f, 1.0f,		// 7.
			// Top.
			0.0f, 0.0f,		// 7.
			1.0f, 0.0f,		// 3.
			1.0f, 1.0f,		// 2.
			0.0f, 1.0f,		// 6.
			// Bottom.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 5.
			1.0f, 1.0f,		// 1.
			0.0f, 1.0f		// 0.
		};
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};

struct FrontWallL : public Shape
{
	FrontWallL()
	{

		shape_indices = {
			// Front.
			0, 1, 2,
			2, 3, 0,
			// Right.
			4, 5, 6,
			6, 7, 4,
			// Back.
			8, 9, 10,
			10, 11, 8,
			// Left.
			12, 13, 14,
			14, 15, 12,
			// Top.
			16, 17, 18,
			18, 19, 16,
			// Bottom.
			20, 21, 22,
			22, 23, 20
		};
		shape_vertices = {
			// Front.
			-0.5f, 0.0f, 2.0f,		// 0.
			0.4f, 0.0f, 2.0f,		// 1.
			0.4f, 1.0f, 2.0f,		// 2.
			-0.5f, 1.0f, 2.0f,		// 3.
			// Right.
			0.4f, 0.0f, 2.0f,		// 4. 12	Z was 0
			0.4f, 0.0f, 1.95f,		// 0. 13
			0.4f, 1.0f, 1.95f,		// 3. 14
			0.4f, 1.0f, 2.0f,
			// Back.
			0.4f, 0.0f, 1.95f,		// 5. 8
			-0.5f, 0.0f, 1.95f,		// 4. 9
			-0.5f, 1.0f, 1.95f,		// 7. 10
			0.4f, 1.0f, 1.95f,		// 6. 11
			// Left.
			-0.5f, 0.0f, 1.95f,		// 1. 4		
			-0.5f, 0.0f, 2.0f,		// 5. 5		Z was 0
			-0.5f, 1.0f, 2.0f,		// 6. 6		Z was 0
			-0.5f, 1.0f, 1.95f,		// 2. 7		
			// Top.
			-0.5f, 1.0f, 1.95f,		// 7. 16  Z was 0
			-0.5f, 1.0f, 2.0f,		// 3. 17  
			0.4f, 1.0f, 2.0f,		// 2. 18
			0.4f, 1.0f, 1.95f,		// 6. 19	Z was 0
			// Bottom.
			0.4f, 0.0f, 1.95f,		// 4. 20	Z was 0
			-0.5f, 0.0f, 2.0f,		// 5. 21	Z was 0
			-0.5f, 0.0f, 2.0f,		// 1. 22
			0.4f, 0.0f, 1.95f		// 0. 23
		};
		shape_uvs = {
			// Front.
			0.0f, 0.0f, 	// 0.
			3.0f, 0.0f, 	// 1.
			3.0f, 1.0f, 	// 2.
			0.0f, 1.0f,		// 3.
			// Right.
			0.0f, 0.0f, 	// 1.
			1.0f, 0.0f, 	// 5.
			1.0f, 1.0f, 	// 6.
			0.0f, 1.0f,		// 2.
			// Back.
			0.0f, 0.0f, 	// 5.
			3.0f, 0.0f, 	// 4.
			3.0f, 1.0f,		// 7.
			0.0f, 1.0f,		// 6.
			// Left.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 0.
			1.0f, 1.0f,		// 3.
			0.0f, 1.0f,		// 7.
			// Top.
			0.0f, 0.0f,		// 7.
			1.0f, 0.0f,		// 3.
			1.0f, 1.0f,		// 2.
			0.0f, 1.0f,		// 6.
			// Bottom.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 5.
			1.0f, 1.0f,		// 1.
			0.0f, 1.0f		// 0.
		};
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};

struct FrontWallR : public Shape
{
	FrontWallR()
	{

		shape_indices = {
			// Front.
			0, 1, 2,
			2, 3, 0,
			// Right.
			4, 5, 6,
			6, 7, 4,
			// Back.
			8, 9, 10,
			10, 11, 8,
			// Left.
			12, 13, 14,
			14, 15, 12,
			// Top.
			16, 17, 18,
			18, 19, 16,
			// Bottom.
			20, 21, 22,
			22, 23, 20
		};
		shape_vertices = {
			// Front.
			0.6f, 0.0f, 2.0f,		// 0.
			1.5f, 0.0f, 2.0f,		// 1.
			1.5f, 1.0f, 2.0f,		// 2.
			0.6f, 1.0f, 2.0f,		// 3.
			// Right.
			1.5f, 0.0f, 2.0f,		// 4. 12	Z was 0
			1.5f, 0.0f, 1.95f,		// 0. 13
			1.5f, 1.0f, 1.95f,		// 3. 14
			1.5f, 1.0f, 2.0f,
			// Back.
			1.5f, 0.0f, 1.95f,		// 5. 8
			0.6f, 0.0f, 1.95f,		// 4. 9
			0.6f, 1.0f, 1.95f,		// 7. 10
			1.5f, 1.0f, 1.95f,		// 6. 11
			// Left.
			0.6f, 0.0f, 1.95f,		// 1. 4		
			0.6f, 0.0f, 2.0f,		// 5. 5		Z was 0
			0.6f, 1.0f, 2.0f,		// 6. 6		Z was 0
			0.6f, 1.0f, 1.95f,		// 2. 7		
			// Top.
			0.6f, 1.0f, 1.95f,		// 7. 16  Z was 0
			0.6f, 1.0f, 2.0f,		// 3. 17  
			1.5f, 1.0f, 2.0f,		// 2. 18
			1.5f, 1.0f, 1.95f,		// 6. 19	Z was 0
			// Bottom.
			1.5f, 0.0f, -3.3f,		// 4. 20	Z was 0
			1.5f, 0.0f, -3.3f,		// 5. 21	Z was 0
			0.6f, 0.0f, -3.25f,		// 1. 22
			0.6f, 0.0f, -3.25f		// 0. 23
		};
		shape_uvs = {
			// Front.
			0.0f, 0.0f, 	// 0.
			3.0f, 0.0f, 	// 1.
			3.0f, 1.0f, 	// 2.
			0.0f, 1.0f,		// 3.
			// Right.
			0.0f, 0.0f, 	// 1.
			1.0f, 0.0f, 	// 5.
			1.0f, 1.0f, 	// 6.
			0.0f, 1.0f,		// 2.
			// Back.
			0.0f, 0.0f, 	// 5.
			3.0f, 0.0f, 	// 4.
			3.0f, 1.0f,		// 7.
			0.0f, 1.0f,		// 6.
			// Left.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 0.
			1.0f, 1.0f,		// 3.
			0.0f, 1.0f,		// 7.
			// Top.
			0.0f, 0.0f,		// 7.
			1.0f, 0.0f,		// 3.
			1.0f, 1.0f,		// 2.
			0.0f, 1.0f,		// 6.
			// Bottom.
			0.0f, 0.0f,		// 4.
			1.0f, 0.0f,		// 5.
			1.0f, 1.0f,		// 1.
			0.0f, 1.0f		// 0.
		};
		ColorShape(1.0f, 1.0f, 1.0f);
	}
};
