#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__));

static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "OPENGL ERROR: " << error << " | " << function << " : " << file << " : " << line << "\n";
		return false;
	}
	return true;
}

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

	GLuint vertexBuffer;
	GLCall(glGenBuffers(1, &vertexBuffer));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
	GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), positions, GL_STATIC_DRAW));

	GLCall(glEnableVertexAttribArray(0));
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0));

	GLuint indexBuffer;
	GLCall(glGenBuffers(1, &indexBuffer));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW));


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

		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));

		// Draw a triangle at the screen
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	glDeleteProgram(program);

	glfwTerminate();

	return 0;
}