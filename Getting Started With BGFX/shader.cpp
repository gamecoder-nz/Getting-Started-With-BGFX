#include "shader.h"
#include <fstream>
#include <print>
#include <vector>

ShaderProgram::ShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
	m_VertexShader = CreateShader(vertexShaderFilename);
	m_FragmentShader = CreateShader(fragmentShaderFilename);

	m_ShaderProgramHandle = bgfx::createProgram(m_VertexShader, m_FragmentShader, true);
}

ShaderProgram::~ShaderProgram()
{
	bgfx::destroy(m_ShaderProgramHandle);
}

bgfx::ShaderHandle ShaderProgram::CreateShader(const char* filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		std::print("Unable to open file {}", filename);
	}

	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> fileBuffer(fileSize);

	file.read(fileBuffer.data(), fileSize);
	bgfx::ShaderHandle shader = bgfx::createShader(bgfx::copy(fileBuffer.data(), fileSize));

	if (bgfx::isValid(shader) == false)
	{
		std::print("Unable to create shader");
	}

	file.close();

	return shader;
}