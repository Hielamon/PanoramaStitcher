#pragma once
#include <SPhoenix\Scene.h>

namespace SP
{
	class DemoScene : public Scene
	{
	public:
		DemoScene() {}
		~DemoScene() {}

		void addOpaqueMesh(const std::shared_ptr<Mesh>& pMesh,
						   const std::shared_ptr<ShaderProgram> &pShaderProgramTmp = nullptr)
		{
			if (pMesh.use_count() != 0)
			{
				mvpOpaqueMesh.push_back(pMesh);
				Scene::addMesh(pMesh, pShaderProgramTmp);
			}
		}

		void addTransparentMesh(const std::shared_ptr<Mesh>& pMesh,
						   const std::shared_ptr<ShaderProgram> &pShaderProgramTmp = nullptr)
		{
			if (pMesh.use_count() != 0)
			{
				mvpTransparentMesh.push_back(pMesh);
				Scene::addMesh(pMesh, pShaderProgramTmp);
			}
		}

		//The inputs is aimed for early pruning
		virtual void draw()
		{
			if (!mbUploaded)
			{
				SP_CERR("The current scene has not been uploaded befor drawing");
				return;
			}

			_drawMeshVector(mvpOpaqueMesh);
			_drawMeshVector(mvpTransparentMesh);
		}

		std::vector<std::shared_ptr<Mesh>> mvpOpaqueMesh;
		std::vector<std::shared_ptr<Mesh>> mvpTransparentMesh;

	private:
		void _drawMeshVector(std::vector<std::shared_ptr<Mesh>> &vpMesh)
		{
			for (size_t i = 0; i < vpMesh.size(); i++)
			{
				std::shared_ptr<Mesh> &pMesh = vpMesh[i];
				std::shared_ptr<ShaderProgram> &pShader =
					mmLabelToShader[mmMeshToLabel[pMesh]];
				pShader->useProgram();
				GLuint programID = pShader->getProgramID();

				if (pMesh->getAccessible())
				{
					pMesh->draw(programID);
				}
			}
		}

	};
}
