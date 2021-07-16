/* !
@file    glpbo.cpp
@author  z.tay@digipen.edu
@date    15/07/2021

This file contains the definition of static member functions declared in
glpbo.h for tutorial-6 and 7.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "glpbo.h"
#include "glhelper.h"
#include "glslshader.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>

#include <dpml.h>


/*                                                         static member data
----------------------------------------------------------------------------- */
GLsizei GLPbo::pixel_cnt, GLPbo::byte_cnt, GLPbo::height, GLPbo::width;
GLPbo::Color* GLPbo::ptr_to_pbo{nullptr};
GLPbo::Color GLPbo::clear_clr;
GLuint GLPbo::pboid, GLPbo::vaoid, GLPbo::elem_cnt, GLPbo::texid;
GLSLShader GLPbo::shdr_pgm;
GLPbo::Model GLPbo::cube;

/*!
 * @brief ___Scene___
 * *********************
 * Namespace that holds scene variables.
*/
namespace Scene
{
	GLboolean	keyRlast = false;
	GLboolean	keyWlast = false;
	GLboolean	keyMlast = false;

	enum class DrawMode
	{
		Wireframe,
		WireframeColor,
		FlatShaded,
		SmoothShaded
	};

	unsigned int				current_model = 0;
	std::vector<GLPbo::Model>	models;
	DrawMode					current_draw_mode = DrawMode::Wireframe;
	bool						rotate = false;
	double						orientation = 0;

	/*!
	 * @brief ___Scene::LoadScene___
	 * *******************************
	 * Loads a scene from a .scn file.
	 * **************
	 * @param scene 
	 * : Path of the .scn file.
	*/
	void LoadScene(const std::string& scene)
	{
		std::ifstream file(scene);
		if (file)
		{
			std::string line;
			while (file >> line)
			{
				models.push_back(GLPbo::Model());
				DPML::parse_obj_mesh("../meshes/" + line + ".obj",
					models.back().pm,
					models.back().nml,
					models.back().tex,
					models.back().tri,
					true, false, true);

				for (auto& nml : models.back().nml)
				{
					nml += glm::vec3(1, 1, 1);
					nml = nml * 0.5f;
				}
			}
		}
		else
		{
			std::cout << "read scene failed" << std::endl;
		}
	}

	/*!
	 * @brief ___Scene::NextModel___
	 * *******************************
	 * Displays the next model and resets relevant variables.
	*/
	void NextModel()
	{
		current_model = current_model + 1 >= models.size() ? 0 : ++current_model;
		current_draw_mode = DrawMode::Wireframe;
		rotate = false;
		orientation = 0.0;
	}

	/*!
	 * @brief ___Scene::NextDrawMode___
	 * **********************************
	 * Changes the draw mode of the displayed model.
	*/
	void NextDrawMode()
	{
		if (static_cast<int>(current_draw_mode) + 1 > 3)
		{
			current_draw_mode = static_cast<DrawMode>(0);
		}
		else
		{
			current_draw_mode = static_cast<DrawMode>(static_cast<int>(current_draw_mode) + 1);
		}
	}

	/*!
	 * @brief ___Scene::ToggleRotate___
	 * ***********************************
	 * Toggles if the current model should rotate or not.
	*/
	void ToggleRotate()
	{
		rotate = !rotate;
	}

	/*!
	 * @brief ___Scene::ProcessInput___
	 * ***********************************
	 * Processes scene inputs, to switch model, draw mode, rotate, etc.
	*/
	void ProcessInput()
	{
		if (GLHelper::keystateM && GLHelper::keystateM != keyMlast)
		{
			NextModel();
		}
		if (GLHelper::keystateW && GLHelper::keystateW != keyWlast)
		{
			NextDrawMode();
		}
		if (GLHelper::keystateR && GLHelper::keystateR != keyRlast)
		{
			ToggleRotate();
		}

		keyRlast = GLHelper::keystateR;
		keyWlast = GLHelper::keystateW;
		keyMlast = GLHelper::keystateM;
	}

