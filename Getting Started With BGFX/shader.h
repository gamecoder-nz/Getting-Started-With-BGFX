#pragma once

#include <bgfx/bgfx.h>

class ShaderProgram
{
public:
	ShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename);
	bgfx::ProgramHandle GetProgramHandle() { return m_ShaderProgramHandle; }
	~ShaderProgram();

private:
	bgfx::ProgramHandle m_ShaderProgramHandle;
	bgfx::ShaderHandle m_VertexShader;
	bgfx::ShaderHandle m_FragmentShader;

	bgfx::ShaderHandle CreateShader(const char* filename);
};