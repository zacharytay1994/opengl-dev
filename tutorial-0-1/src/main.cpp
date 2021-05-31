/*!
@file    main.cpp
@author  pghali@digipen.edu
@date    10/11/2016

This file uses functionality defined in type GLApp to initialize an OpenGL
context and implement a game loop.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
// Extension loader library's header must be included before GLFW's header!!!
#include "../include/glapp.h"
#include <iostream>
#include <sstream>
#include <iomanip>

/*                                                   type declarations
----------------------------------------------------------------------------- */

/*                                                      function declarations
----------------------------------------------------------------------------- */
static void draw(GLFWwindow*);
static void update(GLFWwindow*);

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
GLApp gl_app;

/*                                                      function definitions
----------------------------------------------------------------------------- */
/*  _________________________________________________________________________ */
/*! main

@param none

@return int

Indicates how the program existed. Normal exit is signaled by a return value of
0. Abnormal termination is signaled by a non-zero return value.
Note that the C++ compiler will insert a return 0 statement if one is missing.
*/
int main() {
  // start with a 16:9 aspect ratio
  if (!gl_app.init(2400, 1350, "Lab 0: Setting up OpenGL 4.5")) {
    std::cout << "Unable to create OpenGL context" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // window's close flag is set by clicking close widget or Alt+F4
  while (!glfwWindowShouldClose(GLApp::ptr_window)) {
    draw(GLApp::ptr_window);
    update(GLApp::ptr_window);
  }

  gl_app.cleanup();
}

/*  _________________________________________________________________________ */
/*! draw

@param GLApp::GLFWwindow*
Handle to window that defines the OpenGL context

@return none

For now, there's nothing to draw - just paint colorbuffer with constant color
*/
static void draw(GLFWwindow *ptr_win) {
  // clear colorbuffer with RGBA value in glClearColor ...
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.f, 1.f, 0.f, 1.f);

  // swap buffers: front <-> back
  glfwSwapBuffers(ptr_win);
}

/*  _________________________________________________________________________ */
/*! update

@param GLFWwindow*
Handle to window that defines the OpenGL context

@return none

For now, there are no objects to animate nor keyboard, mouse button click,
mouse movement, and mouse scroller events to be processed.
The only event triggered is window resize.
*/
static void update(GLFWwindow *ptr_win) {
  // time between previous and current frame
  double delta_time = gl_app.update_time(1.0);
  // write window title with current fps ...
  std::stringstream sstr;
  sstr << std::fixed << std::setprecision(2) << GLApp::title << ": " << GLApp::fps;
  glfwSetWindowTitle(GLApp::ptr_window, sstr.str().c_str());

  glfwPollEvents();
}