	/*!
	 * @brief ___Scene::DrawScene___
	 * *******************************
	 * Renders the current selected model with the draw mode selected.
	 * Also in charge of updating the rotation if toggled.
	*/
	void DrawScene() 
	{
		// initialize random engine
		std::default_random_engine dre;
		dre.seed(12);

		// get numbers in range [-1,-1] 
		std::uniform_real_distribution<float> urdf(-1.0, 1.0);

		// update model transform
		if (Scene::rotate)
		{
			Scene::orientation += GLHelper::update_time();
		}
		GLPbo::viewport_xform(Scene::models[Scene::current_model]);

		switch (Scene::current_draw_mode)
		{
		case Scene::DrawMode::Wireframe:
			for (size_t i = 0; i < Scene::models[Scene::current_model].tri.size(); i += 3)
			{
				GLPbo::render_triangle_wireframe(Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 1]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 2]],
					{ 0,0,0,255 });
			}
			break;
		case Scene::DrawMode::WireframeColor:
			for (size_t i = 0; i < Scene::models[Scene::current_model].tri.size(); i += 3)
			{
				GLPbo::render_triangle_wireframe(Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 1]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 2]],
					{ GLubyte(urdf(dre) * 255.0f), GLubyte(urdf(dre) * 255.0f), GLubyte(urdf(dre) * 255.0f), 255 });
			}
			break;
		case Scene::DrawMode::FlatShaded:
			for (size_t i = 0; i < Scene::models[Scene::current_model].tri.size(); i += 3)
			{
				GLPbo::render_triangle(Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 1]],
					Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 2]],
					{ urdf(dre) * 255.0f, urdf(dre) * 255.0f, urdf(dre) * 255.0f });
			}
			break;
		case Scene::DrawMode::SmoothShaded:
			for (size_t i = 0; i < Scene::models[Scene::current_model].tri.size(); i += 3)
			{
				GLPbo::render_triangle(Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i]], Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 1]], Scene::models[Scene::current_model].pd[Scene::models[Scene::current_model].tri[i + 2]],
					Scene::models[Scene::current_model].nml[Scene::models[Scene::current_model].tri[i]] * 255.0f, Scene::models[Scene::current_model].nml[Scene::models[Scene::current_model].tri[i + 1]] * 255.0f, Scene::models[Scene::current_model].nml[Scene::models[Scene::current_model].tri[i + 2]] * 255.0f);
			}
			break;
		}
	}
}

/*                                                    static member functions
----------------------------------------------------------------------------- */
/*!
 * @brief part 4
 * I assume this is GLPbo::render()
 * Recalculates and updates the color in the pbo every frame.
*/
void GLPbo::emulate()
{
	// processes scene input
	Scene::ProcessInput();

	// binds the pbo buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboid);

	// part 4.1, and 4.2
	// fluctuate colors based on time and set it
	// set the color using set_clear_color()
	/*double fluctuate_cos = cos(glfwGetTime());
	double fluctuate_sin = sin(glfwGetTime());
	fluctuate_cos = fluctuate_cos < 0 ? -fluctuate_cos : fluctuate_cos;
	fluctuate_sin = fluctuate_sin < 0 ? -fluctuate_sin : fluctuate_sin;
	set_clear_color(static_cast<int>(fluctuate_cos * 255.0), static_cast<int>(fluctuate_cos * 100.0), static_cast<int>(fluctuate_sin * 255.0));*/
	// map pbo
	ptr_to_pbo = reinterpret_cast<Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));	/*!< here use GL_WRITE_ONLY */

	// part 4.3, clear color buffer
	clear_color_buffer();

	// draws scene
	Scene::DrawScene();

	// part 4.4, unmap pbo
	glUnmapNamedBuffer(pboid);

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, texid);

	// part 4.5, DMA (direct memory access)
	glTextureSubImage2D(texid, 0, 0, 0, width, height,
		GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// unbind buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
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
	// unbinds texture unit
	glBindTextureUnit(0, texid);
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

	// for tutorial 7 - load scene
	Scene::LoadScene("../scenes/ass-1.scn");

	// part 3.2, set GLPbo::clear_clr
	set_clear_color(255, 255, 255);

	// part 3.3, create pbo and set pbo id by mapping it
	glCreateBuffers(1, &pboid);
	glNamedBufferStorage(pboid, byte_cnt, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	// binds the pbo buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboid);

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
		GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// unbind pbo buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

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
	// delete shader program
	shdr_pgm.DeleteShaderProgram();
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
	std::fill(ptr_to_pbo, ptr_to_pbo + pixel_cnt, clear_clr);
}

/* ____________________________________________________________________________________________________
*									TUTORIAL 7 FUNCTIONS
*  ____________________________________________________________________________________________________
*/

/*!
 * @brief ___viewport_xform___
 * ****************************************************************************************************
 * Transformation that applies rotation and a viewport transform to all
 * vertices of a specified model.
 * ****************************************************************************************************
 * @param model 
 * : Model to transform.
 * ****************************************************************************************************
*/
void GLPbo::viewport_xform(Model& model)
{
	// rotation matrix
	double angle = Scene::orientation;
	glm::mat3 m_rotation{	 cos(angle),  sin(angle),	0,
							-sin(angle),  cos(angle),	0,
								0,		0,		1 };
	model.pd = model.pm;
	for (auto& pd : model.pd)
	{
		// multiply by rotation transform
		pd = m_rotation * pd;
		// apply to viewport
		pd.x = ((pd.x + 1.0f) / 2.0f) * width;
		pd.y = ((pd.y + 1.0f) / 2.0f) * height;
		pd.z = 0.0f;
	}
}

/*!
 * @brief ___set_pixel___
 * ****************************************************************************************************
 * Does scizzoring check and sets a color into the specified location
 * in the pbo.
 * ****************************************************************************************************
 * @param x 
 * : x-coordinate.
 * @param y 
 * : y-coordinate.
 * @param draw_clr
 * : Color to set the pixel to.
 * ****************************************************************************************************
*/
void GLPbo::set_pixel(GLint x, GLint y, GLPbo::Color draw_clr)
{
	// scizzoring test
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		std::cout << "scizzoring test failed" << std::endl;
		return;
	}
	// sets the color in pbo
	if (ptr_to_pbo)
	{
		ptr_to_pbo[y * width + x] = draw_clr;
	}
}

