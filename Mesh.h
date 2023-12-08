#pragma once


#include "BaseObjectClass.h"
#include <vector>
#include "ObjectStructs.h"


// Mesh class inherits from BaseObjectClass
class Mesh : public BaseObjectClass {
private:
	


	// methods
	void createVertexBuffer(std::vector<Vertex> vertices);
	void createIndexBuffer(std::vector<uint32_t> indices);

	void setGeometryDescription() override;
public:
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, ComPtr<ID3D12Device5> device);
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	UINT indexCount;
	UINT vertexCount;
};


