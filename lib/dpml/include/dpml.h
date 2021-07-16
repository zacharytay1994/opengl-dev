#ifndef DPML_H
#define DPML_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "glm/glm.hpp"

namespace DPML
{
  /*  _________________________________________________________________________*/
  /*! parse_obj_mesh
  This function parses an OBJ geometry file and stores the contents of the file
  as array of vertex, array of normal (if required), and an array of texture
  (if required) coordinate data. These three arrays will have the same size.
  Triangles are defined as an array of indices into array of position
  coordinates.

  @param std::string filename
  The name of the file containing the OBJ geometry information.

  @param std::vector<glm::vec3>& positions
  Fill user-supplied container with vertex position attributes.
  The function WILL assume that the container is empty!!!

  @param std::vector<glm::vec3>& normals
  Fill user-supplied container with vertex normal attributes.
  The function WILL assume that the container is empty!!!
  The container will not be touched if parameter "nml_attribs_flag" is
  false.
  If filled, this container will be the same size as "positions".

  @param std::vector<glm::vec2>& texcoords
  Fill user-supplied container with vertex texture coordinate attributes.
  The function WILL assume that the container is empty!!!
  The container will not be touched if parameter "texcoords_attribs_flag" is
  false.
  If filled, this container will be the same size as "positions".

  @param std::vector<unsigned short>& triangles
  Triangle vertices are specified as indices into containers "positions",
  "normals", and "texcoords". Triangles will always have counter-clockwise
  orientation. This means that when looking at a face from the outside of
  the box, the triangles are counter-clockwise oriented.
  Use an indexed draw call to draw the box.

  @param bool load_tex_coord_flag = false
  If parameter is true, then texture coordinates (if present in file) will
  be parsed. Otherwise, texture coordinate (even if present in file) will
  not be read.

  @param bool load_nml_coord_flag = false
  If parameter is true, then per-vertex normal coordinates (if present in file)
  will be parsed if they are present in file, otherwise, the per-vertex
  normals are computed.
  If the parameter is false, normal coordinate will neither be read from
  file (if present) nor explicitly computed.

  @param bool model_centered_flag = true
  In some cases, the modeler might have generated the model such that the
  center (of gravity) of the model is not centered at the origin.
  If the parameter is true, then the function will compute an axis-aligned
  bounding box and translate the position coordinates so that the box's center
  is at the origin.
  If the parameter is false, the position coordinates are left untouched.

  @return bool
  true if successful, otherwise false.
  The function can return false if the file is not present or the file
  is unreadable or doesn't follow the OBJ file format.
  */
  bool parse_obj_mesh(std::string filename,
    std::vector<glm::vec3>& positions,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& texcoords,
    std::vector<unsigned short>& triangles,
    bool                          load_nml_coord_flag,
    bool                          load_tex_coord_flag,
    bool                          model_centered_flag = true
  );

} // end namespace DPML

#endif
