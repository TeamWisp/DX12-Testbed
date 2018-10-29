#include "DX12PipelineStateObject.h"

#include <ppltasks.h>
#include <d3dcompiler.h>
#include <Robuffer.h>

#include <stdio.h>

#include <iostream>

#include "Defines.h"

#pragma comment(lib, "D3DCompiler.lib")

namespace renderer {
	namespace dx12 {
		namespace utils {

			DX12PipelineStateObject::DX12PipelineStateObject(Device* device)
			{
				this->device = device;

				streamOutputDescription = {};

				blendDescription = {};
				blendDescription.AlphaToCoverageEnable = FALSE;
				blendDescription.IndependentBlendEnable = FALSE;
				SetBlendDescriptionRenderTargetBlendDescription();

				SetSampleMask();

				SetRasterizerDescription();

				SetIndexBufferStripCutValue();

				SetPrimitiveTopologyType();

				SetDepthStencilDescriptor();

				SetNumberOfRenderTargets();
				SetRenderTargetFormat();
				SetDepthStencilFormat();

				SetDXGISampleDescriptor();

				SetNodeMask();
			}


			DX12PipelineStateObject::~DX12PipelineStateObject()
			{
				pipelineStateObject->Release();
			}

			int DX12PipelineStateObject::LoadShader(std::wstring filename, ShaderType type, std::vector<D3D_SHADER_MACRO> shaderMacros, ID3DInclude* include, UINT compileOptions)
			{
				ID3DBlob* byteCode;
				ID3DBlob* errorLog;

				LPCSTR target = "vs_5_0";

				switch (type) {
				case ShaderType::VERTEX_SHADER:
					target = "vs_5_0";
					break;
				case ShaderType::HULL_SHADER:
					target = "hs_5_0";
					break;
				case ShaderType::DOMAIN_SHADER:
					target = "ds_5_0";
					break;
				case ShaderType::GEOMETRY_SHADER:
					target = "gs_5_0";
					break;
				case ShaderType::PIXEL_SHADER:
					target = "ps_5_0";
					break;
				}

#ifndef NDEBUG
				compileOptions |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // !NDEBUG


				HRESULT hr = D3DCompileFromFile(filename.c_str(), shaderMacros.data(), include, "main", target, compileOptions, 0, &byteCode, &errorLog);

				if (FAILED(hr)) {
					OutputDebugStringA((char*)errorLog->GetBufferPointer());
					return 1;
				}

				switch (type) {
				case ShaderType::VERTEX_SHADER:
					vertexShader = {};
					vertexShader.BytecodeLength = byteCode->GetBufferSize();
					vertexShader.pShaderBytecode = byteCode->GetBufferPointer();
					break;
				case ShaderType::HULL_SHADER:
					hullShader = {};
					hullShader.BytecodeLength = byteCode->GetBufferSize();
					hullShader.pShaderBytecode = byteCode->GetBufferPointer();
					break;
				case ShaderType::DOMAIN_SHADER:
					domainShader = {};
					domainShader.BytecodeLength = byteCode->GetBufferSize();
					domainShader.pShaderBytecode = byteCode->GetBufferPointer();
					break;
				case ShaderType::GEOMETRY_SHADER:
					geometryShader = {};
					geometryShader.BytecodeLength = byteCode->GetBufferSize();
					geometryShader.pShaderBytecode = byteCode->GetBufferPointer();
					break;
				case ShaderType::PIXEL_SHADER:
					pixelShader = {};
					pixelShader.BytecodeLength = byteCode->GetBufferSize();
					pixelShader.pShaderBytecode = byteCode->GetBufferPointer();
					break;
				}

				return 0;
			}

			void DX12PipelineStateObject::AddInputElementDescription(std::string elementName, UINT elementIndex, DXGI_FORMAT format, UINT inputSlot, UINT alignedByteOffset, D3D12_INPUT_CLASSIFICATION inputSlotClass, UINT instanceDataStepRate)
			{
				D3D12_INPUT_ELEMENT_DESC desc = {};
				char* name = (CHAR*)malloc(sizeof(CHAR)*(strlen(elementName.c_str())+1));
				memcpy(name, elementName.c_str(), elementName.length());
				name[elementName.length()] = '\0';
				desc.SemanticName = name;
				desc.SemanticIndex = elementIndex;
				desc.Format = format;
				desc.InputSlot = inputSlot;
				desc.AlignedByteOffset = alignedByteOffset;
				desc.InputSlotClass = inputSlotClass;
				desc.InstanceDataStepRate = instanceDataStepRate;

				inputElementDescriptions.push_back(desc);
			}

