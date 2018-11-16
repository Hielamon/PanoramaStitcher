#pragma once

#include <SPhoenix\ShaderProgram.h>
#include <OpencvCommon.h>


namespace SP
{
	class UnifyFEShaderProgram : public ShaderProgram
	{
	public:
		UnifyFEShaderProgram(const std::string &vertFilePath, const std::string &fragFilePath)
			: ShaderProgram(vertFilePath, fragFilePath)
		{}
		~UnifyFEShaderProgram() {}

		virtual void createProgram()
		{
			ShaderProgram::createProgram();
			GLuint UnifyFEUBOIndex = glGetUniformBlockIndex(mProgramID, "UnifyFEUBO");
			glUniformBlockBinding(mProgramID, UnifyFEUBOIndex, 2);
		}

		virtual std::shared_ptr<ShaderProgram> createCopy()
		{
			return std::make_shared<UnifyFEShaderProgram>(*this);
		}
	};
}
