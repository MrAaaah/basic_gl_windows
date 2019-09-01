#version 460 core

/*

//precision highp float;
//precision highp int;
//layout(std140, column_major) uniform;

//uniform transform
//{
	//mat4 MVP;
	//mat4 DepthMVP;
	//mat4 DepthBiasMVP;
//} Transform;
*/

in vec3 position;
in vec4 color;

out block
{
	vec4 color;
} out_data;

void main()
{
	gl_Position = vec4(position, 1.0);
	out_data.color = color;
}

