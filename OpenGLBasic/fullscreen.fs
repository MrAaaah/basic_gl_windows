#version 460 core

/*
//precision highp float;
//precision highp int;
//layout(std140, column_major) uniform;

//uniform sampler2DShadow Shadow;
*/

in block
{
	vec2 uv;
} in_data;

out vec4 color;

uniform float time;

float spiral_sdf(vec2 st, float t)
{
	st -= vec2(0.5, 0.5);
	float r = dot(st, st);
	float a = atan(st.y, st.x);
	return abs(sin(fract(log(r)*t+a*0.159)));
}

void main()
{
	color = vec4(in_data.uv, 0.0, 1.0);
	color *= step(0.5, spiral_sdf(in_data.uv, time));
}

