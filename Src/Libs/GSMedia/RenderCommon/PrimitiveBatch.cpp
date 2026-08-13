//--------------------------------------------------------------------------------------
// File: PrimitiveBatch.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "PrimitiveBatch.h"

#include "DXTrace.h"
#include "d3dUtil.h"

using namespace DirectX;
using namespace DirectX::Internal;
using namespace std;
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


// Internal PrimitiveBatch implementation class.
class PrimitiveBatchBase::Impl
{
public:
    Impl(_In_ ID3D11DeviceContext* deviceContext, size_t maxIndices, size_t maxVertices, size_t vertexSize);

    void Begin();
    void End();

    void Draw(D3D11_PRIMITIVE_TOPOLOGY topology, bool isIndexed, _In_opt_count_(indexCount) uint16_t const* indices, size_t indexCount, size_t vertexCount, _Out_ void** pMappedVertices);

private:
    void FlushBatch();

    ComPtr<ID3D11DeviceContext> mDeviceContext;
    ComPtr<ID3D11Buffer> mIndexBuffer;
    ComPtr<ID3D11Buffer> mVertexBuffer;

    size_t mMaxIndices;
    size_t mMaxVertices;
    size_t mVertexSize;

    D3D11_PRIMITIVE_TOPOLOGY mCurrentTopology;
    bool mInBeginEndPair;
    bool mCurrentlyIndexed;

    size_t mCurrentIndex;
    size_t mCurrentVertex;
    
    size_t mBaseIndex;
    size_t mBaseVertex;

    D3D11_MAPPED_SUBRESOURCE mMappedIndices;
    D3D11_MAPPED_SUBRESOURCE mMappedVertices;
};


namespace
{
    // Helper for creating a D3D vertex or index buffer.
    void CreateDynamicBuffer(_In_ ID3D11Device* device, uint32_t bufferSize, D3D11_BIND_FLAG bindFlag, _Outptr_ ID3D11Buffer** pBuffer)
    {
        D3D11_BUFFER_DESC desc = {};

        desc.ByteWidth = bufferSize;
        desc.BindFlags = bindFlag;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        
		HR(device->CreateBuffer(&desc, nullptr, pBuffer));

        assert(pBuffer != nullptr && *pBuffer != nullptr);
        _Analysis_assume_(pBuffer != nullptr && *pBuffer != nullptr);

		D3D11SetDebugObjectName(*pBuffer, "DirectXTK:PrimitiveBatch");
    }
}


// Constructor.
PrimitiveBatchBase::Impl::Impl(_In_ ID3D11DeviceContext* deviceContext, size_t maxIndices, size_t maxVertices, size_t vertexSize)
  : mMaxIndices(maxIndices),
    mMaxVertices(maxVertices),
    mVertexSize(vertexSize),
    mCurrentTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED),
    mInBeginEndPair(false),
    mCurrentlyIndexed(false),
    mCurrentIndex(0),
    mCurrentVertex(0),
    mBaseIndex(0),
    mBaseVertex(0),
    mMappedIndices{},
    mMappedVertices{}
{
    ComPtr<ID3D11Device> device;
    deviceContext->GetDevice(&device);

    if (!maxVertices)
        throw std::exception("maxVertices must be greater than 0");

    if (vertexSize > D3D11_REQ_MULTI_ELEMENT_STRUCTURE_SIZE_IN_BYTES)
        throw std::exception("Vertex size is too large for DirectX 11");

    uint64_t ibBytes = uint64_t(maxIndices) * sizeof(uint16_t);
    if (ibBytes > uint64_t(D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM * 1024u * 1024u)
        || ibBytes > UINT32_MAX)
        throw std::exception("IB too large for DirectX 11");

    uint64_t vbBytes = uint64_t(maxVertices) * uint64_t(vertexSize);
    if (vbBytes > uint64_t(D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM * 1024u * 1024u)
        || vbBytes > UINT32_MAX)
        throw std::exception("VB too large for DirectX 11");

    mDeviceContext = deviceContext;

    // If you only intend to draw non-indexed geometry, specify maxIndices = 0 to skip creating the index buffer.
    if (maxIndices > 0)
    {
        CreateDynamicBuffer(device.Get(), static_cast<uint32_t>(ibBytes), D3D11_BIND_INDEX_BUFFER, &mIndexBuffer);
    }

    // Create the vertex buffer.
    CreateDynamicBuffer(device.Get(), static_cast<uint32_t>(vbBytes), D3D11_BIND_VERTEX_BUFFER, &mVertexBuffer);
}


// Begins a batch of primitive drawing operations.
void PrimitiveBatchBase::Impl::Begin()
{
    if (mInBeginEndPair)
        throw std::exception("Cannot nest Begin calls");

    // Bind the index buffer.
    if (mMaxIndices > 0)
    {
        mDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    }

    // Bind the vertex buffer.
    auto vertexBuffer = mVertexBuffer.Get();
    UINT vertexStride = static_cast<UINT>(mVertexSize);
    UINT vertexOffset = 0;

    mDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);
     
    // If this is a deferred D3D context, reset position so the first Map calls will use D3D11_MAP_WRITE_DISCARD.
    if (mDeviceContext->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED)
    {
        mCurrentIndex = 0;
        mCurrentVertex = 0;
    }

    mInBeginEndPair = true;
}


// Ends a batch of primitive drawing operations.
void PrimitiveBatchBase::Impl::End()
{
    if (!mInBeginEndPair)
        throw std::exception("Begin must be called before End");

    FlushBatch();

    mInBeginEndPair = false;
}


