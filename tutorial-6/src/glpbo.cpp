/* !
@file    glpbo.cpp
@author  z.tay@digipen.edu
@date    04/07/2021

This file contains the definition of static member functions declared in
glpbo.h for tutorial-6.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "glpbo.h"
#include "glhelper.h"
#include "glslshader.h"
#include <iostream>
#include <sstream>

/*                                                         static member data
----------------------------------------------------------------------------- */
GLsizei GLPbo::pixel_cnt, GLPbo::byte_cnt, GLPbo::height, GLPbo::width;
GLPbo::Color* GLPbo::ptr_to_pbo{nullptr};
GLPbo::Color GLPbo::clear_clr;
GLuint GLPbo::pboid, GLPbo::vaoid, GLPbo::elem_cnt, GLPbo::texid;
GLSLShader GLPbo::shdr_pgm;

/*                                                    static member functions
----------------------------------------------------------------------------- */
/*!
 * @brief part 4
 * I assume this is GLPbo::render()
 * Recalculates and updates the color in the pbo every frame.
*/
void GLPbo::emulate()
{
	// part 4.1, and 4.2
	// fluctuate colors based on time and set it
	// set the color using set_clear_color()
	double fluctuate_cos = cos(glfwGetTime());
	double fluctuate_sin = sin(glfwGetTime());
	fluctuate_cos = fluctuate_cos < 0 ? -fluctuate_cos : fluctuate_cos;
	fluctuate_sin = fluctuate_sin < 0 ? -fluctuate_sin : fluctuate_sin;
	set_clear_color(static_cast<int>(fluctuate_cos * 255.0), static_cast<int>(fluctuate_cos * 100.0), static_cast<int>(fluctuate_sin * 255.0));
	// map pbo
	ptr_to_pbo = reinterpret_cast<Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));	/*!< here use GL_WRITE_ONLY */

	// part 4.3, clear color buffer
	clear_color_buffer();

	// part 4.4, unmap pbo
	glUnmapNamedBuffer(pboid);

	// part 4.5, DMA (direct memory access)
	glTextureSubImage2D(texid, 0, 0, 0, width, height,
		GL_RGBA, GL_UNSIGNED_BYTE, ptr_to_pbo);
}

/*!
 * @brief GLPbo::draw_fullwindow_quad()
 * Draws the opengl rect created in setup_quad_vao()
*/
void GLPbo::draw_fullwindow_quad()
{
	// bind texture
	glBindTextureUnit(6, texid);
	// use shader
	shdr_pgm.Use();
	// set texture location
	GLuint tex_loc = glGetUniformLocation(shdr_pgm.GetHandle(), "uTex2d");
	glUniform1i(tex_loc, 6);
	// bind vaoid
	glBindVertexArray(vaoid);
	// draw
	glDrawElements(GL_TRIANGLE_STRIP, elem_cnt, GL_UNSIGNED_SHORT, nullptr);
	// unuse shader
	shdr_pgm.UnUse();
}

/*!
	@brief part 3
	Initializes GLPbo.
*/
void GLPbo::init(GLsizei w, GLsizei h)
{
	// part 3.1, set static data members
	width = w;
	height = h;
	pixel_cnt = width * height;
	byte_cnt = pixel_cnt * sizeof(Color);

	// part 3.2, set GLPbo::clear_clr
	set_clear_color(0, 255, 255);

	// part 3.3, create pbo and set pbo id by mapping it
	glCreateBuffers(1, &pboid);
	glNamedBufferStorage(pboid, byte_cnt, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	// part 3.4, point to PBO address - start of fence (glMapNamedBuffer)
	ptr_to_pbo = reinterpret_cast<Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));	/*!< here use GL_WRITE_ONLY */

	// part 3.5, clear color buffer with color GLPbo::clear_clr
	clear_color_buffer();

	// part 3.6, unmap pbo pointer - end of fence (glUnmapNamedBuffer)
	glUnmapNamedBuffer(pboid);

	// part 3.8, create the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &texid);
	// allocate gpu storage for texture
	glTextureStorage2D(texid, 1, GL_RGBA8, width, height);

	// part 3.9, copy data from cpu to gpu buffer
	glTextureSubImage2D(texid, 0, 0, 0, width, height,
		GL_RGBA, GL_UNSIGNED_BYTE, ptr_to_pbo);

	// part 3.10, create the opengl quad
	setup_quad_vao();

	// part 3.11, create the vertex and fragment shader
	setup_shdrpgm();
}

