#pragma once 

#include "IRenderable.h" // interface to renderable objects

// this class provides access to directX functionality
class DirectXWrapper
{
public:
	// directX has to be aligned in the memory so I have special constructor for this reason
	static DirectXWrapper* GetDirectXWrapper(HWND hwnd, HINSTANCE hinst);
	static void DeleteDirectXWrapper(DirectXWrapper *pWrapper); //destructor

	// public accessors
	ID3D11Device*				GetDevice() { return m_d3dDevice; }
	ID3D11DeviceContext*		GetContext() { return m_immediateContext; }
	ID3D11ShaderResourceView*	GetDepthMap(){ return m_shaderResourceView; };

	void Render(); //render is called every frame and calls render function on all renderable objects
	void SetRenderToTexture();	//sets directX to render to depth map
	void SetRenderToBackBuffer();	//sets back buffer as a render target
	
	// function to control object allocation to the apropriate lists
	//add/remove object that doesn't cast shadows
	void AddNonCasterObject(IRenderable *pObject);
	void RemoveNonCasterObject(IRenderable *pObkect);
	//add/remove object that casts shadows
	void AddCasterObject(IRenderable* pObject);
	void RemoveCasterObject(IRenderable* pObject);
	//add/remove GUI object
	void AddUIObject(IRenderable *pObject);
	void RemoveUIObject(IRenderable* pObject);
	//add/remove particle system object
	void AddParticleObject(IRenderable *pObject);
	void RemoveParticleObject(IRenderable* pObject);
	//add/remove transparent object
	void AddAlphaObject(IRenderable* pObject);
	void RemoveAlphaObject(IRenderable* pObject);

private:
	DirectXWrapper(); // hide constructors as we need to use the factory method
	DirectXWrapper(HWND hwnd, HINSTANCE hinst);
	~DirectXWrapper();	//default destructor

	// COM objects required for directX to render
	D3D_DRIVER_TYPE						m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL					m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*						m_d3dDevice = nullptr;
	ID3D11Device1*						m_d3dDevice1 = nullptr;
	ID3D11DeviceContext*				m_immediateContext = nullptr;
	ID3D11DeviceContext1*				m_immediateContext1 = nullptr;
	IDXGISwapChain*						m_swapChain = nullptr;
	IDXGISwapChain1*					m_swapChain1 = nullptr;
	ID3D11RenderTargetView*				m_renderTargetView = nullptr;
	ID3D11Texture2D*                    m_depthStencil = nullptr;
	ID3D11DepthStencilView*             m_depthStencilView = nullptr;
	ID3D11BlendState*					m_blendState = nullptr;
	//for resources to render to texture I use shadow in variable names ( probably could find some better name, maybe depth)
	ID3D11Texture2D*					m_shadowMapTexture = nullptr;
	ID3D11Texture2D*					m_shadowDepthStencil = nullptr;
	ID3D11RenderTargetView*				m_shadowRenderTargetView = nullptr;
	ID3D11ShaderResourceView*			m_shaderResourceView = nullptr;
	ID3D11DepthStencilView*				m_shadowDepthStencilView = nullptr;
	//two viewports for each of rendering modes
	D3D11_VIEWPORT m_viewport;
	D3D11_VIEWPORT m_shadowViewport;
	
	HWND ourWindowHandle;	//window handle
	HINSTANCE ourInstance;	//window instance
	bool initialised;

	//lists of renderable objects
	vector<IRenderable*>	m_nonCasterList; //objects that don't cast any shadow
	vector<IRenderable*>	m_casterList;		//object that cast shadows
	vector<IRenderable*>	m_uiObjectList;	//GUI objects
	vector<IRenderable*>	m_particleList;	//particle system objects
	vector<IRenderable*>	m_alphaList;	//list of transparent objects
	
};