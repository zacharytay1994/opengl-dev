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

	  GLSLShader	shdr_pgm;		// which shader program?

	  /*!
	   * @brief setup_shdrpgm
	   *	- tutorial 2 primitive setup
	   * @param vtx_shdr 
	   *	- vertex shader
	   * @param frg_shdr 
	   *	- fragment shader
	  */
	  void setup_shdrpgm(std::string vtx_shdr, std::string frg_shdr);
	  void draw();
  };
  // data member to represent geometric model to be rendered
  // C++ requires this object to have a definition in glapp.cpp!!!
  static std::vector<GLApp::GLModel> models;

  // tutorials 2's replacement for setup_vao for GL_POINT primitives
  static GLApp::GLModel points_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel lines_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel trifans_model(int slices, std::string vtx_shdr, std::string frg_shdr);

  static GLApp::GLModel tristrip_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr);

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
