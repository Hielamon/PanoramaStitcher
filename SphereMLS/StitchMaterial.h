#pragma once

#include <SPhoenix\Material.h>
#include "DemoCamera.h"

namespace SP
{
	class StitchMaterial : public Material
	{
	public:
		StitchMaterial() {}
		~StitchMaterial() {}

		void addDemoCamera(const std::shared_ptr<DemoCamera> &pDemoCamera)
		{
			std::stringstream ioStr;
			ioStr << "material.diffuse_maps[" << mvpDemoCamera.size() << "]";
			mvTexName.push_back(ioStr.str());

			ioStr.str("");
			ioStr << "CameraModel[" << mvpDemoCamera.size() << "]"; 
			mvCamMName.push_back(ioStr.str());

			ioStr.str("");
			ioStr << "CameraColor[" << mvpDemoCamera.size() << "]";
			mvCamColorName.push_back(ioStr.str());

			mvpDemoCamera.push_back(pDemoCamera);
		}

		//Get the macro for the shader codes, which defines some necessary variable 
		//Such as texture type the material holds, which is picked from the
		//TextypeToMacro of the TextureGlobal
		virtual std::string getShaderMacros()
		{
			std::string macros = "";
			
			if (mvpDemoCamera.size() > 0)
			{
				std::stringstream ioStr;
				ioStr << "#define CAMERA_NUM " << mvpDemoCamera.size() << "\n";
				
				macros += ioStr.str();
			}

			return macros;
		}

		//Material Activate function which is applied before every draw commands
		//, and after the useProgram statement.
		//This function need two inputs : bValidTexCoord and bValidVertexColor
		//If the texture coords are invalid there is a valid vertex color array
		//The active funtion will just update the shinness paramater,
		//otherwise it will intelligently choose
		//whether to use texture maps or the Blinn-Phong uniform color
		virtual void active(const GLuint &programID,
							bool bValidTexCoord = true,
							bool bValidVertexColor = false)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current material has not been uploaded befor activing");
				return;
			}

			GLint uShininessLoc = glGetUniformLocation(programID, "material.uShininess");
			glUniform1f(uShininessLoc, mShininess);

			GLint uShininessStrengthLoc =
				glGetUniformLocation(programID, "material.uShininessStrength");
			glUniform1f(uShininessStrengthLoc, mShininessStrength);


			for (size_t i = 0; i < mvpDemoCamera.size(); i++)
			{
				int texUnit = TextureType::Tex_DIFFUSE;
				std::shared_ptr<DemoCamera> &pDemoCamera = mvpDemoCamera[i];
				GLuint texBuffer = 0;
				if (pDemoCamera->getMaterialFBO()->getTexbuffer(texUnit, texBuffer))
				{
					int sampleriLoc = glGetUniformLocation(programID, mvTexName[i].c_str());
					glUniform1i(sampleriLoc, int(i));

					glActiveTexture(GL_TEXTURE0 + i);
					glBindTexture(GL_TEXTURE_2D, texBuffer);
				}

				//upload the model matrix
				glm::mat4 projMatrix = pDemoCamera->getProjectionMatrix();
				glm::mat4 viewMatrix = pDemoCamera->getViewMatrix();
				glm::mat4 M = projMatrix * viewMatrix;
				int camModeliLoc = glGetUniformLocation(programID, mvCamMName[i].c_str());
				glUniformMatrix4fv(camModeliLoc, 1, GL_FALSE, glm::value_ptr(M));

				int camColoriLoc = glGetUniformLocation(programID, mvCamColorName[i].c_str());
				glUniform4fv(camColoriLoc, 1, glm::value_ptr(pDemoCamera->getCameraColor()));
			}
			glActiveTexture(GL_TEXTURE0);
			

			if (!bValidVertexColor)
			{
				uploadColor(programID, "material.uDiffuse", mDiffuseColor);
				uploadColor(programID, "material.uAmbient", mAmbientColor);
				uploadColor(programID, "material.uSpecular", mSpecularColor);
			}
		}


	private:
		std::vector<std::shared_ptr<DemoCamera>> mvpDemoCamera;
		std::vector<std::string> mvTexName;
		std::vector<std::string> mvCamMName;
		std::vector<std::string> mvCamColorName;
	};
}
