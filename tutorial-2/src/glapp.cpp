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

std::vector<GLApp::GLModel> GLApp::models;
std::vector<GLApp::GLViewport> GLApp::vps;

void GLApp::init() {
  // empty for now
	// Part 1: clear colorbuffer with the RGBA value in glClearColor
	glClearColor(1.f, 1.f, 1.f, 1.f);

	// Part 2: use entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	
	// part 3:
	GLApp::models.push_back(
		GLApp::points_model(20, 20, "../shaders/tutorial-2.vert", "../shaders/tutorial-2.frag")
	);

	GLApp::models.push_back(
		GLApp::lines_model(40, 40, "../shaders/tutorial-2.vert", "../shaders/tutorial-2.frag")
	);

	GLApp::models.push_back(
		GLApp::trifans_model(50, "../shaders/tutorial-2.vert", "../shaders/tutorial-2.frag")
	);

	GLApp::models.push_back(
		GLApp::tristrip_model(50, 30, "../shaders/tutorial-2.vert", "../shaders/tutorial-2.frag")
	);

	// part 4: print GPU specs ...
	GLHelper::print_specs();
	

	// setup viewports
	GLint w{ GLHelper::width }, h{ GLHelper::height };
	// push top-left
	vps.push_back({ 0, h / 2, w / 2, h / 2 });
	// push top-right
	vps.push_back({ w / 2, h / 2, w / 2, h / 2 });
	// push bot-left
	vps.push_back({ 0, 0, w / 2, h / 2 });
	// push bot-right
	vps.push_back({ w / 2, 0, w / 2, h / 2 });
}

void GLApp::update(double delta_time) {
  // empty for now
}

void GLApp::draw() {
  // empty for now
	// title bar
	std::string stitle{ GLHelper::title };
	glfwSetWindowTitle(GLHelper::ptr_window, stitle.c_str());

	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);

	// now, render rectangular model from NDC coordinates to viewport
	// render to 4 viewports
	/*for (auto& vp : vps)
	{
		glViewport(vp.x, vp.y, vp.width, vp.height);
		GLApp::models[0].draw();
	}*/

	// render rectangular shape from NDC coordinates to viewport
	glViewport(vps[0].x, vps[0].y, vps[0].width, vps[0].height);
	GLApp::models[0].draw();

	// render lines in top-right 
	glViewport(vps[1].x, vps[1].y, vps[1].width, vps[1].height);
	GLApp::models[1].draw();

	// render fan in bottom-left 
	glViewport(vps[2].x, vps[2].y, vps[2].width, vps[2].height);
	GLApp::models[2].draw();

	// render strips in bottom-right 
	glViewport(vps[3].x, vps[3].y, vps[3].width, vps[3].height);
	GLApp::models[3].draw();
}

void GLApp::cleanup() {
  // empty for now
}

GLApp::GLModel GLApp::points_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr)
{
	// secify model corners
	/*std::vector<glm::vec2> pos_vtx{
		glm::vec2(0.25f, 0.25f), glm::vec2(-0.25f, 0.25f),
		glm::vec2(-0.25f, -0.25f), glm::vec2(0.25f, -0.25f)
	};*/

	std::vector<glm::vec2> pos_vtx;
	float stack_interval = 2.0f / stacks, slice_interval = 2.0f / slices;
	for (int i = 0; i <= stacks; ++i)
	{
		for (int j = 0; j <= slices; ++j)
		{
			pos_vtx.push_back({-1.0f + i * stack_interval, -1.0f + j * slice_interval });
		}
	}

	// define vao handle
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 0);
	glBindVertexArray(0);

	GLApp::GLModel mdl;
	mdl.vaoid = vaoid;
	mdl.primitive_type = GL_POINTS;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = pos_vtx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt; // number of primitives (unused)
	return mdl;
}	// end of function GLApp::points_model

GLApp::GLModel GLApp::lines_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr)
{
	// vertical
	int const count_slices{ (slices + 1) * 2 };
	int const count_stacks{ (stacks + 1) * 2 };
	std::vector<glm::vec2> pos_vtx(count_slices + count_stacks);
	int index{ 0 };
	float const u{ 2.f / static_cast<float>(slices) };
	for (int col{ 0 }; col <= slices; ++col)
	{
		float x{ u * static_cast<float>(col) - 1.f };
		pos_vtx[index++] = glm::vec2(x, -1.f);
		pos_vtx[index++] = glm::vec2(x, 1.f);
	}
	// horizontal
	float const w{ 2.f / static_cast<float>(stacks) };
	for (int row{ 0 }; row <= stacks; ++row)
	{
		float y{ w * static_cast<float>(row) - 1.f };
		pos_vtx[index++] = glm::vec2(-1.f, y);
		pos_vtx[index++] = glm::vec2(1.f, y);
	}

	// define vao handle
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 0);
	glBindVertexArray(0);

	// compute and store endpoints for (slices+1) set of lines
	// for each x from -1 to 1
	// start endpoint is (x, -1) and end endpoint is (x, 1)
	// set up VAO as in GLApp::points_model
	GLApp::GLModel mdl;
	mdl.vaoid = vaoid; // set up VAO same as in GLApp::points_model
	mdl.primitive_type = GL_LINES;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = 2 * (slices + 1) + 2 * (stacks + 1); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 2; // number of primitives (not used)
	return mdl;

}

