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
#include <map>

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
  };

  // ___________________________________________________________________________________
  // tutorial 3's stuff
  using VPSS = std::vector<std::pair<std::string, std::string>>;
  static void init_shdrpgms_cont(const std::string& name, GLApp::VPSS const&);
  // ___________________________________________________________________________________
  // tutorial 4's stuff
  /*!
   * @brief insert_shdrpgm
   *	Creates the shader program based on input name in the .scn file
   *	if its not found in shdrpgms, else uses it.
   * @param shdr_pgm_name
   *	Name of the shader program to create.
   * @param vtx_shdr
   *	Filename of the vertex shader to use.
   * @param frg_shdr
   *	Filename of the fragment shader to use.
  */
  static void insert_shdrpgm(std::string shdr_pgm_name, std::string vtx_shdr, std::string frg_shdr);
  // ___________________________________________________________________________________
  // tutorial 5's stuff
  static GLuint setup_texobj(std::string pathname);

  struct GLObject
  {
	  //GLuint shd_ref;
	  std::map<std::string, GLApp::GLModel>::iterator	mdl_ref;	/*!< reference to model in a map */
	  std::map<std::string, GLSLShader>::iterator		shd_ref;	/*< reference to shader in a map */

	  GLuint texture;

	  // function to initialzie object's state
	  void init();
	  
	  // function to draw object model
	  void draw() const;
	  
	  // function to update the object's model transformation matrix
	  void update(GLdouble delta_time);
  };
  // removed in tutorial 3
  //static std::list<GLApp::GLObject> objects;	// singleton
  static std::map<std::string, GLObject>	objects;	// singleton
  static std::map<std::string, GLSLShader>	shdrpgms;	// singleton
  static std::map<std::string, GLModel>		models;	// singleton

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
