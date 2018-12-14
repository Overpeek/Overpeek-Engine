#include "shader.h"
#include "window.h"
#include "../tools/filereader.h"
#include "../tools/logger.h"

namespace graphics {

	Shader::Shader(const char *vertexPath, const char *fragmentPath) {

		//Vertex shader
		tools::Logger::info("Reading vertex shader...");
		std::string vertexShaderStr = tools::readFile(vertexPath);
		const char *vertexShaderChar = vertexShaderStr.c_str();

		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderChar, NULL);
		glCompileShader(vertexShader);

		//Fragment shader
		tools::Logger::info("Reading fragment shader...");
		std::string fragmentShaderStr = tools::readFile(fragmentPath);
		const char *fragmentShaderChar = fragmentShaderStr.c_str();

		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderChar, NULL);
		glCompileShader(fragmentShader);
		tools::Logger::info("Shader compilation done");

		//Compilation error logger
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			tools::Logger::error(std::string("Vertex shader compilation failed!\n") + infoLog);
			glfwTerminate();
			system("pause");
			exit(EXIT_FAILURE);
		}
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			tools::Logger::error(std::string("Fragment shader compilation failed!\n") + infoLog);
			glfwTerminate();
			system("pause");
			exit(EXIT_FAILURE);
		}

		//Shader program
		mShaderProgram = glCreateProgram();
		glAttachShader(mShaderProgram, vertexShader);
		glAttachShader(mShaderProgram, fragmentShader);
		glLinkProgram(mShaderProgram);

		glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(mShaderProgram, 512, NULL, infoLog);
			tools::Logger::error(std::string("Shader program linking failed!\n") + infoLog);
			glfwTerminate();
			system("pause");
			exit(EXIT_FAILURE);
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

}