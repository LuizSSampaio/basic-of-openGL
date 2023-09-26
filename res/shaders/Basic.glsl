#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

void main()
{
	gl_position = position;
} 

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0, 0.5, 0.25, 1.0);
}