namespace
{
    // Can we combine adjacent primitives using this topology into a single draw call?
    bool CanBatchPrimitives(D3D11_PRIMITIVE_TOPOLOGY topology) noexcept
    {
        switch (topology)
        {
        case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:
        case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            // Lists can easily be merged.
            return true;

        default:
            // Strips cannot.
            return false;
        }

        // We could also merge indexed strips by inserting degenerates,
        // but that's not always a perf win, so let's keep things simple.
    }

	// Helper for locking a vertex or index buffer.
	void LockBuffer(_In_ ID3D11DeviceContext* deviceContext, _In_ ID3D11Buffer* buffer, size_t currentPosition, _Out_ size_t* basePosition, _Out_ D3D11_MAPPED_SUBRESOURCE* mappedResource)
	{
		D3D11_MAP mapType = (currentPosition == 0) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;

		HR(deviceContext->Map(buffer, 0, mapType, 0, mappedResource));

		*basePosition = currentPosition;
	}
}


// Adds new geometry to the batch.
_Use_decl_annotations_
void PrimitiveBatchBase::Impl::Draw(D3D11_PRIMITIVE_TOPOLOGY topology, bool isIndexed, uint16_t const* indices, size_t indexCount, size_t vertexCount, void** pMappedVertices)
{
    if (isIndexed && !indices)
        throw std::exception("Indices cannot be null");

    if (indexCount >= mMaxIndices)
        throw std::exception("Too many indices");

    if (vertexCount >= mMaxVertices)
        throw std::exception("Too many vertices");

    if (!mInBeginEndPair)
        throw std::exception("Begin must be called before Draw");

    // Can we merge this primitive in with an existing batch, or must we flush first?
    bool wrapIndexBuffer = (mCurrentIndex + indexCount > mMaxIndices);
    bool wrapVertexBuffer = (mCurrentVertex + vertexCount > mMaxVertices);

    if ((topology != mCurrentTopology) ||
        (isIndexed != mCurrentlyIndexed) ||
        !CanBatchPrimitives(topology) ||
        wrapIndexBuffer || wrapVertexBuffer)
    {
        FlushBatch();
    }

    if (wrapIndexBuffer)
        mCurrentIndex = 0;

    if (wrapVertexBuffer)
        mCurrentVertex = 0;

    // If we are not already in a batch, lock the buffers.
    if (mCurrentTopology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
    {
        if (isIndexed)
        {
            LockBuffer(mDeviceContext.Get(), mIndexBuffer.Get(), mCurrentIndex, &mBaseIndex, &mMappedIndices);
        }

        LockBuffer(mDeviceContext.Get(), mVertexBuffer.Get(), mCurrentVertex, &mBaseVertex, &mMappedVertices);

        mCurrentTopology = topology;
        mCurrentlyIndexed = isIndexed;
    }
    
    // Copy over the index data.
    if (isIndexed)
    {
        auto outputIndices = static_cast<uint16_t*>(mMappedIndices.pData) + mCurrentIndex;
        
        for (size_t i = 0; i < indexCount; i++)
        {
            outputIndices[i] = static_cast<uint16_t>(indices[i] + mCurrentVertex - mBaseVertex);
        }
 
        mCurrentIndex += indexCount;
    }

    // Return the output vertex data location.
    *pMappedVertices = static_cast<uint8_t*>(mMappedVertices.pData) + (mCurrentVertex * mVertexSize);

    mCurrentVertex += vertexCount;
}


// Sends queued primitives to the graphics device.
void PrimitiveBatchBase::Impl::FlushBatch()
{
    // Early out if there is nothing to flush.
    if (mCurrentTopology == D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
        return;

    mDeviceContext->IASetPrimitiveTopology(mCurrentTopology);

    mDeviceContext->Unmap(mVertexBuffer.Get(), 0);

    if (mCurrentlyIndexed)
    {
        // Draw indexed geometry.
        mDeviceContext->Unmap(mIndexBuffer.Get(), 0);

        mDeviceContext->DrawIndexed(
            static_cast<UINT>(mCurrentIndex - mBaseIndex),
            static_cast<UINT>(mBaseIndex),
            static_cast<INT>(mBaseVertex));
    }
    else
    {
        // Draw non-indexed geometry.
        mDeviceContext->Draw(static_cast<UINT>(mCurrentVertex - mBaseVertex), static_cast<UINT>(mBaseVertex));
    }

    mCurrentTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}


// Public constructor.
PrimitiveBatchBase::PrimitiveBatchBase(_In_ ID3D11DeviceContext* deviceContext, size_t maxIndices, size_t maxVertices, size_t vertexSize)
  : pImpl(std::make_unique<Impl>(deviceContext, maxIndices, maxVertices, vertexSize))
{
}


// Move constructor.
PrimitiveBatchBase::PrimitiveBatchBase(PrimitiveBatchBase&& moveFrom) noexcept
  : pImpl(std::move(moveFrom.pImpl))
{
}


// Move assignment.
PrimitiveBatchBase& PrimitiveBatchBase::operator= (PrimitiveBatchBase&& moveFrom) noexcept
{
    pImpl = std::move(moveFrom.pImpl);
    return *this;
}


// Public destructor.
PrimitiveBatchBase::~PrimitiveBatchBase()
{
}


void PrimitiveBatchBase::Begin()
{
    pImpl->Begin();
}


void PrimitiveBatchBase::End()
{
    pImpl->End();
}


_Use_decl_annotations_
void PrimitiveBatchBase::Draw(D3D11_PRIMITIVE_TOPOLOGY topology, bool isIndexed, uint16_t const* indices, size_t indexCount, size_t vertexCount, void** pMappedVertices)
{
    pImpl->Draw(topology, isIndexed, indices, indexCount, vertexCount, pMappedVertices);
}