			void DX12PipelineStateObject::SetStreamOutputDescription(std::vector<D3D12_SO_DECLARATION_ENTRY> soDeclarations, std::vector<UINT> bufferStrides, UINT rasterizedStream)
			{
				streamOutputDeclarations = soDeclarations;
				streamOutputBufferStrides = bufferStrides;
				streamOutputDescription.pSODeclaration = streamOutputDeclarations.data();
				streamOutputDescription.NumEntries = streamOutputDeclarations.size();
				streamOutputDescription.pBufferStrides = streamOutputBufferStrides.data();
				streamOutputDescription.NumStrides = streamOutputBufferStrides.size();
				streamOutputDescription.RasterizedStream = rasterizedStream;
			}

			void DX12PipelineStateObject::SetBlendDescriptionAlphaToCoverageEnable(BOOL enabled)
			{
				blendDescription.AlphaToCoverageEnable = enabled;
			}

			void DX12PipelineStateObject::SetBlendDescriptionIndependentBlendEnable(BOOL enabled)
			{
				blendDescription.IndependentBlendEnable = enabled;
			}

			void DX12PipelineStateObject::SetBlendDescriptionRenderTargetBlendDescription(size_t renderTarget, BOOL blendEnable, D3D12_BLEND srcBlend, D3D12_BLEND dstBlend, D3D12_BLEND_OP blendOp, D3D12_BLEND srcBlendAlpha, D3D12_BLEND dstBlendAlpha, D3D12_BLEND_OP blendOpAlpha, BOOL logicOpEnable, D3D12_LOGIC_OP logicOp, UINT8 renderTargetWriteMask)
			{
				if (renderTarget < 8) {
					blendDescription.RenderTarget[renderTarget].BlendEnable = blendEnable;
					blendDescription.RenderTarget[renderTarget].SrcBlend = srcBlend;
					blendDescription.RenderTarget[renderTarget].DestBlend = dstBlend;
					blendDescription.RenderTarget[renderTarget].BlendOp = blendOp;
					blendDescription.RenderTarget[renderTarget].SrcBlendAlpha = srcBlendAlpha;
					blendDescription.RenderTarget[renderTarget].DestBlendAlpha = dstBlendAlpha;
					blendDescription.RenderTarget[renderTarget].BlendOpAlpha = blendOpAlpha;
					blendDescription.RenderTarget[renderTarget].LogicOpEnable = logicOpEnable;
					blendDescription.RenderTarget[renderTarget].LogicOp = logicOp;
					blendDescription.RenderTarget[renderTarget].RenderTargetWriteMask = renderTargetWriteMask;
				}
			}

			void DX12PipelineStateObject::SetSampleMask(UINT sampleMask)
			{
				this->sampleMask = sampleMask;
			}

			void DX12PipelineStateObject::SetRasterizerDescription(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, BOOL frontCounterClockWise, INT depthBias, FLOAT depthBiasClamp, FLOAT slopeScaledDepthBias, BOOL depthClipEnabled, BOOL multiSampleEnable, BOOL lineAntiAliasing, INT forcedSampleCount, D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRasterizationMode)
			{
				rasterizerDescription.FillMode = fillMode;
				rasterizerDescription.CullMode = cullMode;
				rasterizerDescription.FrontCounterClockwise = frontCounterClockWise;
				rasterizerDescription.DepthBias = depthBias;
				rasterizerDescription.DepthBiasClamp = depthBiasClamp;
				rasterizerDescription.SlopeScaledDepthBias = slopeScaledDepthBias;
				rasterizerDescription.DepthClipEnable = depthClipEnabled;
				rasterizerDescription.MultisampleEnable = multiSampleEnable;
				rasterizerDescription.AntialiasedLineEnable = lineAntiAliasing;
				rasterizerDescription.ForcedSampleCount = forcedSampleCount;
				rasterizerDescription.ConservativeRaster = conservativeRasterizationMode;
			}

			void DX12PipelineStateObject::SetRootSignature(DX12RootSignature * rootSignature)
			{
				this->rootSignature = rootSignature;
			}

