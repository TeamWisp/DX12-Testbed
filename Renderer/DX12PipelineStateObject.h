#pragma once

#include <d3d12.h>

#include "DX12Device.h"

#include "DX12RootSignature.h"

namespace renderer {
	namespace dx12 {
		namespace utils {

			enum class ShaderType {
				VERTEX_SHADER=0,
				HULL_SHADER,
				DOMAIN_SHADER,
				GEOMETRY_SHADER,
				PIXEL_SHADER
			};

			class DX12PipelineStateObject
			{
			public:
				DX12PipelineStateObject(Device* device);
				~DX12PipelineStateObject();

				int LoadShader(std::wstring filename, 
					ShaderType type, 
					std::vector<D3D_SHADER_MACRO> shaderMacros, 
					ID3DInclude* include, 
					UINT compileOptions);

				void AddInputElementDescription(std::string elementName, 
					UINT elementIndex, 
					DXGI_FORMAT format, 
					UINT inputSlot, 
					UINT alignedByteOffset, 
					D3D12_INPUT_CLASSIFICATION inputSlotClass, 
					UINT instanceDataStepRate);

				void SetStreamOutputDescription(std::vector<D3D12_SO_DECLARATION_ENTRY> soDeclarations, std::vector<UINT> bufferStrides, UINT rasterizedStream);

				void SetBlendDescriptionAlphaToCoverageEnable(BOOL enabled);

				void SetBlendDescriptionIndependentBlendEnable(BOOL enabled);

				void SetBlendDescriptionRenderTargetBlendDescription(size_t renderTarget = 0,
					BOOL blendEnable = FALSE, D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND dstBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
					D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND dstBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD,
					BOOL logicOpEnable = FALSE, D3D12_LOGIC_OP logicOp = D3D12_LOGIC_OP_NOOP, UINT8 renderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL);

				void SetSampleMask(UINT sampleMask = 0xffffffff);

				void SetRasterizerDescription(D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK,
					BOOL frontCounterClockWise = TRUE, INT depthBias = 0, FLOAT depthBiasClamp = 0.f, FLOAT slopeScaledDepthBias = 0.f,
					BOOL depthClipEnabled = TRUE, BOOL multiSampleEnable = FALSE, BOOL lineAntiAliasing = FALSE, INT forcedSampleCount = 0,
					D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRasterizationMode = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

				void SetRootSignature(DX12RootSignature* rootSignature);
				DX12RootSignature* GetRootSignature();

				void SetIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE cutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED);

				void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

				void SetNumberOfRenderTargets(UINT number = 1);

				void SetRenderTargetFormat(UINT renderTarget = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

				void SetDepthStencilFormat(DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);

				void SetDXGISampleDescriptor(DXGI_SAMPLE_DESC sample = { 1,0 });

				void SetNodeMask(UINT nodeMask = 0);

				void SetDepthStencilDescriptor(BOOL depthTestEnable = FALSE, D3D12_DEPTH_WRITE_MASK writeMask = D3D12_DEPTH_WRITE_MASK_ZERO,
					D3D12_COMPARISON_FUNC depthFunc = D3D12_COMPARISON_FUNC_LESS,
					BOOL stencilTestEnable = FALSE, UINT8 stencilReadMask = 0, UINT8 stencilWriteMask = 0,
					D3D12_DEPTH_STENCILOP_DESC frontFace = { D3D12_STENCIL_OP_KEEP,D3D12_STENCIL_OP_KEEP ,D3D12_STENCIL_OP_KEEP ,D3D12_COMPARISON_FUNC_ALWAYS },
					D3D12_DEPTH_STENCILOP_DESC backFace = { D3D12_STENCIL_OP_KEEP,D3D12_STENCIL_OP_KEEP ,D3D12_STENCIL_OP_KEEP ,D3D12_COMPARISON_FUNC_ALWAYS });

				int Compile();

				ID3D12PipelineState* GetPSO();

			protected:
				Device * device;

				D3D12_SHADER_BYTECODE vertexShader;
				D3D12_SHADER_BYTECODE hullShader;
				D3D12_SHADER_BYTECODE domainShader;
				D3D12_SHADER_BYTECODE geometryShader;
				D3D12_SHADER_BYTECODE pixelShader;

				std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescriptions;

				D3D12_INPUT_LAYOUT_DESC inputLayoutDescription;

				std::vector<D3D12_SO_DECLARATION_ENTRY> streamOutputDeclarations; 
				std::vector<UINT> streamOutputBufferStrides;

				D3D12_STREAM_OUTPUT_DESC streamOutputDescription;
				
				D3D12_BLEND_DESC blendDescription;

				UINT sampleMask;

				D3D12_RASTERIZER_DESC rasterizerDescription;

				D3D12_DEPTH_STENCIL_DESC depthStencilDescription;

				DX12RootSignature* rootSignature;

				D3D12_INDEX_BUFFER_STRIP_CUT_VALUE indexBufferStripCutValue;

				D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType;

				UINT numRenderTargets;

				DXGI_FORMAT RTVFormats[8];

				DXGI_FORMAT DSVFormat;

				DXGI_SAMPLE_DESC sampleDescriptor;

				UINT nodeMask;

				D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDescription;

				ID3D12PipelineState* pipelineStateObject;
			};

		}
	}
}