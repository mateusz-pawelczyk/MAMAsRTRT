#pragma once
#include <DirectXMath.h>
using Microsoft::WRL::ComPtr;

struct Vertex {
	DirectX::XMFLOAT3 position; // 3D position
	DirectX::XMFLOAT4 color;    // RGBA color
	DirectX::XMFLOAT3 normal;   // Normal vector
	DirectX::XMFLOAT2 texcoord; // Texture coordinates
	DirectX::XMFLOAT3 tangent;  // Tangent vector
	DirectX::XMFLOAT3 binormal; // Binormal vector
	Vertex() : color({ 1.0, 0.5, 0.7, 1.0 }) {}
	// #DXR Extra: Indexed Geometry
	Vertex(DirectX::XMFLOAT4 pos, DirectX::XMFLOAT4 /*n*/, DirectX::XMFLOAT4 col)
		: position(pos.x, pos.y, pos.z), color(col) {}
	Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 col) : position(pos), color(col) {}
	Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 col, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 tangent, DirectX::XMFLOAT3 binormal) : position(pos), color(col), normal(normal), texcoord(texcoord), tangent(tangent), binormal(binormal) {}
};

struct AccelerationStructureBuffers {
	ComPtr<ID3D12Resource> pScratch;      // Scratch memory for AS builder
	ComPtr<ID3D12Resource> pResult;       // Where the AS is
	ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances
	UINT64 resultSizeInBytes = 0;
	UINT64 scratchSizeInBytes = 0;
};