// Direct3D.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Direct3D.h"

#include <ovrvision_pro.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


#pragma warning(push)
#pragma warning(disable:4005)
// Include DirectX
#include "Win32_DirectXAppUtil.h"
#pragma warning(pop)

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;								// 現在のインターフェイス

extern int InitializeCamPlane(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, int w, int h, float zsize);
extern int RendererCamPlane(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);
extern int SetCamImage(ID3D11DeviceContext* DeviceContext, unsigned char* camImage, unsigned int imagesize);
extern int CleanCamPlane();

static bool MainLoop(bool retryCreate)
{
	HRESULT res;
	D3D11_TEXTURE2D_DESC texsture_desc = { 
		DIRECTX.WinSizeW,				// Width
		DIRECTX.WinSizeH,				// Height
		1,								// MipLevels
		1,								// ArraySize
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,	// Format
		{ 1 },							// SampleDesc.Count
		D3D11_USAGE_DEFAULT,			// Usage
	};

	int width = 0, height = 0, pixelsize = 4;
	Camera* mainCam = new Camera(&XMVectorSet(0.0f, 1.6f, 5.0f, 0), &XMQuaternionIdentity());
	DepthBuffer* pEyeDepthBuffer[2] = { nullptr, nullptr };
	OVR::OvrvisionPro ovrvision;

	char        luid[8]; 

	// Setup Device and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	if (DIRECTX.InitDevice(1280, 720, reinterpret_cast<LUID*>(&luid)))
	{
		if (ovrvision.Open(0, OVR::Camprop::OV_CAMHD_FULL, 2, DIRECTX.Device) == 0)
		{
			return false;
		}
		width = ovrvision.GetCamWidth();
		height = ovrvision.GetCamHeight();
		pixelsize = ovrvision.GetCamPixelsize();
		ovrvision.SetCameraSyncMode(false);

		InitializeCamPlane(DIRECTX.Device, DIRECTX.Context, width, height, 1.1f);

		//////////////////////////////////////////////////////////////////////////////
		// 肌色推定

		// Set scaling and size of scaled image
		OVR::ROI size = ovrvision.SetSkinScale(2);
		cv::Mat l, r;
		l.create(size.height, size.width, CV_8UC4);
		r.create(size.height, size.width, CV_8UC4);

		// Estimate skin color range with 120 frames
		ovrvision.DetectHand(120);
		for (bool done = false; done == false;)
		{
			ovrvision.Capture(OVR::Camqt::OV_CAMQT_DMS);
			done = ovrvision.GetScaledImageRGBA(l.data, r.data);
			cv::waitKey(1);
			cv::imshow("Left", l);
			cv::imshow("Right", r);
		}
		cv::destroyAllWindows();
		//////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////////////////
		// How to Create GPU texture and Update texture

		// Set scale 1/2 and get its size
		//OVR::ROI size = ovrvision.SetSkinScale(2); 

		D3D11_TEXTURE2D_DESC desc = {
			size.width,					// Width
			size.height,				// Height
			1,							// MipLevels
			1,							// ArraySize
			DXGI_FORMAT_R8G8B8A8_UNORM,	// Format TODO DXGI_FORMAT_R8G8B8A8_UNORM accept for Pixel Shader
			{ 1 },						// SampleDesc.Count
			D3D11_USAGE_DEFAULT,		// Usage
		};

		// Create Textures
		ID3D11Texture2D *pTextures[2];
		res = DIRECTX.Device->CreateTexture2D(&desc, NULL, &pTextures[0]);
		res = DIRECTX.Device->CreateTexture2D(&desc, NULL, &pTextures[1]);

		// Create GPU sharing textures
		ovrvision.CreateSkinTextures(size.width, size.height, pTextures[0], pTextures[1]);
		/////////////////////////////////////////////////////////////////////////////////////

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {
			DXGI_FORMAT_R8G8B8A8_UNORM,		// DXGI_FORMAT TODO DXGI_FORMAT_R8G8B8A8_UNORM accept for Pixel Shader
			D3D11_SRV_DIMENSION_TEXTURE2D,	// D3D11_SRV_DIMENSION
			{ 0, 1 },						// D3D11_TEX2D_SRV
		};
		D3D11_RENDER_TARGET_VIEW_DESC RenderDesc = {
			DXGI_FORMAT_R8G8B8A8_UNORM,		// DXGI_FORMAT
			D3D11_RTV_DIMENSION_TEXTURE2D,	// D3D11_RTV_DIMENSION
		};
		ID3D11RenderTargetView* pSrv;
		res = DIRECTX.Device->CreateRenderTargetView(pTextures[0], &RenderDesc, &pSrv);
		DIRECTX.SetAndClearRenderTarget(pSrv, NULL);
		//cv::Mat l(size.height, size.width, CV_8UC4);
		//cv::Mat r(size.height, size.width, CV_8UC4);

		// Main loop
		while (DIRECTX.HandleMessages())
		{
			/////////////////////////////////////////////////////////////////////////////////////
			// Capture image and hold it only in GPU
			ovrvision.Capture(OVR::Camqt::OV_CAMQT_DMSRMP);
			// Update textures
			ovrvision.UpdateSkinTextures(pTextures[0], pTextures[1]);
			/////////////////////////////////////////////////////////////////////////////////////

			// Inspect GPU internal image
			ovrvision.InspectTextures(l.data, r.data, 3);
			imshow("Left", l);
			imshow("Right", r);
#if 0
			for (int eye = 0; eye < 2; ++eye)
			{
				//Camera View
				if (eye == 0)
					SetCamImage(DIRECTX.Context, ovrvision.GetCamImageBGRA(OVR::Cameye::OV_CAMEYE_LEFT), width*pixelsize);
				else
					SetCamImage(DIRECTX.Context, ovrvision.GetCamImageBGRA(OVR::Cameye::OV_CAMEYE_RIGHT), width*pixelsize);

				RendererCamPlane(DIRECTX.Device, DIRECTX.Context);
			}
#endif
			// Render 
			//DIRECTX.Context->CopyResource(DIRECTX.BackBuffer, tex->D3D11.pTexture);
			DIRECTX.SwapChain->Present(0, 0);

		}
		ovrvision.Close();
	}
	DIRECTX.ReleaseDevice();
	return true;
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	hInst = hInstance;

	VALIDATE(DIRECTX.InitWindow(hInstance, L"Ovrvision Pro for OculusSDK"), "Failed to open window.");
	DIRECTX.Run(MainLoop);
	return 0;
}