/*!
 * @brief part 3.10, creates the opengl quad
*/
void GLPbo::setup_quad_vao()
{
	// vertex, texture coordinate, and index data
	std::vector<glm::vec2>	vertices = {
		{ -1.0f,  1.0f },
		{ -1.0f, -1.0f },
		{  1.0f,  1.0f },
		{  1.0f, -1.0f }
	};
	std::vector<glm::vec2>	texture_coords1 = {
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	};
	std::vector<GLushort>	indices = {
		0, 1, 2, 3
	};

	unsigned int vbo, vao, ebo;

	// create and bind vbo
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo,
		vertices.size() * sizeof(glm::vec2) +
		texture_coords1.size() * sizeof(glm::vec2),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo,
		0,
		vertices.size() * sizeof(glm::vec2), vertices.data());
	glNamedBufferSubData(vbo,
		vertices.size() * sizeof(glm::vec2),
		texture_coords1.size() * sizeof(glm::vec2), texture_coords1.data());

	// create and bind vao
	glCreateVertexArrays(1, &vao);
	// position
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayVertexBuffer(vao, 0, vbo,
		0,
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	// texture coordinates 1
	glEnableVertexArrayAttrib(vao, 1);
	glVertexArrayVertexBuffer(vao, 1, vbo,
		vertices.size() * sizeof(glm::vec2),
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 1, 1);

	// create and bind ebo
	// indices
	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vao, ebo);
	// unbind vertex array
	glBindVertexArray(0);
	// delete buffers
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	vaoid = vao;
	elem_cnt = indices.size();
}

/*!
	@brief part 3.11, create the vertex and fragment shader
*/
void GLPbo::setup_shdrpgm()
{
	const std::string vs =	"#version 450 core\n"
							"layout(location = 0) in vec2 aVertexPosition;\n"
							"layout(location = 1) in vec2 aTexCoord;\n"
							"layout(location = 0) out vec2 vTexCoord;\n"
							"void main()\n"
							"{\n"
							"gl_Position = vec4(aVertexPosition, 0.0, 1.0);\n"
							"vTexCoord = aTexCoord;\n"
							"}";

	const std::string fs =	"#version 450 core\n"
							"layout(location = 0) in vec2 vTexCoord;\n"
							"layout(location = 0) out vec4 fFragColor;\n"
							"uniform sampler2D uTex2d;\n"
							"void main()\n"
							"{\n"
							"fFragColor = texture(uTex2d,vTexCoord);\n"
							"}";
	if (!shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, vs)) 
	{
		std::cout << shdr_pgm.GetLog() << std::endl;
	}
	if (!shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, fs))
	{
		std::cout << shdr_pgm.GetLog() << std::endl;
	}

	if (!shdr_pgm.Link() || !shdr_pgm.Validate())
	{
		std::cout << "Shaders not linked or validated!" << std::endl;
	}
	shdr_pgm.PrintActiveAttribs();
	shdr_pgm.PrintActiveUniforms();
}

/*!
	@brief part 5,
	Free vbo, pbo and texture in the hardware
*/
void GLPbo::cleanup()
{
	// free vao, pbo, and texture
	glDeleteVertexArrays(1, &vaoid);
	glDeleteBuffers(1, &pboid);
	glDeleteTextures(1, &texid);
}


/*!
 * @brief part 1,
 * First overload of function set_clear_color. Emulates behavior of GL command
 * glClearColor().
 * @param color : 
 * value to set static data memberr GLPbo::clear_clr to.
*/
void GLPbo::set_clear_color(GLPbo::Color color)
{
	GLPbo::clear_clr = color;
}

/*!
 * @brief part 1,
 * Second overload of function set_clear_color. Emulates behavior of GL command
 * glClearColor().
 * @param r : red channel
 * @param g : green channel
 * @param b : blue channel
 * @param a : alpha channel
*/
void GLPbo::set_clear_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
	GLPbo::clear_clr = { r,g,b,a };
}

/*!
 * @brief part 2,
 * This function emulates GL command glClear(GL_COLOR_BUFFER_BIT) by using
 * GLPbo::ptr_to_pbo to fill the PBO (that GLPbo::ptr_to_pbo points to) with
 * the RGBA value in GLPbo::clear_clr.
 * 
 * GLPbo::ptr_to_pbo is of type Color* which is an array of Color objects.
 * Fill the entire array with GLPbo::clear_clr using std::fill/std::fill_n/
 * std::memcpy.
*/
void GLPbo::clear_color_buffer()
{
	GLsizei empty_pixels = GLPbo::pixel_cnt;
	GLsizei filled_pixels = 0;
	
	// start by filling 1 pixel and deducting from empty_pixels
	// the number of pixels filled
	std::fill_n(ptr_to_pbo, 1, GLPbo::clear_clr);
	filled_pixels += 1;
	empty_pixels = GLPbo::pixel_cnt - filled_pixels;
	// start using memcpy to duplicate fills
	while (empty_pixels > 0)
	{
		// if able to duplicate fill double the current filled pixels
		if (filled_pixels <= empty_pixels)
		{
			std::memcpy(GLPbo::ptr_to_pbo + filled_pixels, GLPbo::ptr_to_pbo, filled_pixels * sizeof(GLPbo::Color));
			filled_pixels *= 2;
			empty_pixels = GLPbo::pixel_cnt - filled_pixels;
		}
		else // copy the exact amount to fill remaining pixels and all should be filled
		{
			std::memcpy(GLPbo::ptr_to_pbo + filled_pixels, GLPbo::ptr_to_pbo, empty_pixels * sizeof(GLPbo::Color));
			empty_pixels = 0;
		}
	}
}