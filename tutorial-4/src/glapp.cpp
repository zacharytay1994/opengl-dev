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

GLApp::Camera2D GLApp::camera2d;

GLboolean	keyPlast			= false;	/*!< stores if p was pressed/released last frame */
GLboolean	mouseLeftlast		= false;	/*!< stores if moue left was pressed/released last frame */
GLboolean	keystateVlast		= false;
bool		cam_mdl_orientation = false;
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

	// part 3 - init_scene()
	GLApp::init_scene("../scenes/tutorial-4.scn");

	// part 4 - initialize camera
	GLApp::camera2d.init(GLHelper::ptr_window, &GLApp::objects.at("Camera"));
	
	// part 5: print GPU specs ...
	GLHelper::print_specs();

	// set line width and point size
	glLineWidth(2.f);
	glPointSize(4.f);
}

void GLApp::update(double delta_time) {
	// first, update camera
	GLApp::camera2d.update(GLHelper::ptr_window, delta_time);

	// for each objects, update orientation
	for (auto& o : GLApp::objects)
	{
		o.second.update(delta_time);
	}

	keyPlast = GLHelper::keystateP;
	mouseLeftlast = GLHelper::mouseLeft;
	keystateVlast = GLHelper::keystateV;
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
		if (o.first != "Camera")
		{
			o.second.draw();
		}
	}
	objects["Camera"].draw();
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::init_shdrpgms_cont(const std::string& name, GLApp::VPSS const& vpss)
{
	for (auto const& x : vpss)
	{
		std::vector<std::pair<GLenum, std::string>> shdr_files;
		shdr_files.push_back(std::make_pair(GL_VERTEX_SHADER, x.first));
		shdr_files.push_back(std::make_pair(GL_FRAGMENT_SHADER, x.second));

		GLSLShader shdr_pgm;
		shdr_pgm.CompileLinkValidate(shdr_files);
		// insert shader program into container
		GLApp::shdrpgms[name] = shdr_pgm;
	}
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

void GLApp::init_scene(std::string scene_filename)
{
	std::ifstream ifs{ scene_filename, std::ios::in };
	if (!ifs)
	{
		std::cout << "ERROR: Unable to open scene file: "
			<< scene_filename << "\n";
		exit(EXIT_FAILURE);
	}
	ifs.seekg(0, std::ios::beg);

	std::string line;
	getline(ifs, line);
	std::istringstream line_sstm{ line };
	int obj_cnt;
	line_sstm >> obj_cnt;	// read count of objects in scene
	while (obj_cnt--)
	{
		getline(ifs, line);	// 1st parameter: model's name
		std::istringstream line_modelname{ line };
		std::string model_name;
		line_modelname >> model_name;

		GLObject new_object;

		/* TODO: if model with name model_name is not present in std::map container
		called models, then add this model to the container */
		if (models.find(model_name) == models.end())
		{
			// not found - read model from file and create new vao
			GLModel new_model;
			std::ifstream ifs{ "../meshes/"+model_name+".msh" , std::ios::in };
			if (!ifs)
			{
				std::cout << "ERROR: Unable to open mesh file: "
					<< model_name << "\n";
				exit(EXIT_FAILURE);
			}
			ifs.seekg(0, std::ios::beg);
			std::vector<float> vertices;
			std::vector<GLushort> indices;
			std::string line;
			while (std::getline(ifs, line))
			{
				std::istringstream iss (line);
				char c; double vertex; int index;
				iss >> c;
				switch (c)
				{
				case 'n':
					break;
				case 'v':
					while (iss >> vertex)
					{
						vertices.push_back((float)vertex);
					}
					break;
				case 't':
					// reading indices
					while (iss >> index)
					{
						indices.push_back(index);
					}
					new_model.primitive_type = GL_TRIANGLES;
					break;
				case 'f':
					// reading indices
					while (iss >> index)
					{
						indices.push_back(index);
					}
					new_model.primitive_type = GL_TRIANGLE_FAN;
					break;
				}
			}
			/*for (auto& v : vertices)
			{
				std::cout << v << ",";
			}
			std::cout << "\n";
			for (auto& i : indices)
			{
				std::cout << i << ",";
			}
			std::cout << "\n";*/

			// create new mdl_ref
			GLuint vbo, vao, ebo;
			// vbo
			glCreateBuffers(1, &vbo);
			glNamedBufferStorage(vbo, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
			// vao
			glCreateVertexArrays(1, &vao);
			glEnableVertexArrayAttrib(vao, 0);
			glVertexArrayVertexBuffer(vao, 5, vbo, 0, 2 * sizeof(float));
			glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribBinding(vao, 0, 5);
			// ebo
			glCreateBuffers(1, &ebo);
			glNamedBufferStorage(ebo, indices.size() * sizeof(GLushort), indices.data(), GL_DYNAMIC_STORAGE_BIT);
			glVertexArrayElementBuffer(vao, ebo);
			// unbind vao
			glBindVertexArray(0);
			
			// create new model and add to container
			new_model.vaoid = vao;
			new_model.draw_cnt = indices.size();
			new_model.primitive_cnt = 0;	// not used
			models[model_name] = new_model;
			// give new_object model ref
			new_object.mdl_ref = models.find(model_name);
		}
		else
		{
			// found - use existing mdl_ref
			new_object.mdl_ref = models.find(model_name);
		}

		// get name of object
		std::getline(ifs, line);
		std::istringstream line_objectname{ line };
		std::string object_name;
		line_objectname >> object_name;

		/* TODO: if shader program listed in the scene file is not present in
		std::map container called shdrpgms, then add this shader to the container */
		std::getline(ifs, line);
		std::istringstream line_shdrname{ line };
		std::string shdrname, vertfile, fragfile;
		line_shdrname >> shdrname >> vertfile >> fragfile;
		if (shdrpgms.find(shdrname) == shdrpgms.end())
		{
			// shader not found - create shader
			GLApp::VPSS shdr_file_names{
			std::make_pair<std::string, std::string>
				(vertfile.c_str(), fragfile.c_str())
			};
			GLApp::init_shdrpgms_cont(shdrname, shdr_file_names);
			// reference shader
			new_object.shd_ref = shdrpgms.find(shdrname);
		}
		else
		{
			// shader found - reference shader
			new_object.shd_ref = shdrpgms.find(shdrname);
		}

		/* TODO: read remaining parameters of object from file:
			- object's name
			- RGB parameters for rendering object's model geometry
			- scaling factors to be applied on object's model
			- orientation factors: initial angularr orientation and angular speed
			- object's position in game world
		   
		   Set data member GLApp::GLObject::mdl_ref to iterator that points to
		   model instantiated by this object.
		   
		   Set data member GLApp::GLObject::shd_ref to iterator that points to
		   shader program used by this object.
		   
		   insert this object to std::map container objects. */

		// read color
		std::getline(ifs, line);
		std::istringstream line_color{ line };
		line_color >> new_object.color.x >> new_object.color.y >> new_object.color.z;

		// read scale
		std::getline(ifs, line);
		std::istringstream line_scale{ line };
		line_scale >> new_object.scaling.x >> new_object.scaling.y;

		// read orientation
		std::getline(ifs, line);
		std::istringstream line_orientation{ line };
		line_orientation >> new_object.orientation.x >> new_object.orientation.y;
		new_object.orientation.x *= 3.14f / 180.0f;
		new_object.orientation.y *= 3.14f / 180.0f;

		// read position
		std::getline(ifs, line);
		std::istringstream line_position{ line };
		line_position >> new_object.position.x >> new_object.position.y;

		// insert new_object into objects container
		objects[object_name] = new_object;
	}
}

void GLApp::GLObject::init()
{
}

void GLApp::GLObject::draw() const
{
	// part 1: use shader program
	shd_ref->second.Use();

	// part 2: bind vao handle using glBindVertexArray
	glBindVertexArray(mdl_ref->second.vaoid);

	// part 3: copy objects 3x3 model to ndc matrix to vertex shader
	shd_ref->second.SetUniform("uModel_to_NDC", mdl_to_ndc_xform);
	shd_ref->second.SetUniform("uColor", color);

	// part 4: render using glDrawElements or glDrawArrays
	glDrawElements(mdl_ref->second.primitive_type, mdl_ref->second.draw_cnt, GL_UNSIGNED_SHORT, nullptr);
}

void GLApp::GLObject::update(GLdouble delta_time)
{
	// scaling matrix
	glm::mat3 m_scale {	scaling.x,	0,			0,
						0,			scaling.y,	0,
						0,			0,			1 };

	// rotation matrix
	orientation.x += orientation.y * (GLfloat)delta_time;
	glm::mat3 m_rotation {	 cos(orientation.x),  sin(orientation.x),	0,
							-sin(orientation.x),  cos(orientation.x),	0,
									0,					0,				1 };

	// translation matrix
	glm::mat3 m_translation	{	1,	0,	0,
								0,	1,	0,
								position.x,	position.y,		1 };

	// extents matrix
	glm::mat3 m_extents {	1.0f / 20000.0f,	0,				0,
							0,				1.0f / 20000.0f,	0,
							0,				0,				1 };

	// concatenate matrices for final transformation matrix
	mdl_to_ndc_xform = m_translation * m_rotation * m_scale;
	mdl_to_ndc_xform = camera2d.world_to_ndc_xform * mdl_to_ndc_xform;
	//mdl_to_ndc_xform = m_extents * mdl_to_ndc_xform;
}

void GLApp::Camera2D::init(GLFWwindow* window, GLObject* ptr)
{
	// assign address of object of type GLApp::GLObject with
	// name "Camera" in std::map container GLApp::objects ...
	pgo = &objects["Camera"];

	// compute camera window's aspect ratio ...
	GLsizei fb_width, fb_height;
	glfwGetFramebufferSize(window, &fb_width, &fb_height);
	ar = static_cast<GLfloat>(fb_width) / fb_height;

	// compute camera's up and right vectors ...
	up =	{ -sin(0), cos(0) };
	right = { cos(0), sin(0) };

	// at startup the camera must be initialized to free camera
	view_xform = {	up.x,				right.x,			0,
					up.y,				right.y,			0,
					-pgo->position.x,	-pgo->position.y,	1 };

	// compute other matrices
	camwin_to_ndc_xform = { 2.0f/(ar*height),	0,				0,
							0,					2.0f/height,	0,
							0,					0,				1 };

	world_to_ndc_xform = camwin_to_ndc_xform * view_xform;
}

void GLApp::Camera2D::update(GLFWwindow* window, GLdouble delta_time)
{
	// check keyboard button presses to enable camera interactivity

	// update camera aspect ratio - this must be done every frame
	// because it is possible for the user to change viewport
	// dimensions
	GLsizei fb_width, fb_height;
	glfwGetFramebufferSize(window, &fb_width, &fb_height);
	ar = static_cast<GLfloat>(fb_width) / fb_height;

	// update camera's orientation (if required)
	if (GLHelper::keystateH)
	{
		pgo->orientation.x += turn_speed * (float)delta_time;
	}
	if (GLHelper::keystateK)
	{
		pgo->orientation.x -= turn_speed * (float)delta_time;
	}

	// update camera's up and right vectors (if required)
	if (GLHelper::keystateV && GLHelper::keystateV != keystateVlast)
	{
		cam_mdl_orientation = !cam_mdl_orientation;
		if (cam_mdl_orientation)
		{
			up = { -sin(pgo->orientation.x), cos(pgo->orientation.x) };
			right = { cos(pgo->orientation.x), sin(pgo->orientation.x) };
		}
		else
		{
			up =	{ -sin(0.0), cos(0.0) };
			right = { cos(0.0), sin(0.0) };
		}
	}

	// update camera's position (if required)
	if (GLHelper::keystateU)
	{
		pgo->position.x += (float)cos((double)pgo->orientation.x + 1.57) * linear_speed;
		pgo->position.y += (float)sin((double)pgo->orientation.x + 1.57) * linear_speed;
	}

	// implement camera's zoom effect (if required)
	if (GLHelper::keystateZ)
	{
		if (height_chg_dir == 1)
		{
			if (height > max_height)
			{
				height_chg_dir = -1;
			}
			height += height_chg_val;
		}
		else if (height_chg_dir = -1)
		{
			if (height < min_height)
			{
				height_chg_dir = 1;
			}
			height -= height_chg_val;
		}
	}

	// compute appropriate world-to-camera view transformation matrix
	view_xform = {	right.x,												up.x,											0,
					right.y,												up.y,											0,
					-(right.x*pgo->position.x+right.y*pgo->position.y),		-(up.x*pgo->position.x+up.y*pgo->position.y),	1 };
	
	// compute window-to-NDC transformation 
	camwin_to_ndc_xform = { 2.0f / (ar * height),	0,				0,
							0,						2.0f / height,	0,
							0,						0,				1 };

	// compute world-to-NDC transformation matrix
	world_to_ndc_xform = camwin_to_ndc_xform * view_xform;
}
