/* !
@file    glapp.h
@author  pghali@digipen.edu
@date    10/11/2016

This file contains the declaration of namespace GLApp that encapsulates the
functionality required to implement an OpenGL application including 
compiling, linking, and validating shader programs
setting up geometry and index buffers, 
configuring VAO to present the buffered geometry and index data to
vertex shaders,
configuring textures (in later labs),
configuring cameras (in later labs), 
and transformations (in later labs).
*//*__________________________________________________________________________*/

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLAPP_H
#define GLAPP_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <GL/glew.h> // for access to OpenGL API declarations 
#include <GLFW/glfw3.h>
#include <glhelper.h>
#include <glslshader.h>
#include <list>
#include <random>

struct GLApp {
  static void init();
  static void update(double delta_time);
  static void draw();
  static void cleanup();

  struct GLModel {
	  GLenum		primitive_type{0};	// which OpenGL primitive to be rendered?
	  GLuint		primitive_cnt{0};	// added for tutorial 2
	  GLuint		vaoid{0};			// handle to VAO

	  GLuint		draw_cnt{0};		// added for tutorial 2

	  //GLSLShader	shdr_pgm;		// which shader program?

	  // no longer required in tutorial 3
	  ///*!
	  // * @brief setup_shdrpgm
	  // *	- tutorial 2 primitive setup
	  // * @param vtx_shdr 
	  // *	- vertex shader
	  // * @param frg_shdr 
	  // *	- fragment shader
	  //*/
	  //void setup_shdrpgm(std::string vtx_shdr, std::string frg_shdr);
	  //void draw();
  };
  // data member to represent geometric model to be rendered
  // C++ requires this object to have a definition in glapp.cpp!!!
  static std::vector<GLApp::GLModel> models;

  // tutorials 2's replacement for setup_vao for GL_POINT primitives
  static GLApp::GLModel points_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel lines_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel trifans_model(int slices, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel tristrip_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

  // ___________________________________________________________________________________
  // tutorial 3's stuff
  static std::vector<GLSLShader> shdrpgms;	// singleton in tutorial 3
  static void init_models_cont();

  static GLApp::GLModel box_model();
  
  static GLApp::GLModel mystery_model();

  using VPSS = std::vector<std::pair<std::string, std::string>>;
  static void init_shdrpgms_cont(GLApp::VPSS const&);

  struct GLObject
  {
		GLfloat	angle_speed;	// rate of change of rotation angle per second
		GLfloat	angle_disp;		// current absolute orientation angle

		glm::vec2 scaling;		// scaling parameters
		glm::vec2 position;		// translation vector coordinates

		// model transform = scaling rotation and translation
		glm::mat3 mdl_to_ndc_xform;

		// reference to model
		GLuint mdl_ref;

		// reference to shader to draw
		GLuint shd_ref;

		// function to initialzie object's state
		void init();

		// function to draw object model
		void draw() const;

		// function to update the object's model transformation matrix
		void update(GLdouble delta_time);
  };
  static std::list<GLApp::GLObject> objects;	// singleton

  /*!
   * @brief CreateObject()
   *	- a function that encapsulates creation of a GLObject and
   *	pushing it to objects container
  */
  static void CreateObject();
  // ___________________________________________________________________________________

  /*!
   * @brief GLViewport
   *	- Encapsulates specification of viewport
  */
  struct GLViewport
  {
	  GLint x, y;						/*!< viewport position */
	  GLsizei width, height;			/*!< viewport dimensions */
  };
  static std::vector<GLViewport> vps;	/*!< container for viewports */
};

#endif /* GLAPP_H */
