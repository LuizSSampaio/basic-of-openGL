#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

struct ShaderProgramSources
{
	std::string vertexSource;
	std::string fragmentSource;
};

static ShaderProgramSources parseShader(const std::string& shaderPath)
{
	std::ifstream stream(shaderPath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream stringStream[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
			else
			{
				type = ShaderType::NONE;
				std::cout << "WARNING:	" << "Unknow shader | " << shaderPath << "\n";
			}
		}
		else
		{
			if (type != ShaderType::NONE)
			{
				stringStream[(int)type] << line << "\n";
			}
		}
	}

	return { stringStream[0].str(), stringStream[1].str() };
}

static GLuint compileShader(const std::string& source, GLuint type)
{
	GLuint id = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	GLint result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*)_malloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "ERROR:	" << message << "\n";

		glDeleteShader(id);
		return 0;
	}

	return id;
}

static GLuint createShader(const std::string& vertexShaderText, const std::string& fragmentShaderText)
{
	GLuint program = glCreateProgram();

	GLuint vertexShader = compileShader(vertexShaderText, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentShaderText, GL_FRAGMENT_SHADER);

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

int main()
{
	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
	{
		return -1;
	}

	// Create a windowned mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "OpenGL Test", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Initialize the GLEW
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return -1;
	}

	std::cout << glGetString(GL_VERSION) << "\n";

	{
		GLfloat positions[12]{
			-0.5f, -0.5f,
			0.5f, -0.5f,
			0.5f, 0.5f,
			-0.5f, 0.5f
		};

		GLuint indices[6] = {
			0, 1, 2,
			2, 3, 0
		};

		GLuint vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		VertexBuffer vertexBuffer(positions, 4 * 2 * sizeof(GLfloat));

		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0));

		IndexBuffer indexBuffer(indices, 6);

		ShaderProgramSources parsedShader = parseShader("res/shaders/Basic.glsl");
		GLuint program = createShader(parsedShader.vertexSource, parsedShader.fragmentSource);
		GLCall(glUseProgram(program));

		GLint colorUniformLocation = glGetUniformLocation(program, "u_Color");
		ASSERT(colorUniformLocation != -1)
		glUniform4f(colorUniformLocation, 1.0f, 0.75f, 0.5f, 1.0f);

		GLCall(glBindVertexArray(0));
		GLCall(glUseProgram(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		// Loop until the user close the window
		while (!glfwWindowShouldClose(window))
		{
			// Render here
			glClear(GL_COLOR_BUFFER_BIT);

			GLCall(glUseProgram(program));

			glBindVertexArray(vao);

			indexBuffer.Bind();

			// Draw a triangle at the screen
			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

			indexBuffer.UnBind();

			// Swap front and back buffers
			glfwSwapBuffers(window);

			// Poll for and process events
			glfwPollEvents();
		}

		glDeleteProgram(program);
	}

	glfwTerminate();

	return 0;
}