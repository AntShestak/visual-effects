#include "stdafx.h"
#include "DirectXWrapper.h"

//set the shadow map resolution
const int SHADOWMAP_WIDTH = 1025;
const int SHADOWMAP_HEIGHT = 1025;

//for alignment purpose use malloc
DirectXWrapper* DirectXWrapper::GetDirectXWrapper(HWND hwnd, HINSTANCE hinst)
{
	DirectXWrapper *ret = (DirectXWrapper*)_aligned_malloc(sizeof(DirectXWrapper), 16); // obtain pointer to uninitialised aligned clock of memory
	new (ret)DirectXWrapper(hwnd, hinst); //create directX on the block of memory retrieved
	return ret;
}
//function that calls destructor and sets free alocated memory
void DirectXWrapper::DeleteDirectXWrapper(DirectXWrapper *pWrapper)
{
	pWrapper->~DirectXWrapper(); //calling default destructor
	_aligned_free(pWrapper); // free memory
}

//create all the COM objects in this function
DirectXWrapper::DirectXWrapper(HWND hwnd, HINSTANCE hinst) : ourWindowHandle(hwnd), ourInstance(hinst)
{
	initialised = false; //is not yet initialised

	CoInitialize(NULL); // needed by the WIC loader

	HRESULT hr = S_OK;

	// get the rectangular size of the window
	RECT rc; //rectangle rc
	GetClientRect(ourWindowHandle, &rc); //store client rectangle
	//calculate width & height
	UINT width = rc.right - rc.left; 
	UINT height = rc.bottom - rc.top;

	// determine some device creation flags
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// attempt to cover all bases
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,  // HW rasteriser
		D3D_DRIVER_TYPE_WARP,      // SW rasteriser
		D3D_DRIVER_TYPE_REFERENCE, // SW raseriser
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	//loop that enumerates attempts to create a device starting with HW. Breaks if succeeded.
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_d3dDevice, &m_featureLevel, &m_immediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to Create a DirectX device"); // we print debug messages if anything goes wrong, at least then there's something in the debug log
		return; // couldn't start DirectX
	}

	// Obtain DXGI factory from device 
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to obtain DXGI factory");
		return;
	}

	// having a reference to a DXGI factory I can create other objects

	// set swap chain properties
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = ourWindowHandle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	//created swap chain
	hr = dxgiFactory->CreateSwapChain(m_d3dDevice, &sd, &m_swapChain);
	//release factory as it's no longer needed
	dxgiFactory->Release();
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create Swap Chain");
		return;
	}
	//now initialising resource for rendering to the screen
	//create a back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to get back buffer");
		return;
	}
	//create render target view using back buffer, this will be presented on screen
	hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create render target vies");
		return;
	}
	// Create depth stencil texture
	//set properties on the desc
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	//create depth stencil
	hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, &m_depthStencil);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth stencil texture");
		return;
	}
	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil, &descDSV, &m_depthStencilView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth stencil view");
		return;
	}
	// Setup the viewport for rendering to screen
	m_viewport.Width = (FLOAT)width;
	m_viewport.Height = (FLOAT)height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	////////////////////////////////////////////////////////////
	//now initialising render to texture resources
	///////////////////////////////////////////////////////////
	//creating a texture of size SHADOWMAP_WIDTH x SHADOWMAP_HEIGHT
	//this texture is gonna be used as render target
	//so I set a flag - D3D11_BIND_RENDER_TARGET
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = SHADOWMAP_WIDTH;
	textureDesc.Height = SHADOWMAP_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	//create texture
	hr = m_d3dDevice->CreateTexture2D(&textureDesc, NULL, &m_shadowMapTexture);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create Render Target Texture");
		return;
	}
	//creating render target view for render to texture
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	hr = m_d3dDevice->CreateRenderTargetView(m_shadowMapTexture, &renderTargetViewDesc, &m_shadowRenderTargetView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create Render Target View");
		return;
	}
	//creating shader resource view from the texture above
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = m_d3dDevice->CreateShaderResourceView(m_shadowMapTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create Shader resource View ");
		return;
	}
	//Creating depth stencil
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = SHADOWMAP_WIDTH;
	depthBufferDesc.Height = SHADOWMAP_HEIGHT;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	hr = m_d3dDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_shadowDepthStencil);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth texture");
		return;
	}
	//creating depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = depthBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = m_d3dDevice->CreateDepthStencilView(m_shadowDepthStencil, &depthStencilViewDesc, &m_shadowDepthStencilView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth stencil view");
		return;
	}
	//setting the viewPort for texture size. viewPort for renderind depthMap
	m_shadowViewport.Width = (float)SHADOWMAP_WIDTH;
	m_shadowViewport.Height = (float)SHADOWMAP_HEIGHT;
	m_shadowViewport.MinDepth = 0.0f;
	m_shadowViewport.MaxDepth = 1.0f;
	m_shadowViewport.TopLeftX = 0.0f;
	m_shadowViewport.TopLeftY = 0.0f;

	/////////////////////////////////////////////

	//Create the blending state for transparent object rendering
	D3D11_BLEND_DESC blendState;
	ZeroMemory(&blendState, sizeof(blendState));
	blendState.AlphaToCoverageEnable = false;
	blendState.IndependentBlendEnable = false;
	blendState.RenderTarget[0].BlendEnable = true;
	blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_d3dDevice->CreateBlendState(&blendState, &m_blendState);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create blend state");
		return;
	}

	initialised = true; // we succeeded in initialising DirectX
}

