#version 450 core

layout (location=0) in vec3 vInterpColor;
layout (location=1) in vec2 vTexCoord;
layout (location=2) in vec2 vTexCoord2;
layout (location=0) out vec4 fFragColor;

uniform int uMode;
uniform bool uBlend;
uniform int uTileSize;

uniform sampler2D uTex2d;

void main () 
{
	// vertex color mode
	if (uMode == 0) {
		fFragColor = vec4(vInterpColor, 1.0);
	}

	// checkered mode
	else if (uMode == 1) {
		// checkered data
		int		size = 32;
		vec3	color0 = vec3(1.0,0.0,1.0);
		vec3	color1 = vec3(0.0,0.68,0.94);

		// checkered calculation
		int		check_x = int(floor(float(gl_FragCoord.x)/float(size)));
		int		check_y = int(floor(float(gl_FragCoord.y)/float(size)));
		int		check = int(mod(check_x+check_y,2));

		// output checkered color
		if (check == 0) {
			fFragColor = vec4(color0, 1.0);
		}
		else {
			fFragColor = vec4(color1, 1.0);
		}
	}
	
	// ease checkered
	else if (uMode == 2) {
		// checkered data
		vec3	color0 = vec3(1.0,0.0,1.0);
		vec3	color1 = vec3(0.0,0.68,0.94);

		// checkered calculation
		int		check_x = int(floor(float(gl_FragCoord.x)/float(uTileSize)));
		int		check_y = int(floor(float(gl_FragCoord.y)/float(uTileSize)));
		int		check = int(mod(check_x+check_y,2));

		// output checkered color
		if (check == 0) {
			fFragColor = vec4(color0, 1.0);
		}
		else {
			fFragColor = vec4(color1, 1.0);
		}
	}

	// 1 duck
	else if (uMode == 3) {
		fFragColor = texture(uTex2d, vTexCoord);
	}
	// multiple ducks
	else if (uMode >= 4) {
		fFragColor = texture(uTex2d, vTexCoord2);
	}

	// output checkered color blended with vertex color
	if (uBlend && uMode != 0) {
		fFragColor = vec4(	fFragColor.x*vInterpColor.x,
							fFragColor.y*vInterpColor.y,
							fFragColor.z*vInterpColor.z, 
							fFragColor.a	);
	}
}