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
  /*!
   * @brief init_scene
   *	Initializes scene from a .scn file.
   * @param scene_filename
   *	Filename of the scene.
  */
  static void init_scene(std::string scene_filename);

  struct GLObject
  {
	  // removed in tutorial 4
	  //GLfloat	angle_speed;	// rate of change of rotation angle per second
	  //GLfloat	angle_disp;		// current absolute orientation angle
	  glm::vec2 orientation;	/*!< orientation.x is angle_disp and
									 orientation.y is angle_speed
									 both values specified in degrees */

	  glm::vec2 scaling;		// scaling parameters
	  glm::vec2 position;		// translation vector coordinates
	  
	  // model transform = scaling rotation and translation
	  glm::mat3 mdl_to_ndc_xform;
	  
	  // removed in tutorial 4
	  //// reference to model
	  //GLuint mdl_ref;
	  //// reference to shader to draw
	  //GLuint shd_ref;
	  std::map<std::string, GLApp::GLModel>::iterator	mdl_ref;	/*!< reference to model in a map */
	  std::map<std::string, GLSLShader>::iterator		shd_ref;	/*< reference to shader in a map */
	  
	  // added in tutorial 4
	  glm::vec3 color;
	  glm::mat3 mdl_xform;	/*!< model transformation */

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

  struct Camera2D
  {
	  GLObject* pgo{nullptr};	// pointer to game object that embeds camera
	  glm::vec2 right{}, up{};
	  glm::mat3 view_xform{};
	  glm::mat3 camwin_to_ndc_xform{};
	  glm::mat3 world_to_ndc_xform{};
	  // additional parameters ...
	  GLint height{ 1000 };
	  GLfloat ar{0.0f};		// aspect ratio

	  // window change parameters ...
	  GLint min_height{ 500 }, max_height{ 2000 };
	  // height is increasing if 1 and decreasing if -1
	  GLint height_chg_dir{ 1 };
	  // incrememts by which window height is changes per Z key press
	  GLint height_chg_val{ 5 };
	  // camera's speed when button U is pressed
	  GLfloat linear_speed{ 2.f };
	  // camera's turn speed
	  GLfloat turn_speed{ 2.f };
	  
	  // keyboard button press flags
	  GLboolean camtype_flag	{ GL_FALSE };	// button V
	  GLboolean zoom_flag		{ GL_FALSE };	// button Z
	  GLboolean left_turn_flag	{ GL_FALSE };	// button H
	  GLboolean right_turn_flag	{ GL_FALSE };	// button K
	  GLboolean move_flag		{ GL_FALSE };	// button U

	  void init(GLFWwindow* window, GLObject* ptr);
	  void update(GLFWwindow* window, GLdouble delta_time);
  };
  static Camera2D camera2d;
};

#endif /* GLAPP_H */
