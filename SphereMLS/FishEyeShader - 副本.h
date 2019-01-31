#pragma once

#include <SPhoenix\ShaderProgram.h>
#include <OpencvCommon.h>


namespace SP
{
	class UnifyFEShaderProgram : public ShaderProgram
	{
	public:

		UnifyFEShaderProgram()
			: ShaderProgram() {}

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
			std::shared_ptr<UnifyFEShaderProgram> pUnifyFEShader
				= std::make_shared<UnifyFEShaderProgram>();

			for (size_t i = 0; i < SHADER_KINDS; i++)
			{
				pUnifyFEShader->setShaderCode(mvCode[i], ShaderType(i));
				pUnifyFEShader->setShaderPath(mvShaderPath[i], ShaderType(i));
			}

			return pUnifyFEShader;
		}
	};
}