DirectXWrapper::~DirectXWrapper()
{
	//release all the objects created
	if (m_immediateContext) m_immediateContext->ClearState();
	if (m_renderTargetView) m_renderTargetView->Release();
	if (m_swapChain1) m_swapChain1->Release();
	if (m_swapChain) m_swapChain->Release();
	if (m_immediateContext1) m_immediateContext1->Release();
	if (m_immediateContext) m_immediateContext->Release();
	if (m_d3dDevice1) m_d3dDevice1->Release();
	if (m_d3dDevice) m_d3dDevice->Release();
	if (m_depthStencil) m_depthStencil->Release();
	if (m_depthStencilView) m_depthStencilView->Release();
	if (m_shadowMapTexture) m_shadowMapTexture->Release();
	if (m_shadowDepthStencil) m_shadowDepthStencil->Release();
	if (m_shadowRenderTargetView) m_shadowRenderTargetView->Release();
	if (m_shaderResourceView) m_shaderResourceView->Release();
	if (m_shadowDepthStencilView) m_shadowDepthStencilView->Release();
	if (m_blendState) m_blendState->Release();

}
//render function is called every frame from GraphicsEngine
void DirectXWrapper::Render()
{
	//I set topology here as all my primitives use the same topology
	m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 
	//1st render pass
	//set render target to render to texture
	SetRenderToTexture();
	// iterate only the objects that cast shadows and call render function with pass 1
	for (unsigned int i = 0; i < m_casterList.size(); i++)
	{
		m_casterList[i]->Render(this,1);
	}

	//now 2nd rendering pass
	//set render target to render to back buffer
	SetRenderToBackBuffer();
	//set blend state
	m_immediateContext->OMSetBlendState(m_blendState, 0, 0xffffffff);

	// iterate through all the objects
	//first objects that dont cast shadow
	for (unsigned int i = 0; i < m_nonCasterList.size(); i++)
	{
		m_nonCasterList[i]->Render(this, 2); //call render with pass = 2
	}
	//now again object that cast shadow but this time rendering to back buffer
	for (unsigned int i = 0; i < m_casterList.size(); i++)
	{
		m_casterList[i]->Render(this, 2); //call render with pass = 2
	}
	//now I can render tanserent object
	for (unsigned int i = 0; i < m_alphaList.size(); i++)
	{
		m_alphaList[i]->Render(this, 2);
	}
	//particle objects
	for (unsigned int i = 0; i < m_particleList.size(); i++)
	{
		m_particleList[i]->Render(this, 2); //pass doesn't really matter here
	}
	//and final list is UI list so it renders on top of other objects properly
	for (unsigned int i = 0; i < m_uiObjectList.size(); i++)
	{
		m_uiObjectList[i]->Render(this, 2);
	}

	// Swap the front and back buffers 
	m_swapChain->Present(0, 0);

}
////////////////////////////////////////////////////////////////
//some list allocation control functions
//only difference between them is that each uses different list
//////////////////
//add/remove object that doesn't cast shadows
void DirectXWrapper::AddNonCasterObject(IRenderable *pObject) 
{
	m_nonCasterList.push_back(pObject);
	// call the init
	pObject->Initialise(this);
	return;
}
void DirectXWrapper::RemoveNonCasterObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < m_nonCasterList.size(); i++)
	{
		if (m_nonCasterList[i] == pObject)
		{
			m_nonCasterList.erase(m_nonCasterList.begin() + i);
			break;
		}
	}
	return;
}
//add/remove object that casts shadow
void DirectXWrapper::AddCasterObject(IRenderable *pObject)
{
	m_casterList.push_back(pObject);
	// call the init
	pObject->Initialise(this);
	return;
}
void DirectXWrapper::RemoveCasterObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < m_casterList.size(); i++)
	{
		if (m_casterList[i] == pObject)
		{
			m_casterList.erase(m_casterList.begin() + i);
			break;
		}
	}
	return;
}
//add/remove GUI object
void DirectXWrapper::AddUIObject(IRenderable* pObject)
{
	m_uiObjectList.push_back(pObject);
	pObject->Initialise(this);
	return;
}
void DirectXWrapper::RemoveUIObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < m_uiObjectList.size(); i++)
	{
		if (m_uiObjectList[i] == pObject)
		{
			m_uiObjectList.erase(m_uiObjectList.begin() + i);
			break;
		}
	}
	return;
}
//add/remove paticle system object
void DirectXWrapper::AddParticleObject(IRenderable* pObject)
{
	m_particleList.push_back(pObject);
	pObject->Initialise(this);
	return;
}
void DirectXWrapper::RemoveParticleObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < m_particleList.size(); i++)
	{
		if (m_particleList[i] == pObject)
		{
			m_particleList.erase(m_particleList.begin() + i);
			break;
		}
	}
	return;
}
//add/remove transparent object
void DirectXWrapper::AddAlphaObject(IRenderable* pObject)
{
	m_alphaList.push_back(pObject);
	pObject->Initialise(this);
	return;
}
void DirectXWrapper::RemoveAlphaObject(IRenderable *pObject)
{
	for (unsigned int i = 0; i < m_alphaList.size(); i++)
	{
		if (m_alphaList[i] == pObject)
		{
			m_alphaList.erase(m_alphaList.begin() + i);
			break;
		}
	}

	return;
}

////////////////////////////////////////////////////////////

//rendering control functions
//this functions sets rendering target to a back buffer
void DirectXWrapper::SetRenderToBackBuffer()
{
	//set the correct render target
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	// clear the backbuffer
	m_immediateContext->ClearRenderTargetView(m_renderTargetView, Colors::Black); //regarding color, black is nice, BUT hard to see the smokes
	// Clear the depth buffer to 1.0 (max depth)
	m_immediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//set the correct viewport
	m_immediateContext->RSSetViewports(1, &m_viewport);
	return;
}
//this functions sets rendering target to a depthMapTexture
void DirectXWrapper::SetRenderToTexture()
{
	//set the correct render target
	m_immediateContext->OMSetRenderTargets(1, &m_shadowRenderTargetView, m_shadowDepthStencilView);
	// clear the backbuffer
	m_immediateContext->ClearRenderTargetView(m_shadowRenderTargetView, Colors::Crimson);
	// Clear the depth buffer to 1.0 (max depth)
	m_immediateContext->ClearDepthStencilView(m_shadowDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	//se the correct viewport
	m_immediateContext->RSSetViewports(1, &m_shadowViewport);
	return;
}


