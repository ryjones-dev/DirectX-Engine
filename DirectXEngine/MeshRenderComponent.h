#pragma once
#include "RenderComponent.h"

class MeshRenderComponent : public RenderComponent
{
public:
	MeshRenderComponent(Scene* scene, unsigned int entity);
	~MeshRenderComponent();

	Mesh* getMesh() const;
	void changeMesh(Mesh* mesh);

private:
	Mesh* m_mesh;
};