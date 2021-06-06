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

std::vector<GLApp::GLModel> GLApp::models;
std::vector<GLApp::GLViewport> GLApp::vps;

// tutorial 3 shader program singleton
std::vector<GLSLShader> GLApp::shdrpgms;
std::list<GLApp::GLObject> GLApp::objects;

GLboolean	keyPlast			= false;	/*!< stores if p was pressed/released last frame */
GLboolean	mouseLeftlast		= false;	/*!< stores if moue left was pressed/released last frame */
bool		create_objects		= true;		/*!< flag to create/kill objects */
int			polymode			= 0;		/*!< polygon render mode */
int			box_count			= 0;		/*!< number of boxes */
int			mystery_count		= 0;		/*!< number of mystery shapes */

void GLApp::init() {
  // empty for now
	// Part 1: clear colorbuffer with the RGBA value in glClearColor
	glClearColor(1.f, 1.f, 1.f, 1.f);

	// Part 2: use entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);

	// part 3:
	GLApp::VPSS shdr_file_names{
		std::make_pair<std::string, std::string>
		("../shaders/tutorial-3.vert", "../shaders/tutorial-3.frag")
	};
	GLApp::init_shdrpgms_cont(shdr_file_names);

	// part 4:
	GLApp::init_models_cont();
	
	// part 5: print GPU specs ...
	GLHelper::print_specs();

	// set line width and point size
	glLineWidth(2.f);
	glPointSize(4.f);
}

void GLApp::update(double delta_time) {
	// part 1: if p pressed, update polygon rasterization mode
	if (GLHelper::keystateP && GLHelper::keystateP != keyPlast)
	{
		polymode = ++polymode > 2 ? 0 : polymode;
		switch (polymode)
		{
		case 0:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		case 1:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case 2:
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;
		}
	}

	// part 2: if mouse pressed, spawn/kill objects
	if (GLHelper::mouseLeft && GLHelper::mouseLeft != mouseLeftlast)
	{
		// create first object
		if (objects.size() <= 0)
		{
			CreateObject();
		}
		else
		{
			int iterations{ 0 };
			// if 1 create more objects at random
			if (objects.size() <= 1)
			{
				create_objects = true;
			}
			// else remove half
			else if (objects.size() >= 32768)
			{
				create_objects = false;
			}
			if (create_objects)
			{
				iterations = objects.size();
				// create new objects at back of object list
				for (int i = 0; i < iterations; ++i)
				{
					CreateObject();
				}
			}
			else
			{
				iterations = objects.size() / 2;
				// remove objects at front of object list
				for (int i = 0; i < iterations; ++i)
				{
					if (objects.front().mdl_ref != 1)
					{
						--box_count;
					}
					else
					{
						--mystery_count;
					}
					objects.pop_front();
				}
			}
		}
	}

	// part 3: for each objects, update orientation
	for (auto& o : GLApp::objects)
	{
		o.update(delta_time);
	}

	keyPlast = GLHelper::keystateP;
	mouseLeftlast = GLHelper::mouseLeft;
}

void GLApp::draw() {
	// title bar
	std::stringstream ss;
	ss	<< GLHelper::title	<< " | "
		<< objects.size()	<< " objects | "
		<< box_count		<< " box | "
		<< mystery_count	<< " mystery stuff | "
		<< "fps: " << GLHelper::fps	<< " | ";
	glfwSetWindowTitle(GLHelper::ptr_window, ss.str().c_str());

	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);

	// render all objects
	for (auto& o : GLApp::objects)
	{
		o.draw();
	}
}

void GLApp::cleanup() {
  // empty for now
}

GLApp::GLModel GLApp::points_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr)
{
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
	//mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
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
	//mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
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
	//mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
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
	//mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = idx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 2; // number of primitives (not used)
	return mdl;
}

void GLApp::init_models_cont()
{
	GLApp::models.push_back(GLApp::box_model());
	GLApp::models.push_back(GLApp::mystery_model());
}

