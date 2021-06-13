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

#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include <sstream>

/*                                                   objects with file scope
----------------------------------------------------------------------------- */

std::vector<GLApp::GLViewport> GLApp::vps;

// added in tutorial 4 define singleton containers
std::map<std::string, GLSLShader>		GLApp::shdrpgms;
std::map<std::string, GLApp::GLModel>	GLApp::models;
std::map<std::string, GLApp::GLObject>	GLApp::objects;

GLboolean	keyTlast			= false;
GLboolean	keyMlast			= false;
GLboolean	keyAlast			= false;

int		uMode	= 0;
bool	uBlend	= false;
bool	blend_alpha = false;
float	ease	= 0.0f;

void GLApp::init() {
  // empty for now
	// Part 1: clear colorbuffer with the RGBA value in glClearColor
	glClearColor(1.f, 1.f, 1.f, 1.f);

	// Part 2: use entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);

	// part 3 - initialize scene
	// initialize model data
	std::vector<glm::vec2>	vertices = {
		{ -1.0f,  1.0f },
		{ -1.0f, -1.0f },
		{  1.0f,  1.0f },
		{  1.0f, -1.0f }
	};
	std::vector<glm::vec3>	color = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.5f, 0.5f }
	};
	std::vector<glm::vec2>	texture_coords1 = {
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	};
	std::vector<glm::vec2>	texture_coords2 = {
		{ 0.0f, 5.0f },
		{ 0.0f, 0.0f },
		{ 5.0f, 5.0f },
		{ 5.0f, 0.0f }
	};
	std::vector<GLushort>	indices = {
		0, 1, 2, 3
	};
	unsigned int vbo, vao, ebo;
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo,
		vertices.size() * sizeof(glm::vec2) +
		color.size() * sizeof(glm::vec3) +
		texture_coords1.size() * sizeof(glm::vec2) +
		texture_coords2.size() * sizeof(glm::vec2),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo, 
		0, 
		vertices.size() * sizeof(glm::vec2), vertices.data());
	glNamedBufferSubData(vbo, 
		vertices.size() * sizeof(glm::vec2),
		color.size() * sizeof(glm::vec3), color.data());
	glNamedBufferSubData(vbo, 
		vertices.size() * sizeof(glm::vec2) + 
		color.size() * sizeof(glm::vec3),
		texture_coords1.size() * sizeof(glm::vec2), texture_coords1.data());
	glNamedBufferSubData(vbo,
		vertices.size() * sizeof(glm::vec2) + 
		color.size() * sizeof(glm::vec3) +
		texture_coords1.size() * sizeof(glm::vec2),
		texture_coords2.size() * sizeof(glm::vec2), texture_coords2.data());

	glCreateVertexArrays(1, &vao);
	// position
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayVertexBuffer(vao, 0, vbo, 
		0, 
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 0, 0);
	// color
	glEnableVertexArrayAttrib(vao, 1);
	glVertexArrayVertexBuffer(vao, 1, vbo, 
		vertices.size() * sizeof(glm::vec2),
		sizeof(glm::vec3));
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 1, 1);
	// texture coordinates 1
	glEnableVertexArrayAttrib(vao, 2);
	glVertexArrayVertexBuffer(vao, 2, vbo, 
		vertices.size() * sizeof(glm::vec2) + 
		color.size() * sizeof(glm::vec3),
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 2, 2);
	// texture coordinates 2
	glEnableVertexArrayAttrib(vao, 3);
	glVertexArrayVertexBuffer(vao, 3, vbo,
		vertices.size() * sizeof(glm::vec2) +
		color.size() * sizeof(glm::vec3) +
		texture_coords1.size() * sizeof(glm::vec2),
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao, 3, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, 3, 3);

	// indices
	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vao, ebo);

	GLModel new_model;
	new_model.vaoid = vao;
	new_model.primitive_type = GL_TRIANGLE_STRIP;
	new_model.draw_cnt = indices.size();
	new_model.primitive_cnt = new_model.draw_cnt / 2;
	models["box"] = new_model;

	GLObject new_object;
	new_object.mdl_ref = models.find("box");

	// load shader
	insert_shdrpgm("tutorial5", "../shaders/tutorial-5.vert", "../shaders/tutorial-5.frag");
	new_object.shd_ref = shdrpgms.find("tutorial5");

	new_object.texture = setup_texobj("../images/duck-rgba-256.tex");

	objects["box"] = new_object;
	
	// part 5: print GPU specs ...
	GLHelper::print_specs();

	// set line width and point size
	glLineWidth(2.f);
	glPointSize(4.f);
}