/*!
 * @brief ___line_bresenham_octant1256___
 * ****************************************************************************************************
 *	: Bresenham line drawing algorithm for if the line falls within octant 1256
 * ****************************************************************************************************
 * @param x1 
 *	: Start x
 * @param y1 
 *	: Start y
 * @param x2
 *	: End x
 * @param y2
 *	: End y
 * @param draw_clr
 *	: Color of the line to be drawn
 * ****************************************************************************************************
*/
void line_bresenham_octant1256(GLint x1, GLint y1, GLint x2, GLint y2, GLPbo::Color draw_clr)
{
	int dx = x2 - x1, dy = y2 - y1;
	int xstep = (dx < 0) ? -1 : 1;
	int ystep = (dy < 0) ? -1 : 1;
	dx = (dx < 0) ? -dx : dx;
	dy = (dy < 0) ? -dy : dy;
	int d = 2 * dx - dy, dmin = 2 * dx, dmaj = 2 * dx - 2 * dy;
	GLPbo::set_pixel(x1, y1, draw_clr);
	while (--dy > 0)
	{
		x1 += (d > 0) ? xstep : 0;
		d += (d > 0) ? dmaj : dmin;
		y1 += ystep;
		GLPbo::set_pixel(x1, y1, draw_clr);
	}
}

/*!
 * @brief ___ line_bresenham_octant0347___
 * ****************************************************************************************************
 *	Bresenham line drawing algorithm for if 
 *  the line falls within octant 0347
 * ****************************************************************************************************
 * @param x1
 *	: Start x
 * @param y1
 *	: Start y
 * @param x2
 *	: End x
 * @param y2
 *	: End y
 * @param draw_clr
 *	: Color of the line to be drawn
 * ****************************************************************************************************
*/
void line_bresenham_octant0347(GLint x1, GLint y1, GLint x2, GLint y2, GLPbo::Color draw_clr)
{
	int dx = x2 - x1, dy = y2 - y1;
	int xstep = (dx < 0) ? -1 : 1;
	int ystep = (dy < 0) ? -1 : 1;
	dx = (dx < 0) ? -dx : dx;
	dy = (dy < 0) ? -dy : dy;
	int d = 2 * dy - dx, dmin = 2 * dy, dmaj = 2 * dy - 2 * dx;
	GLPbo::set_pixel(x1, y1, draw_clr);
	while (--dx > 0)
	{
		y1 += (d > 0) ? ystep : 0;
		d += (d > 0) ? dmaj : dmin;
		x1 += xstep;
		GLPbo::set_pixel(x1, y1, draw_clr);
	}
}

