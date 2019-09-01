#version 460 core

/*
//precision highp float;
//precision highp int;
//layout(std140, column_major) uniform;

//uniform sampler2DShadow Shadow;
*/

in block
{
	vec4 color;
} in_data;

out vec4 color;

void main()
{
	color = in_data.color;
}