void GLApp::update(double delta_time) {

	// for each objects, update orientation
	for (auto& o : GLApp::objects)
	{
		o.second.update(delta_time);
	}

	if (GLHelper::keystateT && GLHelper::keystateT != keyTlast)
	{
		uMode = uMode + 1 > 6 ? 0 : uMode + 1;
		if (uMode == 3)
		{
			ease = 0.0f;
		}
	}

	if (GLHelper::keystateM && GLHelper::keystateM != keyMlast)
	{
		uBlend = !uBlend;
	}
	
	if (GLHelper::keystateA && GLHelper::keystateA != keyAlast)
	{
		blend_alpha = !blend_alpha;
	}

	ease += delta_time / 15.0f;

	keyTlast = GLHelper::keystateT;
	keyMlast = GLHelper::keystateM;
	keyAlast = GLHelper::keystateA;
}

void GLApp::draw() {
	// title bar
	std::stringstream ss;
	ss << std::fixed;
	ss.precision(2);
	ss << GLHelper::title << " | ";
		/*<< "FPS: " << GLHelper::fps << " | "
		<< "Camera position: (" << camera2d.pgo->position.x << "," << camera2d.pgo->position.y << ") | "
		<< "Camera orientation: " << (int)(camera2d.pgo->orientation.x * (180.0f / 3.14f)) << " degrees | "
		<< "Window height: " << camera2d.height;*/
	glfwSetWindowTitle(GLHelper::ptr_window, ss.str().c_str());

	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);

	// render all objects
	for (auto& o : GLApp::objects)
	{
		o.second.draw();
	}
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::insert_shdrpgm(std::string shdr_pgm_name, std::string vtx_shdr, std::string frg_shdr)
{
	std::vector<std::pair<GLenum, std::string>> shdr_files{
		std::make_pair(GL_VERTEX_SHADER, vtx_shdr),
		std::make_pair(GL_FRAGMENT_SHADER, frg_shdr)
	};
	GLSLShader shdr_pgm;
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked())
	{
		std::cout << "Unable to compile/link/validate shader programs\n";
		std::cout << shdr_pgm.GetLog() << "\n";
		std::exit(EXIT_FAILURE);
	}
	// add compiled, linked and validated shader program to
	// std::map container GLApp::shdrpgms
	GLApp::shdrpgms[shdr_pgm_name] = shdr_pgm;
}

GLuint GLApp::setup_texobj(std::string pathname)
{
	GLuint width{ 256 }, height{ 256 }, bytes_per_texel{ 4 };
	GLuint byte_size = width * height * bytes_per_texel;
	// todo
	char* ptr_texels = new char[byte_size];
	std::ifstream ifs{ pathname, std::ios::in | std::ios::binary };
	if (!ifs)
	{
		std::cout << "ERROR: Unable to open image file: "
			<< pathname << "\n";
		exit(EXIT_FAILURE);
	}
	/*	Puts file cursor to the start of the file. */
	ifs.seekg(0, std::ios::beg);
	int index = 0;
	char c;
	while (!ifs.eof())
	{
		ifs.read(ptr_texels, byte_size);
	}
	ifs.close();

	// gl texture object
	GLuint texobj_hdl;
	// create the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &texobj_hdl);
	// allocate gpu storage for texture
	glTextureStorage2D(texobj_hdl, 1, GL_RGBA8, width, height);
	// copy data from cpu to gpu buffer
	glTextureSubImage2D(texobj_hdl, 0, 0, 0, width, height,
		GL_RGBA, GL_UNSIGNED_BYTE, ptr_texels);

	delete[] ptr_texels;
	return texobj_hdl;
}

void GLApp::GLObject::init()
{
}

void GLApp::GLObject::draw() const
{
	// bind texture
	glBindTextureUnit(6, texture);
	switch (uMode)
	{
		case 3:
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case 4:
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case 5:
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
		case 6:
			glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
	}

	if (blend_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	// part 1: use shader program
	shd_ref->second.Use();

	// tells fragment shader sampler2D uTex2d will use texture image unit 6
	GLuint tex_loc = glGetUniformLocation(shd_ref->second.GetHandle(), "uTex2d");
	glUniform1i(tex_loc, 6);

	// part 2: bind vao handle using glBindVertexArray
	glBindVertexArray(mdl_ref->second.vaoid);

	// part 3: set shader mode
	shd_ref->second.SetUniform("uMode", uMode);
	shd_ref->second.SetUniform("uBlend", uBlend);
	shd_ref->second.SetUniform("uTileSize", 16 + int(sin(ease) * 256));

	// part 4: render using glDrawElements or glDrawArrays
	glDrawElements(mdl_ref->second.primitive_type, mdl_ref->second.draw_cnt, GL_UNSIGNED_SHORT, nullptr);
}

void GLApp::GLObject::update(GLdouble delta_time)
{
}