/*!
 * @brief ___render_linebresenham___
 * ****************************************************************************************************
 * : Renders a line between 2 points using the bresenham line drawing algorithm
 * ****************************************************************************************************
 * @param px0 
 * : Start x
 * @param py0
 * : Start y
 * @param px1
 * : End x
 * @param py1
 * : End y
 * @param draw_clr
 * : Color of the line to be drawn
 * ****************************************************************************************************
*/
void GLPbo::render_linebresenham(GLint px0, GLint py0, GLint px1, GLint py1, GLPbo::Color draw_clr)
{
	// check if line to be drawn falls within 1256 or 0347
	// i.e. is rise <= run, 0347, else 1256
	GLint rise = abs(py1 - py0);
	GLint run = abs(px1 - px0);
	if (rise <= run)
	{
		line_bresenham_octant0347(px0, py0, px1, py1, draw_clr);
	}
	else
	{
		line_bresenham_octant1256(px0, py0, px1, py1, draw_clr);
	}
}

/*!
 * @brief ___render_triangle_wireframe___
 * ****************************************************************************************************
 * Renders a triangle in wireframe mode using bresenham line drawing algorithms.
 * ****************************************************************************************************
 * @param p0 
 * : Vertex 1 of the triangle.
 * @param p1 
 * : Vertex 2 of the triangle.
 * @param p2 
 * : Vertex 3 of the triangle.
 * @param clr 
 * : Color to render the triangle edges.
 * @return bool
 * : If triangle was successfully rendered. I.e. not backface-culled.
 * ****************************************************************************************************
*/
bool GLPbo::render_triangle_wireframe(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, const GLPbo::Color& clr)
{
	// backface culling
	glm::vec3 vec1 = p0 - p1;
	glm::vec3 vec2 = p2 - p1;
	if (vec1.x * vec2.y - vec2.x * vec1.y < 0.0)
	{
		render_linebresenham((GLint)p0.x, (GLint)p0.y, (GLint)p1.x, (GLint)p1.y, clr);
		render_linebresenham((GLint)p0.x, (GLint)p0.y, (GLint)p2.x, (GLint)p2.y, clr);
		render_linebresenham((GLint)p1.x, (GLint)p1.y, (GLint)p2.x, (GLint)p2.y, clr);
		return true;
	}
	return false;
}

/*!
 * @brief ___CalculateEvaluation___
 * ****************************************************************************************************
 * Calculated the evaluation value of a window coordinate to see if its
 * inside or outside an edge.
 * ****************************************************************************************************
 * @param edge
 * : Edge to check against.
 * @param x 
 * : x-coordinate.
 * @param y 
 * : y-coordinate.
 * @return float
 * : The evaluation value.
 * ****************************************************************************************************
*/
float CalculateEvaluation(const glm::vec3& edge, const float& x, const float& y)
{
	return edge.x * x + edge.y * y + edge.z;
}

/*!
 * @brief ___TopLeft___
 * ****************************************************************************************************
 * Precalculates top left for a given edge.
 * ****************************************************************************************************
 * @param edge 
 * : Edge that top left is being calculated for.
 * @return bool
 * : If the edge is a top left edge.
 * ****************************************************************************************************
*/
bool TopLeft(const glm::vec3& edge)
{
	if (edge.x > 0.f)	return true;	// on left edge
	if (edge.x < 0.f)	return false;	// on right edge
	if (edge.y < 0.f)	return true;	// on top edge
	return false;						// on bottom edge
}

/*!
 * @brief ___PointInEdgeTopLeft___
 * ****************************************************************************************************
 * reference link: https://drive.google.com/file/d/1BvbTMFUUp_SUjtVJZfi44NDx4oRa-7d4/view?usp=sharing.
 * Returns if an evaluated pixel belongs in the edge.
 * ****************************************************************************************************
 * @param edge 
 * : Edge to check with
 * @param eval 
 * : Precalculated evaluation
 * @return 
 * : If evaluation falls within the edge
 * ****************************************************************************************************
*/
bool PointInEdgeTopLeft(const glm::vec3& edge, const float& eval)
{
	if (eval > 0.f)		return true;	// completely inside edge
	if (eval < 0.f)		return false;	// completely outside edge
	if (edge.x > 0.f)	return true;	// on left edge
	if (edge.x < 0.f)	return false;	// on right edge
	if (edge.y < 0.f)	return true;	// on top edge
	return false;						// on bottom edge
}

/*!
 * @brief ___PointInEdgeTopLeftOptimized___
 * ****************************************************************************************************
 * Optimized check with precalculated top left of edge.
 * ****************************************************************************************************
 * @param eval 
 * : Incremented evaluation.
 * @param tl
 * : Precalculated topleft.
 * @return bool
 * : If evaluated point falls within edge.
 * ****************************************************************************************************
*/
bool PointInEdgeTopLeftOptimized(const float& eval, bool tl)
{
	return (eval > 0.f || (eval == 0.f && tl)) ? true : false;
}

