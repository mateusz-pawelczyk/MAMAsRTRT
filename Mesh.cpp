#include "stdafx.h"
#include "Mesh.h"

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}
UINT Mesh::meshInstanceCount = 0;

void Mesh::addDerivedInstance() { meshInstanceCount++; }


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, ComPtr<ID3D12Device5> new_device)
{
	m_device = new_device;
	createVertexBuffer(vertices); 
	createIndexBuffer(indices);

	setGeometryDescription();
}




void Mesh::createVertexBuffer(std::vector<Vertex> vertices)
{
	const UINT sphereVBSize =
		static_cast<UINT>(vertices.size()) * sizeof(Vertex);

	/// TODO: make more efficient (https://chat.openai.com/g/g-t8o06eKD1-real-time-ray-tracing-master/c/19b79424-f3af-478c-b928-51a6aca5b03e)
	// Note: using upload heaps to transfer static data like vert buffers is not
	// recommended. Every time the GPU needs it, the upload heap will be
	// marshalled over. Please read up on Default Heap usage. An upload heap is
	// used here for code simplicity and because there are very few verts to
	// actually transfer.
	CD3DX12_HEAP_PROPERTIES heapProperty =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferResource =
		CD3DX12_RESOURCE_DESC::Buffer(sphereVBSize);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperty, D3D12_HEAP_FLAG_NONE, &bufferResource, //
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)));
	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(
		0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(vertexBuffer->Map(
		0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices.data(), sphereVBSize);
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sphereVBSize;

	vertexCount = static_cast<UINT>(vertices.size());
}

void Mesh::createIndexBuffer(std::vector<uint32_t> indices)
{
	const UINT sphereIBSize = static_cast<UINT>(indices.size()) * sizeof(UINT);

	/// TODO: make more efficient (https://chat.openai.com/g/g-t8o06eKD1-real-time-ray-tracing-master/c/19b79424-f3af-478c-b928-51a6aca5b03e)
	// Note: using upload heaps to transfer static data like vert buffers is not
	// recommended. Every time the GPU needs it, the upload heap will be
	// marshalled over. Please read up on Default Heap usage. An upload heap is
	// used here for code simplicity and because there are very few verts to
	// actually transfer.
	CD3DX12_HEAP_PROPERTIES heapProperty =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferResource =
		CD3DX12_RESOURCE_DESC::Buffer(sphereIBSize);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperty, D3D12_HEAP_FLAG_NONE, &bufferResource, //
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer)));

	// Copy the triangle data to the index buffer.
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(
		0, 0); // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(indexBuffer->Map(0, &readRange,
		reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indices.data(), sphereIBSize);
	indexBuffer->Unmap(0, nullptr);

	// Initialize the index buffer view.
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sphereIBSize;

	indexCount = static_cast<UINT>(indices.size());
}

void Mesh::setGeometryDescription()
{
	m_geometryDescription.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_geometryDescription.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
	m_geometryDescription.Triangles.IndexCount = indexCount;
	m_geometryDescription.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
	m_geometryDescription.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	m_geometryDescription.Triangles.VertexCount = vertexCount;
	m_geometryDescription.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
	m_geometryDescription.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	m_geometryDescription.Flags = m_visibility ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
	// additional option: 
	// m_geometryDescription.Triangles.Transform3x4: transforming the vertices once when building. Is 3x4 because the 4th row is always [0,0,0,1] so we can leave it out for more efficiency

}