GLApp::GLModel GLApp::box_model()
{
	// define box vertices
	std::vector<glm::vec2> vertices{ {-0.5, 0.5}, {-0.5, -0.5}, {0.5, 0.5}, {0.5, -0.5} };

	// create color
	std::vector<glm::vec3> colors;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		colors.push_back({ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX });
	}

	// define indices
	std::vector<GLushort> indices{ 0, 1, 2, 3 };

	// define vbo handle
	GLuint vbo_handle;
	glCreateBuffers(1, &vbo_handle);
	glNamedBufferStorage(vbo_handle, 
		sizeof(glm::vec2) * vertices.size() + sizeof(glm::vec3) * colors.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

	glNamedBufferSubData(vbo_handle, 0,
		sizeof(glm::vec2) * vertices.size(), vertices.data());

	glNamedBufferSubData(vbo_handle, sizeof(glm::vec2) * vertices.size(),
		sizeof(glm::vec3) * colors.size(), colors.data());

	// define vao handle
	GLuint vao_handle;
	glCreateVertexArrays(1, &vao_handle);

	// for position - attrib index 0, buffer index 5
	glEnableVertexArrayAttrib(vao_handle, 0);
	glVertexArrayVertexBuffer(vao_handle, 5, vbo_handle, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao_handle, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_handle, 0, 5);

	// for color - attrib index 1, buffer index 6
	glEnableVertexArrayAttrib(vao_handle, 1);
	glVertexArrayVertexBuffer(vao_handle, 6, vbo_handle, sizeof(glm::vec2) * vertices.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vao_handle, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_handle, 1, 6);

	// define ebo_handle
	GLuint ebo_handle;
	glCreateBuffers(1, &ebo_handle);
	glNamedBufferStorage(ebo_handle, sizeof(GLushort) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vao_handle, ebo_handle);

	glBindVertexArray(0);

	GLApp::GLModel model;
	model.vaoid = vao_handle;
	model.primitive_type = GL_TRIANGLE_STRIP;
	model.draw_cnt = vertices.size();	
	model.primitive_cnt = 0;			// not used
	return model;
}

GLApp::GLModel GLApp::mystery_model()
{
	// define box vertices
	std::vector<glm::vec2> vertices{ {-((float)rand() / RAND_MAX + 0.5) * 0.5, ((float)rand() / RAND_MAX + 0.5) * 0.5},
									{-((float)rand() / RAND_MAX + 0.5) * 0.5, -((float)rand() / RAND_MAX + 0.5) * 0.5},
									{((float)rand() / RAND_MAX + 0.5) * 0.5, ((float)rand() / RAND_MAX + 0.5) * 0.5},
									{((float)rand() / RAND_MAX + 0.5) * 0.5, -((float)rand() / RAND_MAX + 0.5) * 0.5} };

	// create color
	std::vector<glm::vec3> colors;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		colors.push_back({ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX });
	}

	// define indices
	std::vector<GLushort> indices{ 0, 1, 2, 3 };

	// define vbo handle
	GLuint vbo_handle;
	glCreateBuffers(1, &vbo_handle);
	glNamedBufferStorage(vbo_handle,
		sizeof(glm::vec2) * vertices.size() + sizeof(glm::vec3) * colors.size(), nullptr, GL_DYNAMIC_STORAGE_BIT);

	glNamedBufferSubData(vbo_handle, 0,
		sizeof(glm::vec2) * vertices.size(), vertices.data());

	glNamedBufferSubData(vbo_handle, sizeof(glm::vec2) * vertices.size(),
		sizeof(glm::vec3) * colors.size(), colors.data());

	// define vao handle
	GLuint vao_handle;
	glCreateVertexArrays(1, &vao_handle);

	// for position - attrib index 0, buffer index 5
	glEnableVertexArrayAttrib(vao_handle, 0);
	glVertexArrayVertexBuffer(vao_handle, 5, vbo_handle, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vao_handle, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_handle, 0, 5);

	// for color - attrib index 1, buffer index 6
	glEnableVertexArrayAttrib(vao_handle, 1);
	glVertexArrayVertexBuffer(vao_handle, 6, vbo_handle, sizeof(glm::vec2) * vertices.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vao_handle, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_handle, 1, 6);

	// define ebo_handle
	GLuint ebo_handle;
	glCreateBuffers(1, &ebo_handle);
	glNamedBufferStorage(ebo_handle, sizeof(GLushort) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
	glVertexArrayElementBuffer(vao_handle, ebo_handle);

	glBindVertexArray(0);

	GLApp::GLModel model;
	model.vaoid = vao_handle;
	model.primitive_type = GL_TRIANGLE_STRIP;
	model.draw_cnt = vertices.size();
	model.primitive_cnt = 0;			// not used
	return model;
}

