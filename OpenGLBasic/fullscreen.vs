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


out block
{
	vec2 uv;
} out_data;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	out_data.uv = vec2((x+1.0)*0.5, (y+1.0)*0.5);
	gl_Position = vec4(x, y, 0, 1);
}