/*!
 * @brief ___render_triangle___
 * ****************************************************************************************************
 * reference link: https://drive.google.com/file/d/1VomuXWXlopSPPvPuC3UtZXdCIyprOORS/view?usp=sharing
 * Renders a flat shaded triangle.
 * ****************************************************************************************************
 * @param p0 
 * : Vertex 1 of the triangle.
 * @param p1 
 * : Vertex 2 of the triangle.
 * @param p2
 * : Vertex 3 of the triangle.
 * @param clr 
 * : Color to shade the triangle.
 * @return bool
 * : If triangle was successfully rendered. I.e. not backface-culled.
 * ****************************************************************************************************
*/
bool GLPbo::render_triangle(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 clr)
{
	// backface culling
	glm::vec3 vec1 = p0 - p1;
	glm::vec3 vec2 = p2 - p1;
	if (vec1.x * vec2.y - vec2.x * vec1.y >= 0.0)
	{
		return false;
	}

	// compute edge equations
	// reference slides: https://drive.google.com/file/d/1BvbTMFUUp_SUjtVJZfi44NDx4oRa-7d4/view?usp=sharing
	glm::vec3 l0 = {	p1.y - p2.y,
						p2.x - p1.x,
						(p2.y - p1.y) * p1.x - (p2.x - p1.x) * p1.y };

	glm::vec3 l1 = {	p2.y - p0.y,
						p0.x - p2.x,
						(p0.y - p2.y) * p2.x - (p0.x - p2.x) * p2.y };

	glm::vec3 l2 = {	p0.y - p1.y,
						p1.x - p0.x,
						(p1.y - p0.y) * p0.x - (p1.x - p0.x) * p0.y };

	// compute aabb
	float xmin = std::min({ p0.x, p1.x, p2.x });
	float xmax = std::max({ p0.x, p1.x, p2.x });
	float ymin = std::min({ p0.y, p1.y, p2.y });
	float ymax = std::max({ p0.y, p1.y, p2.y });

	xmin = floor(xmin);
	xmax = ceil(xmax);
	ymin = floor(ymin);
	ymax = ceil(ymax);

	// evaluate
	float eval0 = CalculateEvaluation(l0, xmin + 0.5f, ymin + 0.5f);
	float eval1 = CalculateEvaluation(l1, xmin + 0.5f, ymin + 0.5f);
	float eval2 = CalculateEvaluation(l2, xmin + 0.5f, ymin + 0.5f);

	bool l0_tl = TopLeft(l0);
	bool l1_tl = TopLeft(l1);
	bool l2_tl = TopLeft(l2);

	// start rendering
	for (int y = (int)ymin; y < (int)ymax; ++y)
	{
		// start values for horizontal spans
		float h_eval0 = eval0, h_eval1 = eval1, h_eval2 = eval2;
		for (int x = (int)xmin; x < (int)xmax; ++x)
		{
			if (PointInEdgeTopLeftOptimized(h_eval0, l0_tl) && PointInEdgeTopLeftOptimized(h_eval1, l1_tl) && PointInEdgeTopLeftOptimized(h_eval2, l2_tl))
			{
				set_pixel(x, y, {(GLubyte)clr.x,(GLubyte)clr.y,(GLubyte)clr.z,255});
			}
			// incrementally update h_eval0, h_eval1, h_eval2
			h_eval0 += l0.x;
			h_eval1 += l1.x;
			h_eval2 += l2.x;
		}
		// incrementally update eval0, eval1, eval2
		eval0 += l0.y;
		eval1 += l1.y;
		eval2 += l2.y;
	}
	return true;
}

