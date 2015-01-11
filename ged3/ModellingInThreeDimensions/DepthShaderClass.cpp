////////////////////////////////////////////////////////////////////////////////
// Filename: depthshaderclass.cpp
// http://rastertek.com/dx10tut40.html
////////////////////////////////////////////////////////////////////////////////
#include "DeptShaderClass.h"


DepthShaderClass::DepthShaderClass()
{
	m_effect = 0;
	m_technique = 0;
	m_layout = 0;

	m_worldMatrixPtr = 0;
	m_viewMatrixPtr = 0;
	m_projectionMatrixPtr = 0;
}


DepthShaderClass::DepthShaderClass(const DepthShaderClass& other)
{
}


DepthShaderClass::~DepthShaderClass()
{
}


bool DepthShaderClass::Initialize(ID3D10Device* device)
{
	bool result;


	// Initialize the shader that will be used to draw the triangles.
	result = InitializeShader(device, L"data/Shadow/depth.fx");
	if(!result)
	{
		return false;
	}

	return true;
}


void DepthShaderClass::Shutdown()
{
	// Shutdown the shader effect.
	ShutdownShader();

	return;
}


void DepthShaderClass::Render(ID3D10Device* device, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);

	// Now render the prepared buffers with the shader.
	RenderShader(device, indexCount);

	return;
}


bool DepthShaderClass::InitializeShader(ID3D10Device* device, WCHAR* filename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	D3D10_INPUT_ELEMENT_DESC polygonLayout[1];
	unsigned int numElements;
    D3D10_PASS_DESC passDesc;


	// Initialize the error message.
	errorMessage = 0;

	// Load the shader in from the file.
	result = D3DX10CreateEffectFromFile(filename, NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, 
										device, NULL, NULL, &m_effect, &errorMessage, NULL);
	if(FAILED(result))
		return false;

	// Get a pointer to the technique inside the shader.
	m_technique = m_effect->GetTechniqueByName("DepthTechnique");
	if(!m_technique)
	{
		return false;
	}

	// Now setup the layout of the data that goes into the shader.
	// This setup needs to match the VertexType stucture in this class and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Get the description of the first pass described in the shader technique.
    m_technique->GetPassByIndex(0)->GetDesc(&passDesc);

	// Create the input layout.
    result = device->CreateInputLayout(polygonLayout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_layout);
	if(FAILED(result))
	{
		return false;
	}

	// Get pointers to the three matrices inside the shader so we can update them from this class.
    m_worldMatrixPtr = m_effect->GetVariableByName("worldMatrix")->AsMatrix();
	m_viewMatrixPtr = m_effect->GetVariableByName("viewMatrix")->AsMatrix();
    m_projectionMatrixPtr = m_effect->GetVariableByName("projectionMatrix")->AsMatrix();

	return true;
}


void DepthShaderClass::ShutdownShader()
{
	// Release the pointers to the matrices inside the shader.
	m_worldMatrixPtr = 0;
	m_viewMatrixPtr = 0;
	m_projectionMatrixPtr = 0;

	// Release the pointer to the shader layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pointer to the shader technique.
	m_technique = 0;

	// Release the pointer to the shader.
	if(m_effect)
	{
		m_effect->Release();
		m_effect = 0;
	}

	return;
}

void DepthShaderClass::SetShaderParameters(D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix)
{
	// Set the world matrix variable inside the shader.
    m_worldMatrixPtr->SetMatrix((float*)&worldMatrix);

	// Set the view matrix variable inside the shader.
	m_viewMatrixPtr->SetMatrix((float*)&viewMatrix);

	// Set the projection matrix variable inside the shader.
    m_projectionMatrixPtr->SetMatrix((float*)&projectionMatrix);

	return;
}


void DepthShaderClass::RenderShader(ID3D10Device* device, int indexCount)
{
    D3D10_TECHNIQUE_DESC techniqueDesc;
	unsigned int i;
	

	// Set the input layout.
	device->IASetInputLayout(m_layout);

	// Get the description structure of the technique from inside the shader so it can be used for rendering.
    m_technique->GetDesc(&techniqueDesc);

    // Go through each pass in the technique (should be just one currently) and render the triangles.
	for(i=0; i<techniqueDesc.Passes; ++i)
    {
        m_technique->GetPassByIndex(i)->Apply(0);
        device->DrawIndexed(indexCount, 0, 0);
    }

	return;
}