GLApp::GLModel GLApp::trifans_model(int slices, std::string vtx_shdr, std::string frg_shdr)
{
	std::vector<glm::vec2> pos_vtx;
	pos_vtx.push_back({ 0.0f,0.0f });
	// step 1
	float theta{ 0.0f }, theta_interval { 2.0f*3.14159f / slices };
	float radius{ 1.0f };
	float x, y;
	for (int i = 0; i < slices; ++i)
	{
		x = radius * cos(theta);
		y = radius * sin(theta);
		pos_vtx.push_back({ x, y });
		theta += theta_interval;
	}

	// step 2 - create color
	std::vector<glm::vec3> clr_vtx;
	for (size_t i = 0; i < pos_vtx.size(); ++i)
	{
		clr_vtx.push_back({ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX });
	}
	
	// define vbo handle
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);

	glNamedBufferStorage(vbo_hdl, 
		sizeof(glm::vec2) * pos_vtx.size() + sizeof(glm::vec3) * clr_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);

	glNamedBufferSubData(vbo_hdl, 0,
		sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());

	glNamedBufferSubData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());

	// define vao handle
	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	// for position
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 4, vbo_hdl, 
		0,
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 4);
	// for color
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 5, vbo_hdl, 
		sizeof(glm::vec2)*pos_vtx.size(),
		sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 5);

	// create index data
	std::vector<GLushort> idx;
	for (int i = 0; i <= slices; ++i)
	{
		idx.push_back(i);
	}
	idx.push_back(1);

	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort)* idx.size(), idx.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	glBindVertexArray(0);

	GLApp::GLModel mdl;
	mdl.vaoid = vaoid; // set up VAO same as in GLApp::points_model
	mdl.primitive_type = GL_TRIANGLE_FAN;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = idx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 2; // number of primitives (not used)
	return mdl;
}

GLApp::GLModel GLApp::tristrip_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr)
{
	// generate vertices
	std::vector<glm::vec2> pos_vtx;
	float stack_interval = 2.0f / stacks, slice_interval = 2.0f / slices;
	for (int i = 0; i <= stacks; ++i)
	{
		for (int j = 0; j <= slices; ++j)
		{
			pos_vtx.push_back({ -1.0f + i * stack_interval, -1.0f + j * slice_interval });
		}
	}
	// generate indices
	std::vector<GLushort> idx;
	for (int row = 0; row < stacks; ++row)
	{
		if (row != 0)
		{
			idx.push_back((row + 1) * (slices + 1));
		}
		for (int col = 0; col <= slices; ++col)
		{
			idx.push_back((slices + 1) * (row+1) + col);
			idx.push_back((slices + 1) * row + col);
		}
		if (row != stacks - 1)
		{
			idx.push_back((row) * (slices + 1) + slices);
		}
	}

	// step 2 - create color
	std::vector<glm::vec3> clr_vtx;
	for (size_t i = 0; i < pos_vtx.size(); ++i)
	{
		clr_vtx.push_back({ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX });
	}

	// define vbo handle
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);

	glNamedBufferStorage(vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size() + sizeof(glm::vec3) * clr_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);

	glNamedBufferSubData(vbo_hdl, 0,
		sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());

	glNamedBufferSubData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());

	// define vao handle
	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	// for position
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 6, vbo_hdl,
		0,
		sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 6);
	// for color
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 7, vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 7);

	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort) * idx.size(), idx.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	glBindVertexArray(0);

	GLApp::GLModel mdl;
	mdl.vaoid = vaoid; // set up VAO same as in GLApp::points_model
	mdl.primitive_type = GL_TRIANGLE_STRIP;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = idx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 2; // number of primitives (not used)
	return mdl;
}

void GLApp::GLModel::setup_shdrpgm(std::string vtx_shdr, std::string frg_shdr)
{
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, vtx_shdr));
	shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, frg_shdr));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked())
	{
		std::cout << "Unable to compile/link/validate shader program\n";
		std::cout << shdr_pgm.GetLog() << "\n";
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
	/*glDrawElements(primitive_type, idx_elem_cnt, GL_UNSIGNED_SHORT, NULL);*/
	// draw points
	switch (primitive_type)
	{
	case GL_POINTS:
		glPointSize(10.f);
		glVertexAttrib3f(1, 1.f, 0.0f, 0.f);	// red color for points
		glDrawArrays(primitive_type, 0, draw_cnt);
		glPointSize(1.f);
		break;
	case GL_LINES:
		glLineWidth(3.f);
		glVertexAttrib3f(1, 0.f, 0.f, 1.f);		// blue color for lines
		glDrawArrays(primitive_type, 0, draw_cnt);
		glLineWidth(1.f);
		break;
	case GL_TRIANGLE_FAN:
		glDrawElements(primitive_type, draw_cnt, GL_UNSIGNED_SHORT, NULL);
		break;
	case GL_TRIANGLE_STRIP:
		glDrawElements(primitive_type, draw_cnt, GL_UNSIGNED_SHORT, NULL);
		break;
	}

	// after completing the rendering, we tell the driver that VAO
	// vaoid and current shader program are no longer current
	glBindVertexArray(0);
	shdr_pgm.UnUse();
}
