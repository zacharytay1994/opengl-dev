/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <glapp.h>
#include <glhelper.h>

#include <iostream>
#include <array>

/*                                                   objects with file scope
----------------------------------------------------------------------------- */

GLApp::GLModel GLApp::mdl = {};
void GLApp::init() {
  // empty for now
	// Part 1: clear colorbuffer with the RGBA value in glClearColor
	glClearColor(0.f, 1.f, 0.f, 1.f);

	// Part 2: use entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	
	// part 3:
	mdl.setup_vao();
	mdl.setup_shdrpgm();

	// task 2
	GLubyte const* str_ven = glGetString(GL_VENDOR);
	std::cout << "GPU Vendor : " << str_ven << std::endl;

	str_ven = glGetString(GL_RENDERER);
	std::cout << "GL Renderer : " << str_ven << std::endl;

	str_ven = glGetString(GL_VERSION);
	std::cout << "GL Version : " << str_ven << std::endl;

	str_ven = glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "GL Shader Version : " << str_ven << std::endl;

	int major = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	std::cout << "GL Major Version : " << major << std::endl;

	int minor = 0;
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	std::cout << "GL Minor Version : " << minor << std::endl;

	std::cout << "Current OpenGL Context is double buffered\n";

	GLint count = 0;
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &count);
	std::cout << "Maximum Vertex Count : " << count << std::endl;

	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &count);
	std::cout << "Maximum Indices Count : " << count << std::endl;

	GLint tex_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_size);
	std::cout << "GL Maximum texture size: " << tex_size << std::endl;

	int results[4];
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, results);
	std::cout << "Maximum Viewport Dimensions : " << results[0] << " x " << results[1] << std::endl;
}

double count = 0.0;
void GLApp::update(double delta_time) {
  // empty for now
	count += delta_time;
	glClearColor(0.5f + (float)sin(count), (float)sin(count), (float)cos(count + 1.57), 1.f);
}

void GLApp::draw() {
  // empty for now
	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);
	// now, render rectangular model from NDC coordinates to viewport
	mdl.draw();
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::GLModel::setup_vao()
{
	// Define vertex position and color attributes
	std::array<glm::vec2, 4> pos_vtx{
	glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
	glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};
	std::array<glm::vec3, 4> clr_vtx{
	glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 1.f)
	};
	// transfer vertex position and color attributes to VBO
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size() + sizeof(glm::vec3) * clr_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo_hdl, 0,
		sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());
	glNamedBufferSubData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());
	// encapsulate information about contents of VBO and VBO handle
	// to another object called VAO
	glCreateVertexArrays(1, &vaoid);
	// for vertex position array, we use vertex attribute index 8
	// and vertex buffer binding point 3
	glEnableVertexArrayAttrib(vaoid, 8);
	glVertexArrayVertexBuffer(vaoid, 3, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 8, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 8, 3);
	// for vertex color array, we use vertex attribute index 9
	// and vertex buffer binding point 4
	glEnableVertexArrayAttrib(vaoid, 9);
	glVertexArrayVertexBuffer(vaoid, 4, vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 9, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 9, 4);
	primitive_type = GL_TRIANGLES;
	std::array<GLushort, 6> idx_vtx{ 0, 1, 2, 2, 3, 0 };
	idx_elem_cnt = idx_vtx.size();
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort) * idx_elem_cnt,
		reinterpret_cast<GLvoid*>(idx_vtx.data()),
		GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vaoid, ebo_hdl);
	glBindVertexArray(0);
}

void GLApp::GLModel::setup_shdrpgm()
{
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.push_back(std::make_pair(
		GL_VERTEX_SHADER,
		"../shaders/tutorial-1.vert"));
	shdr_files.push_back(std::make_pair(
		GL_FRAGMENT_SHADER,
		"../shaders/tutorial-1.frag"));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs" << "\n";
		std::cout << shdr_pgm.GetLog() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void GLApp::GLModel::draw()
{
	// there are many shader programs initialized - here we're saying
	// which specific shader program should be used to render geometry
	shdr_pgm.Use();
	// there are many models, each with their own initialized VAO object
	// here, we're saying which VAO's state should be used to set up pipe
	glBindVertexArray(vaoid);
	// here, we're saying what primitive is to be rendered and how many
	// such primitives exist.
	// the graphics driver knows where to get the indices because the VAO
	// containing this state information has been made current ...
	glDrawElements(primitive_type, idx_elem_cnt, GL_UNSIGNED_SHORT, NULL);
	// after completing the rendering, we tell the driver that VAO
	// vaoid and current shader program are no longer current
	glBindVertexArray(0);
	shdr_pgm.UnUse();
}