/*!
 * @brief ___render_triangle___
 * ****************************************************************************************************
 * Renders the triangle with smooth shading. I.e. same as flat shading
 * just with barycentric interpolated colors.
 * ****************************************************************************************************
 * @param p0 
 * : Vertex 1 of the triangle.
 * @param p1 
 * : Vertex 2 of the triangle.
 * @param p2 
 * : Vertex 3 of the triangle.
 * @param c0 
 * : Color at 1st vertex.
 * @param c1 
 * : Color at 2nd vertex.
 * @param c2 
 * : Color at 3rd vertex.
 * @return bool
 * : If triangle was successfully rendered. I.e. not backface-culled.
 * ****************************************************************************************************
*/
bool GLPbo::render_triangle(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 const& c0,
	glm::vec3 const& c1, glm::vec3 const& c2)
{
	// backface culling
	glm::vec3 vec1 = p0 - p1;
	glm::vec3 vec2 = p2 - p1;
	if (vec1.x * vec2.y - vec2.x * vec1.y >= 0.0)
	{
		return false;
	}

	// compute edge equations
	// reference slides: https://drive.google.com/file/d/1BvbTMFUUp_SUjtVJZfi44NDx4oRa-7d4/view?usp=sharing
	glm::vec3 l0 = {	p1.y - p2.y,
						p2.x - p1.x,
						(p2.y - p1.y) * p1.x - (p2.x - p1.x) * p1.y };

	glm::vec3 l1 = {	p2.y - p0.y,
						p0.x - p2.x,
						(p0.y - p2.y) * p2.x - (p0.x - p2.x) * p2.y };

	glm::vec3 l2 = {	p0.y - p1.y,
						p1.x - p0.x,
						(p1.y - p0.y) * p0.x - (p1.x - p0.x) * p0.y };

	// compute aabb
	float xmin = std::min({ p0.x, p1.x, p2.x });
	float xmax = std::max({ p0.x, p1.x, p2.x });
	float ymin = std::min({ p0.y, p1.y, p2.y });
	float ymax = std::max({ p0.y, p1.y, p2.y });

	xmin = floor(xmin);
	xmax = ceil(xmax);
	ymin = floor(ymin);
	ymax = ceil(ymax);

	// evaluate
	float eval0 = CalculateEvaluation(l0, xmin + 0.5f, ymin + 0.5f);
	float eval1 = CalculateEvaluation(l1, xmin + 0.5f, ymin + 0.5f);
	float eval2 = CalculateEvaluation(l2, xmin + 0.5f, ymin + 0.5f);

	bool l0_tl = TopLeft(l0);
	bool l1_tl = TopLeft(l1);
	bool l2_tl = TopLeft(l2);

	// Barycentric interpolation reference: https://drive.google.com/file/d/1HYkDd45NZ3EfSm_ow9YMNMFxJFi8dqWY/view?usp=sharing
	// calculate double area
	float double_area = (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);

	// calculate initial barycentric coordinates
	float ib_0 = eval0 / double_area;
	float ib_1 = eval1 / double_area;
	float ib_2 = eval2 / double_area;

	// calculate initial colors
	glm::vec3 ic_0 = ib_0 * c0;
	glm::vec3 ic_1 = ib_1 * c1;
	glm::vec3 ic_2 = ib_2 * c2;

	// scalar increment x
	glm::vec3 clr_incx_0 = (l0.x / double_area) * c0;
	glm::vec3 clr_incx_1 = (l1.x / double_area) * c1;
	glm::vec3 clr_incx_2 = (l2.x / double_area) * c2;

	// scalar increment y
	glm::vec3 clr_incy_0 = (l0.y / double_area) * c0;
	glm::vec3 clr_incy_1 = (l1.y / double_area) * c1;
	glm::vec3 clr_incy_2 = (l2.y / double_area) * c2;

	glm::vec3 v_color = ic_0 + ic_1 + ic_2;

	// start rendering
	for (int y = (int)ymin; y < (int)ymax; ++y)
	{
		// start values for horizontal spans
		float h_eval0 = eval0, h_eval1 = eval1, h_eval2 = eval2;
		glm::vec3 h_color = v_color;
		for (int x = (int)xmin; x < (int)xmax; ++x)
		{
			if (PointInEdgeTopLeftOptimized(h_eval0, l0_tl) && PointInEdgeTopLeftOptimized(h_eval1, l1_tl) && PointInEdgeTopLeftOptimized(h_eval2, l2_tl))
			{
				set_pixel(x, y, {(GLubyte)h_color.x,(GLubyte)h_color.y,(GLubyte)h_color.z,255});
			}
			// incrementally update h_eval0, h_eval1, h_eval2
			h_eval0 += l0.x;
			h_eval1 += l1.x;
			h_eval2 += l2.x;

			// incrementally update color on the x
			h_color += clr_incx_0 + clr_incx_1 + clr_incx_2;
		}
		// incrementally update eval0, eval1, eval2
		eval0 += l0.y;
		eval1 += l1.y;
		eval2 += l2.y;

		// incrementally update eval0, eval1, eval2
		v_color += clr_incy_0 + clr_incy_1 + clr_incy_2;
	}
	return true;
}