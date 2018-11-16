#pragma once
#include <SPhoenix\Sphere.h>

namespace SP
{
	class TMesh : public Mesh
	{
	public:
		TMesh() {}
		TMesh(Mesh &mesh) : Mesh(mesh) {}
		~TMesh() {}

		virtual void drawOnlyInScene(const GLuint &programID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current mesh has not been uploaded befor drawing");
				return;
			}

			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			mpVertexArray->draw(programID);
			glCullFace(GL_BACK);
			mpVertexArray->draw(programID);
			glDisable(GL_CULL_FACE);
		}

		virtual void draw(const GLuint &programID)
		{
			if (!mbUploaded)
			{
				SP_CERR("The current mesh has not been uploaded befor drawing");
				return;
			}

			int uMeshIDLoc = glGetUniformLocation(programID, "uMeshID");
			glUniform1ui(uMeshIDLoc, mMeshID);

			mpMaterial->active(programID, mbHasTexCoord, mbHasVertexColor);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			mpVertexArray->draw(programID);
			glCullFace(GL_BACK);
			mpVertexArray->draw(programID);
			glDisable(GL_CULL_FACE);
		}
	};

	class GridMesh : public Mesh
	{
	public:
		GridMesh() {}
		GridMesh(Mesh &mesh) : Mesh(mesh) {}
		~GridMesh() {}

		virtual void drawOnlyInScene(const GLuint &programID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			Mesh::drawOnlyInScene(programID);

			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}

		virtual void draw(const GLuint &programID)
		{
			GLint rastMode;
			glGetIntegerv(GL_POLYGON_MODE, &rastMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			Mesh::draw(programID);

			glPolygonMode(GL_FRONT_AND_BACK, rastMode);
		}
	};

	class TCubeSphere : public TMesh
	{
	public:
		TCubeSphere(float radius, glm::vec4 color, int subdivison = 0)
			//: CubeSphere(radius, color, subdivison)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = 2.0f * std::sqrtf(3.0f) * radius / 3.0f;
			float a_2 = a * 0.5f;
			std::vector<glm::vec3> vCubeVertice =
			{
				//up rectangle
				{ a_2, a_2, a_2 },
				{ a_2, a_2, -a_2 },
				{ -a_2, a_2, -a_2 },
				{ -a_2, a_2, a_2 },

				//down rectangle
				{ a_2, -a_2, a_2 },
				{ a_2, -a_2, -a_2 },
				{ -a_2, -a_2, -a_2 },
				{ -a_2, -a_2, a_2 }
			};

			std::vector<GLuint> vCubeIndice =
			{
				0, 1, 2,/*  */0, 2, 3,
				////////////////////////y-positive up
				0, 7, 4,/*  */0, 3, 7,
				////////////////////////z-positive back 
				0, 4, 5,/*  */0, 5, 1,
				////////////////////////x-positive right
				3, 6, 7,/*  */3, 2, 6,
				////////////////////////x-negative left
				1, 5, 6,/*  */1, 6, 2,
				////////////////////////z-negative front
				4, 6, 5,/*  */4, 7, 6
				////////////////////////y-negative down 
			};

			int numInterval = 1 << subdivison;
			int numVertice = 2 * (numInterval + 1) * (numInterval + 1) +
				(numInterval - 1) * numInterval * 4;
			int numFace = 6 * numInterval * numInterval * 2;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 3);

			{
				float aInterval = a / numInterval;

				//Add the top cube sphere part
				{
					int vertIdx = 0, indiceIdx = 0;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, a_2, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = a_2 - aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart -= aInterval;
					}


				}

				std::vector<GLuint> vLastUpRowVertIdx(numInterval * 4);
				{
					//Left up
					for (int i = 0, vertIdx = 0;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vLastUpRowVertIdx[i] = vertIdx;
					//Front up
					for (int i = 0, vertIdx = (numInterval + 1) * numInterval;
						 i < numInterval; i++, vertIdx++)
						vLastUpRowVertIdx[i + numInterval] = vertIdx;
					//Right up
					for (int i = 0, vertIdx = (numInterval + 1) * (numInterval + 1) - 1;
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vLastUpRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back up
					for (int i = 0, vertIdx = numInterval;
						 i < numInterval; i++, vertIdx--)
						vLastUpRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the middle cube sphere parts
				//Left-->Front-->Right-->Back
				{
					int vertIdx = (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numInterval * numInterval * 2) * 3;

					if (numInterval > 1)
					{
						float yStart = a_2 - aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;
					}

					for (int i = 2; i < numInterval; i++)
					{
						float yStart = a_2 - float(i) * aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 2] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;

						yStart -= aInterval;
					}

					if (numInterval > 1)
					{
						int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1)
							- numInterval * 4;
						for (int i = 0; i < numInterval * 4; i++)
						{
							vLastUpRowVertIdx[i] = vertIdx;
							vertIdx++;
						}
					}
				}

				//Add the bottom cube sphere part
				{
					int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numFace - numInterval * numInterval * 2) * 3;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, -a_2, -a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = -a_2 + aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, -a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, -a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart += aInterval;
					}
				}

				std::vector<GLuint> vDownRowVertIdx(numInterval * 4);
				{
					//Left down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1);
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vDownRowVertIdx[i] = vertIdx;
					//Front down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * (numInterval + 1);
						 i < numInterval; i++, vertIdx++)
						vDownRowVertIdx[i + numInterval] = vertIdx;
					//Right down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * numInterval - 1;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vDownRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back down
					for (int i = 0, vertIdx = numVertice - 1;
						 i < numInterval; i++, vertIdx--)
						vDownRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the faces between the last up and the Down row vertices
				{
					int indiceIdx = (numFace - (numInterval + 4) * numInterval * 2) * 3;
					int i = 0;
					for (; i < numInterval * 4 - 1; i++)
					{
						vIndice[indiceIdx] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 5] = vDownRowVertIdx[i + 1];
						indiceIdx += 6;
					}

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];
				}

			}

			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				vert = glm::normalize(vert);
				glm::vec3 vertColor = 0.5f * (vert + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(vertColor, color.w));

				vert *= radius;
			});

			std::vector<GLuint> vPartIndice;
			float cutZ = -0.2 * radius;
			for (size_t i = 0, idx = 0; i < numFace; i++, idx += 3)
			{
				GLuint indice1 = vIndice[idx];
				GLuint indice2 = vIndice[idx + 1];
				GLuint indice3 = vIndice[idx + 2];

				glm::vec3 &v1 = vVertice[indice1];
				glm::vec3 &v2 = vVertice[indice2];
				glm::vec3 &v3 = vVertice[indice3];

				if (v1.z < cutZ || v2.z < cutZ || v3.z < cutZ)
				{
					vPartIndice.push_back(indice1);
					vPartIndice.push_back(indice2);
					vPartIndice.push_back(indice3);
				}
			}

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vPartIndice, TRIANGLES);
			pVertexArray->setNormals(vVertice);
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}
		~TCubeSphere() {}

	private:

	};

	class CubeGrid
	{
	public:
		CubeGrid(int subdivison = 0)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = 1.0f;
			float a_2 = a * 0.5f;

			int numInterval = 1 << subdivison;
			int numVertice = 2 * (numInterval + 1) * (numInterval + 1) +
				(numInterval - 1) * numInterval * 4;
			int numFace = 6 * numInterval * numInterval * 2;

			vVertice.resize(numVertice);
			vIndice.resize(numFace * 3);
			vTexCoordOnFlat.resize(numFace * 3);

			{
				float aInterval = a / numInterval;
				float yFlatInterval = 1.0f / 3.0f, xFlatInterval = 1.0f / 4.0f;
				float yInterval = yFlatInterval / numInterval;
				float xInterval = xFlatInterval / numInterval;

				//Add the top cube sphere part
				{
					int vertIdx = 0, indiceIdx = 0, texcoordIdx = 0;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, a_2, a_2 };
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = a_2 - aInterval;
					float yTexStart = 1.0f - yInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, a_2, zStart };
						vertIdx++;

						xStart = -a_2 + aInterval;
						float xTexStart = xFlatInterval + xInterval;

						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, a_2, zStart };

							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart + yInterval);

							vertIdx++;
							indiceIdx += 6;
							texcoordIdx += 6;

							xStart += aInterval;
							xTexStart += xInterval;
						}

						zStart -= aInterval;
						yTexStart -= yInterval;
					}


				}

				std::vector<GLuint> vLastUpRowVertIdx(numInterval * 4);
				{
					//Left up
					for (int i = 0, vertIdx = 0;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vLastUpRowVertIdx[i] = vertIdx;
					//Front up
					for (int i = 0, vertIdx = (numInterval + 1) * numInterval;
						 i < numInterval; i++, vertIdx++)
						vLastUpRowVertIdx[i + numInterval] = vertIdx;
					//Right up
					for (int i = 0, vertIdx = (numInterval + 1) * (numInterval + 1) - 1;
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vLastUpRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back up
					for (int i = 0, vertIdx = numInterval;
						 i < numInterval; i++, vertIdx--)
						vLastUpRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the middle cube sphere parts
				//Left-->Front-->Right-->Back
				{
					int vertIdx = (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numInterval * numInterval * 2) * 3,
						texcoordIdx = (numInterval * numInterval * 2) * 3;

					if (numInterval > 1)
					{
						float yStart = a_2 - aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						vertIdx++;

						float xTexStart = xInterval, yTexStart = 1.0f - yFlatInterval - yInterval;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							zStart -= aInterval;
							xTexStart += xInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							xStart += aInterval;
							xTexStart += xInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							zStart += aInterval;
							xTexStart += xInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							xStart -= aInterval;
							xTexStart += xInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 5] = vertStartIdx;

						vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

						indiceIdx += 6;
						texcoordIdx += 6;
					}

					for (int i = 2; i < numInterval; i++)
					{
						float yStart = a_2 - float(i) * aInterval;

						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						vertIdx++;

						float xTexStart = xInterval, yTexStart = 1.0f - yFlatInterval - yInterval * i;

						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							zStart -= aInterval;
							xTexStart += xInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							xStart += aInterval;
							xTexStart += xInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							zStart += aInterval;
							xTexStart += xInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

							indiceIdx += 6;
							texcoordIdx += 6;
							vertIdx++;

							xStart -= aInterval;
							xTexStart += xInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 2] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 5] = vertStartIdx;

						vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart - xInterval, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart - xInterval, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart);

						indiceIdx += 6;
						texcoordIdx += 6;
					}

					if (numInterval > 1)
					{
						int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1)
							- numInterval * 4;
						for (int i = 0; i < numInterval * 4; i++)
						{
							vLastUpRowVertIdx[i] = vertIdx;
							vertIdx++;
						}
					}
				}

				//Add the bottom cube sphere part
				{
					int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numFace - numInterval * numInterval * 2) * 3,
						texcoordIdx = (numFace - numInterval * numInterval * 2) * 3;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, -a_2, -a_2 };
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = -a_2 + aInterval;
					float yTexStart = yFlatInterval - yInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, -a_2, zStart };
						vertIdx++;

						xStart = -a_2 + aInterval;
						float xTexStart = xFlatInterval + xInterval;

						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, -a_2, zStart };

							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart - xInterval, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart, yTexStart);
							vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart - xInterval, yTexStart + yInterval);
							vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart, yTexStart + yInterval);

							vertIdx++;
							indiceIdx += 6;
							texcoordIdx += 6;

							xStart += aInterval;
							xTexStart += xInterval;
						}

						zStart += aInterval;
						yTexStart -= yInterval;
					}
				}

				std::vector<GLuint> vDownRowVertIdx(numInterval * 4);
				{
					//Left down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1);
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vDownRowVertIdx[i] = vertIdx;
					//Front down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * (numInterval + 1);
						 i < numInterval; i++, vertIdx++)
						vDownRowVertIdx[i + numInterval] = vertIdx;
					//Right down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * numInterval - 1;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vDownRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back down
					for (int i = 0, vertIdx = numVertice - 1;
						 i < numInterval; i++, vertIdx--)
						vDownRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the faces between the last up and the Down row vertices
				{
					int indiceIdx = (numFace - (numInterval + 4) * numInterval * 2) * 3;
					int texcoordIdx = (numFace - (numInterval + 4) * numInterval * 2) * 3;

					float xTexStart = 0, yTexStart = yFlatInterval;

					int i = 0;
					for (; i < numInterval * 4 - 1; i++)
					{
						vIndice[indiceIdx] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 5] = vDownRowVertIdx[i + 1];

						vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart + xInterval, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart, yTexStart);
						vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart + xInterval, yTexStart + yInterval);
						vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart + xInterval, yTexStart);

						indiceIdx += 6;
						texcoordIdx += 6;

						xTexStart += xInterval;
					}

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];

					vTexCoordOnFlat[texcoordIdx] = glm::vec2(xTexStart, yTexStart);
					vTexCoordOnFlat[texcoordIdx + 1] = glm::vec2(xTexStart, yTexStart + yInterval);
					vTexCoordOnFlat[texcoordIdx + 2] = glm::vec2(xTexStart + xInterval, yTexStart + yInterval);
					vTexCoordOnFlat[texcoordIdx + 3] = glm::vec2(xTexStart, yTexStart);
					vTexCoordOnFlat[texcoordIdx + 4] = glm::vec2(xTexStart + xInterval, yTexStart + yInterval);
					vTexCoordOnFlat[texcoordIdx + 5] = glm::vec2(xTexStart + xInterval, yTexStart);
				}

			}
		}
		~CubeGrid() {}

		std::vector<glm::vec3> getSphereVertice(float radius = 1.0f)
		{
			std::vector<glm::vec3> vSphereVertice(vVertice.size());

			for (size_t i = 0; i < vVertice.size(); i++)
			{
				vSphereVertice[i] = glm::normalize(vVertice[i]) * radius;
			}

			return vSphereVertice;
		}

		std::vector<glm::vec3> getFlatVertice()
		{
			std::vector<glm::vec3> vFlatVertice(vIndice.size());

			for (size_t i = 0; i < vIndice.size(); i++)
			{
				vFlatVertice[i] = vVertice[vIndice[i]];
			}

			return vFlatVertice;
		}

		std::vector<glm::vec3> getFlatSphereVertice(float radius = 1.0f)
		{
			std::vector<glm::vec3> vFlatSphereVertice(vIndice.size());

			for (size_t i = 0; i < vIndice.size(); i++)
			{
				vFlatSphereVertice[i] = glm::normalize(vVertice[vIndice[i]]) * radius;
			}

			return vFlatSphereVertice;
		}

		std::vector<glm::vec3> vVertice;
		std::vector<GLuint> vIndice;
		std::vector<glm::vec2> vTexCoordOnFlat;
	};

	class TZAxisPlane : public TMesh
	{
	public:
		TZAxisPlane(float a, float b, float z, glm::vec4 color)
		{
			float a_2 = a * 0.5, b_2 = b * 0.5;
			std::vector<glm::vec3> vVertice =
			{
				{ a_2, b_2, z },
				{ a_2, -b_2, z },
				{ -a_2, -b_2, z },
				{ -a_2, b_2, z }
			};

			std::vector<glm::vec3> vNormal =
			{
				{ 0.0f, 0.0f, -1.0f },
				{ 0.0f, 0.0f, -1.0f },
				{ 0.0f, 0.0f, -1.0f },
				{ 0.0f, 0.0f, -1.0f }
			};

			std::vector<GLuint> vIndice =
			{
				0, 1, 2,
				0, 2, 3
			};

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vVertice, vIndice, TRIANGLES);
			pVA->setNormals(vNormal);

			std::shared_ptr<Material> pMat =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setVertexArray(pVA);
			pVA->addInstance();
			setMaterial(pMat);
		}
		~TZAxisPlane() {}

	private:

	};

	class ZAxisPlaneGrid : public Mesh
	{
	public:
		ZAxisPlaneGrid(float a, float b, float z, glm::vec4 color, int subdivision = 0)
		{
			float a_2 = a * 0.5, b_2 = b * 0.5;
			int numInterval = 1 << subdivision;
			int numVertice = numInterval * 4;
			int numSegment = (numInterval + 1) * 2;
			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numSegment * 2);

			float xInterval = a / numInterval;
			float yInterval = b / numInterval;

			//Add vertices on the two row and col segment
			float xStart = -a_2;
			int vertIdx = 0, indiceIdx = 0;
			for (size_t i = 0; i <= numInterval; i++)
			{
				glm::vec3 v1 = { xStart, b_2, z };
				glm::vec3 v2 = { xStart, -b_2, z };
				vVertice[vertIdx] = v1;
				vVertice[vertIdx + 1] = v2;

				vIndice[indiceIdx] = vertIdx;
				vIndice[indiceIdx + 1] = vertIdx + 1;

				vertIdx += 2;
				indiceIdx += 2;
				xStart += xInterval;
			}
			vIndice[numSegment] = 0;
			vIndice[numSegment + 1] = numSegment - 2;

			float yStart = b_2 - yInterval;
			vertIdx = numSegment, indiceIdx = numSegment + 2;
			for (size_t i = 1; i < numInterval; i++)
			{
				glm::vec3 v1 = { -a_2, yStart, z };
				glm::vec3 v2 = { a_2, yStart, z };
				vVertice[vertIdx] = v1;
				vVertice[vertIdx + 1] = v2;

				vIndice[indiceIdx] = vertIdx;
				vIndice[indiceIdx + 1] = vertIdx + 1;

				vertIdx += 2;
				indiceIdx += 2;
				yStart -= yInterval;
			}

			vIndice[vIndice.size() - 2] = 1;
			vIndice[vIndice.size() - 1] = numSegment - 1;

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vVertice, vIndice, LINES);

			std::shared_ptr<Material> pMat =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setVertexArray(pVA);
			pVA->addInstance();
			setMaterial(pMat);
		}
		~ZAxisPlaneGrid() {}

	private:

	};

	class CubeSphereGrid : public GridMesh
	{
	public:
		CubeSphereGrid(float radius, glm::vec4 color, int subdivison = 0)
			//: CubeSphere(radius, color, subdivison)
		{
			//icosahedron parameters a, b for the rectangle height and width
			float a = 2.0f * std::sqrtf(3.0f) * radius / 3.0f;
			float a_2 = a * 0.5f;
			std::vector<glm::vec3> vCubeVertice =
			{
				//up rectangle
				{ a_2, a_2, a_2 },
				{ a_2, a_2, -a_2 },
				{ -a_2, a_2, -a_2 },
				{ -a_2, a_2, a_2 },

				//down rectangle
				{ a_2, -a_2, a_2 },
				{ a_2, -a_2, -a_2 },
				{ -a_2, -a_2, -a_2 },
				{ -a_2, -a_2, a_2 }
			};

			std::vector<GLuint> vCubeIndice =
			{
				0, 1, 2,/*  */0, 2, 3,
				////////////////////////y-positive up
				0, 7, 4,/*  */0, 3, 7,
				////////////////////////z-positive back 
				0, 4, 5,/*  */0, 5, 1,
				////////////////////////x-positive right
				3, 6, 7,/*  */3, 2, 6,
				////////////////////////x-negative left
				1, 5, 6,/*  */1, 6, 2,
				////////////////////////z-negative front
				4, 6, 5,/*  */4, 7, 6
				////////////////////////y-negative down 
			};

			int numInterval = 1 << subdivison;
			int numVertice = 2 * (numInterval + 1) * (numInterval + 1) +
				(numInterval - 1) * numInterval * 4;
			int numFace = 6 * numInterval * numInterval * 2;

			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 3);

			{
				float aInterval = a / numInterval;

				//Add the top cube sphere part
				{
					int vertIdx = 0, indiceIdx = 0;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, a_2, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = a_2 - aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart -= aInterval;
					}


				}

				std::vector<GLuint> vLastUpRowVertIdx(numInterval * 4);
				{
					//Left up
					for (int i = 0, vertIdx = 0;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vLastUpRowVertIdx[i] = vertIdx;
					//Front up
					for (int i = 0, vertIdx = (numInterval + 1) * numInterval;
						 i < numInterval; i++, vertIdx++)
						vLastUpRowVertIdx[i + numInterval] = vertIdx;
					//Right up
					for (int i = 0, vertIdx = (numInterval + 1) * (numInterval + 1) - 1;
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vLastUpRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back up
					for (int i = 0, vertIdx = numInterval;
						 i < numInterval; i++, vertIdx--)
						vLastUpRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the middle cube sphere parts
				//Left-->Front-->Right-->Back
				{
					int vertIdx = (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numInterval * numInterval * 2) * 3;

					if (numInterval > 1)
					{
						float yStart = a_2 - aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
							vIndice[indiceIdx + 2] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vLastUpRowVertIdx[vertIdx - vertStartIdx];
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[vertIdx - vertStartIdx - 1];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;
					}

					for (int i = 2; i < numInterval; i++)
					{
						float yStart = a_2 - float(i) * aInterval;
						int vertStartIdx = vertIdx;
						vVertice[vertIdx] = { -a_2, yStart, a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						//Left
						float zStart = a_2 - aInterval, xStart = -a_2;
						for (int i = 1; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart -= aInterval;
						}
						//Front
						zStart = -a_2, xStart = -a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart += aInterval;
						}
						//Right
						zStart = -a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							zStart += aInterval;
						}
						//Back
						zStart = a_2, xStart = a_2;
						for (int i = 0; i < numInterval; i++)
						{
							vVertice[vertIdx] = { xStart, yStart, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);

							vIndice[indiceIdx] = vertIdx - 1;
							vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
							vIndice[indiceIdx + 2] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 3] = vertIdx - 1;
							vIndice[indiceIdx + 4] = vertIdx - numInterval * 4;
							vIndice[indiceIdx + 5] = vertIdx;

							indiceIdx += 6;
							vertIdx++;

							xStart -= aInterval;
						}

						vIndice[indiceIdx] = vertIdx - 1;
						vIndice[indiceIdx + 1] = vertIdx - 1 - numInterval * 4;
						vIndice[indiceIdx + 2] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 3] = vertIdx - 1;
						vIndice[indiceIdx + 4] = vertStartIdx - numInterval * 4;
						vIndice[indiceIdx + 5] = vertStartIdx;
						indiceIdx += 6;

						yStart -= aInterval;
					}

					if (numInterval > 1)
					{
						int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1)
							- numInterval * 4;
						for (int i = 0; i < numInterval * 4; i++)
						{
							vLastUpRowVertIdx[i] = vertIdx;
							vertIdx++;
						}
					}
				}

				//Add the bottom cube sphere part
				{
					int vertIdx = numVertice - (numInterval + 1) * (numInterval + 1),
						indiceIdx = (numFace - numInterval * numInterval * 2) * 3;

					float xStart = -a_2;
					for (int i = 0; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { xStart, -a_2, -a_2 };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;
						xStart += aInterval;
					}

					float zStart = -a_2 + aInterval;
					for (int i = 1; i <= numInterval; i++)
					{
						vVertice[vertIdx] = { -a_2, -a_2, zStart };
						//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
						vertIdx++;

						xStart = -a_2 + aInterval;
						for (int j = 1; j <= numInterval; j++)
						{
							vVertice[vertIdx] = { xStart, -a_2, zStart };
							//vVertice[vertIdx] = glm::normalize(vVertice[vertIdx]);
							vIndice[indiceIdx] = vertIdx;
							vIndice[indiceIdx + 1] = vertIdx - 1;
							vIndice[indiceIdx + 2] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 3] = vertIdx;
							vIndice[indiceIdx + 4] = vertIdx - numInterval - 2;
							vIndice[indiceIdx + 5] = vertIdx - numInterval - 1;

							vertIdx++;
							indiceIdx += 6;

							xStart += aInterval;
						}

						zStart += aInterval;
					}
				}

				std::vector<GLuint> vDownRowVertIdx(numInterval * 4);
				{
					//Left down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1);
						 i < numInterval; i++, vertIdx -= (numInterval + 1))
						vDownRowVertIdx[i] = vertIdx;
					//Front down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * (numInterval + 1);
						 i < numInterval; i++, vertIdx++)
						vDownRowVertIdx[i + numInterval] = vertIdx;
					//Right down
					for (int i = 0, vertIdx = numVertice - (numInterval + 1) * numInterval - 1;
						 i < numInterval; i++, vertIdx += (numInterval + 1))
						vDownRowVertIdx[i + numInterval * 2] = vertIdx;
					//Back down
					for (int i = 0, vertIdx = numVertice - 1;
						 i < numInterval; i++, vertIdx--)
						vDownRowVertIdx[i + numInterval * 3] = vertIdx;
				}

				//Add the faces between the last up and the Down row vertices
				{
					int indiceIdx = (numFace - (numInterval + 4) * numInterval * 2) * 3;
					int i = 0;
					for (; i < numInterval * 4 - 1; i++)
					{
						vIndice[indiceIdx] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
						vIndice[indiceIdx + 2] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
						vIndice[indiceIdx + 4] = vLastUpRowVertIdx[i + 1];
						vIndice[indiceIdx + 5] = vDownRowVertIdx[i + 1];
						indiceIdx += 6;
					}

					vIndice[indiceIdx] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 1] = vLastUpRowVertIdx[i];
					vIndice[indiceIdx + 2] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 3] = vDownRowVertIdx[i];
					vIndice[indiceIdx + 4] = vLastUpRowVertIdx[0];
					vIndice[indiceIdx + 5] = vDownRowVertIdx[0];
				}

			}

			std::vector<glm::vec4> vColor;
			std::for_each(vVertice.begin(), vVertice.end(), [&](glm::vec3 &vert)
			{
				vert = glm::normalize(vert);
				glm::vec3 vertColor = 0.5f * (vert + glm::vec3(1.0f));
				//glm::vec3 color =  vert;
				vColor.push_back(glm::vec4(vertColor, color.w));

				vert *= radius;
			});

			std::vector<GLuint> vPartIndice;
			float cutZ = -0.2 * radius;
			for (size_t i = 0, idx = 0; i < numFace; i++, idx += 3)
			{
				GLuint indice1 = vIndice[idx];
				GLuint indice2 = vIndice[idx + 1];
				GLuint indice3 = vIndice[idx + 2];

				glm::vec3 &v1 = vVertice[indice1];
				glm::vec3 &v2 = vVertice[indice2];
				glm::vec3 &v3 = vVertice[indice3];

				if (v1.z < cutZ && v2.z < cutZ && v3.z < cutZ)
				{
					vPartIndice.push_back(indice1);
					vPartIndice.push_back(indice2);
					vPartIndice.push_back(indice3);
				}
			}

			std::shared_ptr<VertexArray> pVertexArray = std::make_shared<VertexArray>(
				vVertice, vPartIndice, TRIANGLES);
			//pVertexArray->setNormals(vVertice);
			//pVertexArray->setColors(vColor);

			std::shared_ptr<Material> pMaterial =
				std::make_shared<Material>(color, glm::vec4(0.0f));

			setMaterial(pMaterial);
			setVertexArray(pVertexArray);
			addInstance();
		}
		~CubeSphereGrid() {}
	};

	class TCylinder : public TMesh
	{
	public:
		TCylinder(float radius, float height, glm::vec4 color = glm::vec4(1.0f),
				  int subDivAngle = 0, int subDivHeight = 0) 
		{
			int numAngleInterval = 4 << subDivAngle;
			int numHeightInterval = 1 << subDivHeight;
			float h_2 = height * 0.5;
			int numVertice = numAngleInterval * (numHeightInterval + 1);
			int numFace = numAngleInterval * numHeightInterval * 2;
			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 2 * 3);

			float angleInterval = glm::two_pi<float>() / numAngleInterval;
			float yInterval = height / numHeightInterval;

			float angleOffset = 0.0f, yOffset = -h_2;
			for (size_t i = 0, vertIdx = 0; i <= numHeightInterval; i++)
			{
				angleOffset = 0.0f;
				for (size_t j = 0; j < numAngleInterval; j++)
				{
					vVertice[vertIdx] = { cos(angleOffset) * radius, yOffset, -sin(angleOffset) * radius };
					angleOffset += angleInterval;
					vertIdx++;
				}
				yOffset += yInterval;
			}

			for (size_t i = 0, vertIdx = 1, indiceIdx = 0; i < numHeightInterval; i++)
			{
				for (size_t j = 1; j < numAngleInterval; j++)
				{
					vIndice[indiceIdx] = vertIdx;
					vIndice[indiceIdx + 1] = vertIdx + numAngleInterval;
					vIndice[indiceIdx + 2] = vertIdx - 1;
					vIndice[indiceIdx + 3] = vertIdx - 1;
					vIndice[indiceIdx + 4] = vertIdx + numAngleInterval;
					vIndice[indiceIdx + 5] = vertIdx - 1 + numAngleInterval ;

					indiceIdx += 6;
					vertIdx++;
				}
				vIndice[indiceIdx] = vertIdx - 1;
				vIndice[indiceIdx + 1] = vertIdx - numAngleInterval;
				vIndice[indiceIdx + 2] = vertIdx;
				vIndice[indiceIdx + 3] = vertIdx - 1;
				vIndice[indiceIdx + 4] = vertIdx;
				vIndice[indiceIdx + 5] = vertIdx - 1 + numAngleInterval;

				indiceIdx += 6;
				vertIdx++;
			}

			std::vector<GLuint> vPartIndice;
			float cutZ = -0.2 * radius;
			for (size_t i = 0, idx = 0; i < numFace; i++, idx += 3)
			{
				GLuint indice1 = vIndice[idx];
				GLuint indice2 = vIndice[idx + 1];
				GLuint indice3 = vIndice[idx + 2];

				glm::vec3 &v1 = vVertice[indice1];
				glm::vec3 &v2 = vVertice[indice2];
				glm::vec3 &v3 = vVertice[indice3];

				if (v1.z < cutZ || v2.z < cutZ || v3.z < cutZ)
				{
					vPartIndice.push_back(indice1);
					vPartIndice.push_back(indice2);
					vPartIndice.push_back(indice3);
				}
			}

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vVertice, vPartIndice,
											  PrimitiveType::TRIANGLES);
			std::vector<glm::vec3> vNormal(numVertice);
			for (size_t i = 0; i < numVertice; i++)
			{
				vNormal[i] = glm::vec3(vVertice[i].x, 0.0f, vVertice[i].z);
			}
			pVA->setNormals(vNormal);
			pVA->addInstance();

			std::shared_ptr<Material> pMat = std::make_shared<Material>();

			setVertexArray(pVA);
			setMaterial(pMat);
		}
		~TCylinder() {}
	};

	class CylinderGrid : public GridMesh
	{
	public:
		CylinderGrid(float radius, float height, glm::vec4 color = glm::vec4(1.0f),
				  int subDivAngle = 0, int subDivHeight = 0)
		{
			int numAngleInterval = 4 << subDivAngle;
			int numHeightInterval = 1 << subDivHeight;
			float h_2 = height * 0.5;
			int numVertice = numAngleInterval * (numHeightInterval + 1);
			int numFace = numAngleInterval * numHeightInterval * 2;
			std::vector<glm::vec3> vVertice(numVertice);
			std::vector<GLuint> vIndice(numFace * 3);

			float angleInterval = glm::two_pi<float>() / numAngleInterval;
			float yInterval = height / numHeightInterval;

			float angleOffset = 0.0f, yOffset = -h_2;
			for (size_t i = 0, vertIdx = 0; i <= numHeightInterval; i++)
			{
				angleOffset = 0.0f;
				for (size_t j = 0; j < numAngleInterval; j++)
				{
					vVertice[vertIdx] = { cos(angleOffset) * radius, yOffset, -sin(angleOffset) * radius };
					angleOffset += angleInterval;
					vertIdx++;
				}
				yOffset += yInterval;
			}

			for (size_t i = 0, vertIdx = 1, indiceIdx = 0; i < numHeightInterval; i++)
			{
				for (size_t j = 1; j < numAngleInterval; j++)
				{
					vIndice[indiceIdx] = vertIdx;
					vIndice[indiceIdx + 1] = vertIdx + numAngleInterval;
					vIndice[indiceIdx + 2] = vertIdx - 1;
					vIndice[indiceIdx + 3] = vertIdx - 1;
					vIndice[indiceIdx + 4] = vertIdx + numAngleInterval;
					vIndice[indiceIdx + 5] = vertIdx - 1 + numAngleInterval;

					indiceIdx += 6;
					vertIdx++;
				}
				vIndice[indiceIdx] = vertIdx - 1;
				vIndice[indiceIdx + 1] = vertIdx - numAngleInterval;
				vIndice[indiceIdx + 2] = vertIdx;
				vIndice[indiceIdx + 3] = vertIdx - 1;
				vIndice[indiceIdx + 4] = vertIdx;
				vIndice[indiceIdx + 5] = vertIdx - 1 + numAngleInterval;

				indiceIdx += 6;
				vertIdx++;
			}

			std::vector<GLuint> vPartIndice;
			float cutZ = -0.2 * radius;
			for (size_t i = 0, idx = 0; i < numFace; i++, idx += 3)
			{
				GLuint indice1 = vIndice[idx];
				GLuint indice2 = vIndice[idx + 1];
				GLuint indice3 = vIndice[idx + 2];

				glm::vec3 &v1 = vVertice[indice1];
				glm::vec3 &v2 = vVertice[indice2];
				glm::vec3 &v3 = vVertice[indice3];

				if (v1.z < cutZ || v2.z < cutZ || v3.z < cutZ)
				{
					vPartIndice.push_back(indice1);
					vPartIndice.push_back(indice2);
					vPartIndice.push_back(indice3);
				}
			}

			std::shared_ptr<VertexArray> pVA =
				std::make_shared<VertexArray>(vVertice, vPartIndice,
											  PrimitiveType::TRIANGLES);
			std::vector<glm::vec3> vNormal(numVertice);
			for (size_t i = 0; i < numVertice; i++)
			{
				vNormal[i] = glm::vec3(vVertice[i].x, 0.0f, vVertice[i].z);
			}
			//pVA->setNormals(vNormal);
			pVA->addInstance();

			std::shared_ptr<Material> pMat = std::make_shared<Material>();

			setVertexArray(pVA);
			setMaterial(pMat);
		}
		~CylinderGrid() {}
	};
}
