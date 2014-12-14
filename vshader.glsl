#version 150

in vec4 vPosition;
in vec2 tPosition;
in vec3 nPosition;

out vec2 tex;
out vec3 fN;
out vec3 fE;
out vec3 fL;

uniform vec3 theta;
uniform vec4 model_view;
uniform vec4 zoom;

void
main()
{
    vec3 angles = radians( theta );
    vec3 c = cos( angles );
    vec3 s = sin( angles );

    mat4 r_x = 
	  mat4( 1.0,  0.0,  0.0, 0.0,
		    0.0,  c.x,  s.x, 0.0,
		    0.0, -s.x,  c.x, 0.0,
		    0.0,  0.0,  0.0, 1.0 );

    mat4 r_y = 
	  mat4( c.y, 0.0, -s.y, 0.0,
		    0.0, 1.0,  0.0, 0.0,
		    s.y, 0.0,  c.y, 0.0,
		    0.0, 0.0,  0.0, 1.0 );

    mat4 r_z = 
	  mat4( c.z, -s.z, 0.0, 0.0,
		    s.z,  c.z, 0.0, 0.0,
		    0.0,  0.0, 1.0, 0.0,
		    0.0,  0.0, 0.0, 1.0 );
	
	mat4 scale =
	  mat4( zoom.x, 0.0, 0.0, 0.0,
		    0.0,  zoom.y, 0.0, 0.0,
		    0.0,  0.0, zoom.z, 0.0,
		    0.0,  0.0, 0.0, 1.0 );

    gl_Position = (r_z * r_y * r_x * vPosition + model_view) * scale;
	tex = tPosition;

    fN = nPosition;
    fE = vPosition.xyz;
    fL = vec3(0.0, 1.0, 2.0);
}
