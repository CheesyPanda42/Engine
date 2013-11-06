#include "D3Dclass.h"


D3Dclass::D3Dclass(void)
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState =0;
	m_rasterState = 0;
}

D3Dclass::D3Dclass(const D3Dclass& other)
{
}


D3Dclass::~D3Dclass(void)
{
}


bool D3Dclass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput *adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLen;
	DXGI_MODE_DESC *displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D *backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	// vsync setting
	m_vsync_enabled = vsync;


	// Create DX graphics interface factory
	result = CreateDXGIFactory(_uuidof(IDXGIFactory), (void**)&factory);
	if(FAILED(result))
	{
		return false;
	}

	// Use factor to create adapter for primary graphics interface
	result = factory->EnumAdapters(0,&adapter);
	if(FAILED(result))
	{
		return false;
	}

	// Enumerate primary adapter output
	result = adapter->EnumOutputs(0, &adapterOutput);
	if(FAILED(result))
	{
		return false;
	}

	// Get number of modes that fit the format for the adapter output
	result = adapterOutput-> GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Greate a list to hold all the possible display modes for the monitor/vc combo
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if(FAILED(result))
	{
		return false;
	}

	// Find display mode that matches screen width and height
	// When a match is found, store num and dem of the refresh rate for that monitor
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int) screenWidth)
		{
			if(displayModeList[i].Height == (unsigned int) screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}



	// Get adapter description 
	result = adapter->GetDesc(&adapterDesc);
	if(FAILED(result))
	{
		return false;
	}

	m_videoCardMem = (int)(adapterDesc.DedicatedVideoMemory/1024/1024);

	// Convert the name of the video card to a char array
	error = wcstombs_s(&stringLen, m_videoCardDesc, 128, adapterDesc.Description, 128);
	if(error != 0)
	{
		return false;
	}


	// Release display mode list
	delete [] displayModeList;
	displayModeList = 0;

	// Release adapter output
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter
	adapter->Release();
	adapter = 0;

	// Release the factory
	factory->Release();
	factory = 0;

	// Initialize swap chain desc
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer
	swapChainDesc.BufferCount = 1;

	// Set width and height of back buffer
	swapChainDesc.BufferDesc.Width  = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for back buffer
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	// Set the refresh rate of back buffer
	if(m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set usage of back buffer
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set handle for window
	swapChainDesc.OutputWindow = hwnd;

	// Turn multisampling off
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// set window or fullscreen
	if(fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}


	// Set scan line ordering
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; 

	// Discard back buffer
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set advanced flag
	swapChainDesc.Flags = 0;

	// Set DirectX feature level
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create swap chain, D3D device, and D3D device context
	// For non DX11, can sway D3D_DRIVER_TYPE_HARDWARE with D3D_DRIVER_TYPE_REFERENCE
	// Not, this uses CPU instead of graphics card, so is much slower
	result = D3D11CreateDeviceAndSwapChain( NULL,
											D3D_DRIVER_TYPE_HARDWARE,
											NULL,
											0,
											&featureLevel,
											1,
											D3D11_SDK_VERSION,
											&swapChainDesc,
											&m_swapChain,
											&m_device, 
											NULL,
											&m_deviceContext);


	if(FAILED(result))
	{
		return false;
	}


	// Get the pointer to the back buffer
	result = m_swapChain->GetBuffer(0,_uuidof(ID3D11Texture2D),(LPVOID*)&backBufferPtr);
	if(FAILED(result))
	{
		return false;
	}

	// Create render target
	result = m_device->CreateRenderTargetView(backBufferPtr,NULL,&m_renderTargetView);
	if(FAILED(result))
	{
		return false;
	}

	// Release pointer to back buffer
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize description of the depth buffer
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up description of depth buffer
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create teh texture for the depth buffer
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if(FAILED(result))
	{
		return false;
	}


	// Initialize description of stencil state
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set description of stencil state
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil ops if pixel is front facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil if back facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	// Set depth stencil state
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// Initialize depth stencil view
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// set up depth stencil view
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create depth stencil view
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if(FAILED(result))
	{
		return false;
	}


	// Bind render target view and depth stencil buffer
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Setup taster description
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create tasterizer state
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(result))
	{
		return false;
	}

	// Set rasterizer state
	m_deviceContext->RSSetState(m_rasterState);

	// Setup viewport for rendering
	viewport.Height = (float) screenWidth;
	viewport.Width = (float) screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create viewport
	m_deviceContext->RSSetViewports(1,&viewport);

	// Setup proj matrix
	fieldOfView = (float)D3DX_PI/4.0f;
	screenAspect = (float)screenWidth/(float)screenHeight;

	// Create proj matrix for 3D rendering
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

	// Initialize world matrix to identity matrix
	D3DXMatrixIdentity(&m_worldMatrix);

	// Create orthographic proj matrix for 2D rendering
	D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;

}



void D3Dclass::Shutdown()
{
	// Set to windowed mode before releasing swap chain
	if(m_swapChain)
	{
		m_swapChain->SetFullscreenState(false,NULL);
	}

	if(m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if(m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if(m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if(m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if(m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if(m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if(m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if(m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}


void D3Dclass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	// setup color to clear buffer to
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// clear back buffer
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	// clear depth buffer
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;

}

void D3Dclass::EndScene()
{
	// Present back buffer to screen
	if(m_vsync_enabled)
	{
		// Lock screen to refresh rate
		m_swapChain->Present(1,0);
	}
	else
	{
		// Present as fast as possible
		m_swapChain->Present(0,0);
	}

	return;
}

ID3D11Device* D3Dclass::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3Dclass::GetDeviceContext()
{
	return m_deviceContext;
}

void D3Dclass::GetprojectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

void D3Dclass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

void D3Dclass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}


void D3Dclass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName,128,m_videoCardDesc);
	memory = m_videoCardMem;
	return;
}