			DX12RootSignature * DX12PipelineStateObject::GetRootSignature()
			{
				return rootSignature;
			}

			void DX12PipelineStateObject::SetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE cutValue)
			{
				indexBufferStripCutValue = cutValue;
			}

			void DX12PipelineStateObject::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
			{
				this->primitiveTopologyType = type;
			}

			void DX12PipelineStateObject::SetNumberOfRenderTargets(UINT number)
			{
				numRenderTargets = number;
			}

			void DX12PipelineStateObject::SetRenderTargetFormat(UINT renderTarget, DXGI_FORMAT format)
			{
				if (renderTarget < 8) {
					RTVFormats[renderTarget] = format;
				}
			}

			void DX12PipelineStateObject::SetDepthStencilFormat(DXGI_FORMAT format)
			{
				DSVFormat = format;
			}

			void DX12PipelineStateObject::SetDXGISampleDescriptor(DXGI_SAMPLE_DESC sample)
			{
				sampleDescriptor = sample;
			}

			void DX12PipelineStateObject::SetNodeMask(UINT nodeMask)
			{
				this->nodeMask = nodeMask;
			}

			void DX12PipelineStateObject::SetDepthStencilDescriptor(BOOL depthTestEnable, D3D12_DEPTH_WRITE_MASK writeMask, D3D12_COMPARISON_FUNC depthFunc, BOOL stencilTestEnable, UINT8 stencilReadMask, UINT8 stencilWriteMask, D3D12_DEPTH_STENCILOP_DESC frontFace, D3D12_DEPTH_STENCILOP_DESC backFace)
			{
				depthStencilDescription.DepthEnable = depthTestEnable;
				depthStencilDescription.DepthWriteMask = writeMask;
				depthStencilDescription.DepthFunc = depthFunc;
				depthStencilDescription.StencilEnable = stencilTestEnable;
				depthStencilDescription.StencilReadMask = stencilReadMask;
				depthStencilDescription.StencilWriteMask = stencilWriteMask;
				depthStencilDescription.FrontFace = frontFace;
				depthStencilDescription.BackFace = backFace;
			}

			int DX12PipelineStateObject::Compile()
			{
				inputLayoutDescription.NumElements = inputElementDescriptions.size();
				inputLayoutDescription.pInputElementDescs = inputElementDescriptions.data();

				pipelineStateDescription = {};
				if (rootSignature != nullptr) {
					pipelineStateDescription.pRootSignature = rootSignature->GetRootSignature();
				}
				else {
					pipelineStateDescription.pRootSignature = nullptr;
				}
				pipelineStateDescription.VS = vertexShader;
				pipelineStateDescription.DS = domainShader;
				pipelineStateDescription.HS = hullShader;
				pipelineStateDescription.GS = geometryShader;
				pipelineStateDescription.PS = pixelShader;
				pipelineStateDescription.StreamOutput = streamOutputDescription;
				pipelineStateDescription.BlendState = blendDescription;
				pipelineStateDescription.SampleMask = sampleMask;
				pipelineStateDescription.RasterizerState = rasterizerDescription;
				pipelineStateDescription.DepthStencilState = depthStencilDescription;
				pipelineStateDescription.InputLayout = inputLayoutDescription;
				pipelineStateDescription.IBStripCutValue = indexBufferStripCutValue;
				pipelineStateDescription.PrimitiveTopologyType = primitiveTopologyType;
				pipelineStateDescription.NumRenderTargets = numRenderTargets;
				for (int i = 0; i < 8; ++i) {
					pipelineStateDescription.RTVFormats[i] = RTVFormats[i];
				}
				pipelineStateDescription.DSVFormat = DSVFormat;
				pipelineStateDescription.SampleDesc = sampleDescriptor;
				pipelineStateDescription.NodeMask = nodeMask;
				pipelineStateDescription.CachedPSO = {};

				HRESULT hr = device->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDescription, IID_PPV_ARGS(&pipelineStateObject));
				if (FAILED(hr)) {
					std::cout << "[ERROR] Creating PSO failed with error code " << hr << std::endl;
					return hr;
				}
				return 0;

			}

			ID3D12PipelineState * DX12PipelineStateObject::GetPSO()
			{
				return pipelineStateObject;
			}

		}
	}
}