void GLApp::init_shdrpgms_cont(GLApp::VPSS const& vpss)
{
	for (auto const& x : vpss)
	{
		std::vector<std::pair<GLenum, std::string>> shdr_files;
		shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, x.first));
		shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, x.second));

		GLSLShader shdr_pgm;
		shdr_pgm.CompileLinkValidate(shdr_files);
		// insert shader program into container
		GLApp::shdrpgms.push_back(shdr_pgm);
	}
}

void GLApp::CreateObject()
{
	// initialize random engine
	std::default_random_engine dre;
	dre.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

	// get numbers in range [-1,-1] 
	std::uniform_real_distribution<float> urdf(-1.0, 1.0);

	// set random position on screen
	double const WorldRange{ 5000.0 };
	GLApp::GLObject go;
	go.position = glm::vec2(urdf(dre) * WorldRange,
		urdf(dre) * WorldRange);

	// give random scale between 50 and 400, i.e. 400-50=350
	float temp_x = ((urdf(dre) + 1.0f) / 2.0f) * 350.0f + 40.0f;
	float temp_y = ((urdf(dre) + 1.0f) / 2.0f) * 350.0f + 40.0f;
	go.scaling = glm::vec2(temp_x,
		temp_y);

	// random angle and angular speed
	go.angle_disp = urdf(dre) * 6.28f;
	go.angle_speed = urdf(dre) * 1.04f;

	// select model
	if (urdf(dre) < 0.0f)
	{
		go.mdl_ref = 0;
		++box_count;
	}
	else
	{
		go.mdl_ref = 1;
		++mystery_count;
	}

	// select shader
	go.shd_ref = 0;

	// push back into objects list
	objects.push_back(go);
}

void GLApp::GLObject::init()
{
}

void GLApp::GLObject::draw() const
{
	// part 1: use shader program
	shdrpgms[shd_ref].Use();

	// part 2: bind vao handle using glBindVertexArray
	glBindVertexArray(models[mdl_ref].vaoid);

	// part 3: copy objects 3x3 model to ndc matrix to vertex shader
	shdrpgms[shd_ref].SetUniform("uModel_to_NDC", mdl_to_ndc_xform);

	// part 4: render using glDrawElements or glDrawArrays
	glDrawElements(models[mdl_ref].primitive_type, models[mdl_ref].draw_cnt, GL_UNSIGNED_SHORT, nullptr);
}

void GLApp::GLObject::update(GLdouble delta_time)
{
	// scaling matrix
	glm::mat3 m_scale {	scaling.x,	0,			0,
						0,			scaling.y,	0,
						0,			0,			1 };

	// rotation matrix
	angle_disp += angle_speed * (GLfloat)delta_time;
	glm::mat3 m_rotation {	 cos(angle_disp),  sin(angle_disp),	0,
							-sin(angle_disp),  cos(angle_disp),	0,
								0,					0,			1 };

	// translation matrix
	glm::mat3 m_translation	{	1,	0,	0,
								0,	1,	0,
								position.x,	position.y,		1 };

	// extents matrix
	glm::mat3 m_extents {	1.0f / 5000.0f,	0,				0,
							0,				1.0f / 5000.0f,	0,
							0,				0,				1 };

	// concatenate matrices for final transformation matrix
	mdl_to_ndc_xform = m_extents * m_translation * m_rotation * m_scale;
}
