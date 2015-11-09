// ovrvision_csharp.cpp
//
//MIT License
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
// Oculus Rift : TM & Copyright Oculus VR, Inc. All Rights Reserved
// Unity : TM & Copyright Unity Technologies. All Rights Reserved

/////////// INCLUDE ///////////

#ifdef WIN32
#include <ovrvision_pro.h>
#include <ovrvision_ar.h>
#include "ovrvision_setting.h"
#include "ovrvision_calibration.h"
#elif MACOSX
//config gdata class
#include "ovrvision_setting.h"
#include "ovrvision_calibration.h"
#undef _OVRVISION_EXPORTS
#include "ovrvision_pro.h"
#include "ovrvision_ar.h"
#endif

/////////// VARS AND DEFS ///////////

//Exporter
#ifdef WIN32
#define CSHARP_EXPORT __declspec(dllexport)
#elif MACOSX
#define CSHARP_EXPORT 
#endif

//AR deta size
#define FLOATDATA_DATA_OFFSET	(10)

//Main Ovrvision Object
static OVR::OvrvisionPro* g_ovOvrvision = NULL;	// Always open
//AR Ovrvision Object
static OVR::OvrvisionAR* g_ovOvrvisionAR = NULL;
//Calibration Ovrvision Object
static OVR::OvrvisionCalibration* g_ovOvrvisionCalib = NULL;

/////////// EXPORT FUNCTION ///////////
#if 0
//C language
#ifdef __cplusplus
extern "C" {
#endif

// int ovOpen(void)
CSHARP_EXPORT int ovOpen(int locationID, float arMeter, int hmdType)
{
	//Create object
	if(g_ovOvrvision==NULL)
		g_ovOvrvision = new OVR::Ovrvision();	//MainVideo

	//Ovrvision Open
	if(g_ovOvrvision->Open(locationID,OVR::OV_CAMVGA_FULL, (OVR::HmdProp)hmdType)!=0)
		return 1;

	//Create AR object
	if(g_ovOvrvisionAR==NULL)
		g_ovOvrvisionAR = new OVR::OvrvisionAR(arMeter, g_ovOvrvision->GetImageWidth(),
														 g_ovOvrvision->GetImageHeight(),
														 g_ovOvrvision->GetFocalPoint());	//AR

	return 0;	//S_OK
}

// int ovClose(void)
CSHARP_EXPORT int ovClose(void)
{
	//Delete
	if(g_ovOvrvision==NULL)
		return 1;

	//Close Ovrvision
	g_ovOvrvision->Close();

	if(g_ovOvrvisionAR) {
		delete g_ovOvrvisionAR;
		g_ovOvrvisionAR = NULL;
	}

	return 0;	//S_OK
}

// int ovPreStoreCamData() -> need ovGetCamImage : ovGetCamImageBGR
CSHARP_EXPORT void ovPreStoreCamData()
{
	g_ovOvrvision->PreStoreCamData();	//Renderer
}

// int ovGetCamImage(unsigned char* pImage, int eye, int qt)
CSHARP_EXPORT void ovGetCamImage(unsigned char* pImage, int eye, int qt)
{
	if(g_ovOvrvision==NULL)
		return;

	//Get image
	g_ovOvrvision->GetCamImage(pImage, (OVR::Cameye)eye, (OVR::OvPSQuality)qt);

}
// int ovGetCamImageBGR(unsigned char* pImage, int eye, int qt)
CSHARP_EXPORT void ovGetCamImageBGR(unsigned char* pImage, int eye, int qt)
{
	if(g_ovOvrvision==NULL)
		return;

	//local var
	int i;

	//Get image
	unsigned char* pData = g_ovOvrvision->GetCamImage((OVR::Cameye)eye, (OVR::OvPSQuality)qt);

	int length = g_ovOvrvision->GetBufferSize();
	int offsetlen = g_ovOvrvision->GetPixelSize();

	//Image copy
	for (i = 0; i < length; i+=offsetlen) {
		//Left Eye
		pImage[i+0] = pData[i+2];	//B
		pImage[i+1] = pData[i+1];	//G
		pImage[i+2] = pData[i+0];	//R
	}
}

// void ovGetCamImageForUnity(unsigned char* pImagePtr_Left, unsigned char* pImagePtr_Right, int qt)
CSHARP_EXPORT void ovGetCamImageForUnity(unsigned char* pImagePtr_Left,
										 unsigned char* pImagePtr_Right, int qt)
{
	if(g_ovOvrvision==NULL)
		return;

	//local var
	int i, n;

	//Get image
	g_ovOvrvision->PreStoreCamData();	//Renderer
	unsigned char* pLeft = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_LEFT, (OVR::OvPSQuality)qt);
	unsigned char* pRight = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_RIGHT, (OVR::OvPSQuality)qt);

	int length = g_ovOvrvision->GetBufferSize();
	int offsetlen = g_ovOvrvision->GetPixelSize();

	//Image copy
	n = 0;
	for (i = 0; i < length; i+=offsetlen) {
		//Left Eye
		pImagePtr_Left[n+0] = pLeft[i+0];	//R
		pImagePtr_Left[n+1] = pLeft[i+1];	//G
		pImagePtr_Left[n+2] = pLeft[i+2];	//B
		pImagePtr_Left[n+3] = 255;			//A
		n+=4;
	}

	n = 0;
	for (i = 0; i < length; i+=offsetlen) {
		//Right Eye
		pImagePtr_Right[n+0] = pRight[i+0];
		pImagePtr_Right[n+1] = pRight[i+1];
		pImagePtr_Right[n+2] = pRight[i+2];
		pImagePtr_Right[n+3] = 255;
		n+=4;
	}
}

//AR
CSHARP_EXPORT void ovGetCamImageWithAR(unsigned char* pImage, int eye, int qt)
{
	if(g_ovOvrvision==NULL || g_ovOvrvisionAR==NULL)
		return;

	ovGetCamImage(pImage, (OVR::Cameye)eye, (OVR::OvPSQuality)qt);
	g_ovOvrvisionAR->SetImageRGB(pImage);
}
CSHARP_EXPORT void ovGetCamImageBGRWithAR(unsigned char* pImage, int eye, int qt)
{
	if(g_ovOvrvision==NULL || g_ovOvrvisionAR==NULL)
		return;

	ovGetCamImageBGR(pImage, (OVR::Cameye)eye, (OVR::OvPSQuality)qt);
	g_ovOvrvisionAR->SetImageRGB(pImage);
}
CSHARP_EXPORT void ovGetCamImageForUnityWithAR(unsigned char* pImagePtr_Left,
												unsigned char* pImagePtr_Right, int qt)
{
	if(g_ovOvrvision==NULL || g_ovOvrvisionAR==NULL)
		return;

	//local var
	int i, n = 0;

	//Get image
	g_ovOvrvision->PreStoreCamData();	//Renderer
	unsigned char* pLeft = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_LEFT, (OVR::OvPSQuality)qt);
	unsigned char* pRight = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_RIGHT, (OVR::OvPSQuality)qt);

	int length = g_ovOvrvision->GetBufferSize();
	int offsetlen = g_ovOvrvision->GetPixelSize();

	//Image copy
	for (i = 0; i < length; i+=offsetlen) {
		//Left Eye
		pImagePtr_Left[n+0] = pLeft[i+0];	//R
		pImagePtr_Left[n+1] = pLeft[i+1];	//G
		pImagePtr_Left[n+2] = pLeft[i+2];	//B
		pImagePtr_Left[n+3] = 255;			//A
		//Right Eye
		pImagePtr_Right[n+0] = pRight[i+0];
		pImagePtr_Right[n+1] = pRight[i+1];
		pImagePtr_Right[n+2] = pRight[i+2];
		pImagePtr_Right[n+3] = 255;

		n+=4;
	}

	g_ovOvrvisionAR->SetImageRGB(pLeft);
}

//This method will be detected if a hand is put in front of a camera. 
CSHARP_EXPORT int ovPutHandInFrontOfCamera(unsigned char thres_less, unsigned char* pImageBuf)
{
	return (int)g_ovOvrvision->PutHandInFrontOfCamera(thres_less, pImageBuf);
}

//Get image width
CSHARP_EXPORT int ovGetImageWidth()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetImageWidth();
}

//Get image height
CSHARP_EXPORT int ovGetImageHeight()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetImageHeight();
}

//Get image framerate
CSHARP_EXPORT int ovGetImageRate()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetImageRate();
}

//Get buffer size
CSHARP_EXPORT int ovGetBufferSize()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetBufferSize();
}


//Set exposure
CSHARP_EXPORT void ovSetExposure(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetExposure(value);
}

//Set white balance
CSHARP_EXPORT void ovSetWhiteBalance(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetWhiteBalance(value);
}

//Set contrast ( manual only )
CSHARP_EXPORT void ovSetContrast(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetContrast(value);
}

//Set Saturation ( manual only )
CSHARP_EXPORT void ovSetSaturation(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetSaturation(value);
}

//Set brightness ( manual only )
CSHARP_EXPORT void ovSetBrightness(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetBrightness(value);
}

//Set sharpness ( manual only )
CSHARP_EXPORT void ovSetSharpness(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetSharpness(value);
}

//Set gamma ( manual only )
CSHARP_EXPORT void ovSetGamma(int value)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->SetGamma(value);
}


//Get exposure
CSHARP_EXPORT int ovGetExposure()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetExposure();
}

//Get white balance
CSHARP_EXPORT int ovGetWhiteBalance()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetWhiteBalance();
}

//Get contrast
CSHARP_EXPORT int ovGetContrast()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetContrast();
}

//Get saturation
CSHARP_EXPORT int ovGetSaturation()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetSaturation();
}

//Get brightness
CSHARP_EXPORT int ovGetBrightness()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetBrightness();
}

//Get sharpness
CSHARP_EXPORT int ovGetSharpness()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetSharpness();
}

//Get gamma
CSHARP_EXPORT int ovGetGamma()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetGamma();
}

//Get focalPoint
CSHARP_EXPORT float ovGetFocalPoint()
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetFocalPoint();
}

//Get Oculus Right-eye Gap
CSHARP_EXPORT float ovGetOculusRightGap(int at)
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->GetOculusRightGap(at);
}


//Set parameter
CSHARP_EXPORT int ovSetParamXMLfromFile(unsigned char* filename)
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->SetParamXMLfromFile((char*)filename);
}

//Save parameter
CSHARP_EXPORT int ovSaveParamXMLtoFile(unsigned char* savefilename)
{
	if(g_ovOvrvision==NULL)
		return 0;

	return g_ovOvrvision->SaveParamXMLtoFile((char*)savefilename);
}

//Save tmp parameter : csharp only
CSHARP_EXPORT void ovSaveParamXMLtoTempFile(int* config1, float* config2)
{
	if(g_ovOvrvision==NULL)
		return;

	g_ovOvrvision->DirectSaveParamXMLtoTempFile(config1, config2);
}

////////////// Ovrvision AR //////////////

// ovARSetImage(unsigned char* pImgSrc)
CSHARP_EXPORT void ovARSetImage(unsigned char* pImgSrc)
{
	if(g_ovOvrvisionAR==NULL)
		return;

	g_ovOvrvisionAR->SetImageRGB(pImgSrc);
}

// void ovARRender(void)
CSHARP_EXPORT void ovARRender()
{
	if(g_ovOvrvisionAR==NULL)
		return;

	//Rendering
	g_ovOvrvisionAR->Render();
}

// int ovARGetData(float* mdata, int datasize)
CSHARP_EXPORT int ovARGetData(float* mdata, int datasize)
{
	int i;
	if(g_ovOvrvisionAR==NULL)
		return (-1);

	int marklen = g_ovOvrvisionAR->GetMarkerDataSize();
	OVR::OvMarkerData* dt = g_ovOvrvisionAR->GetMarkerData();

	for(i = 0; i < marklen; i++)
	{
		int ioffset = i * FLOATDATA_DATA_OFFSET;
		if(i >= (datasize / FLOATDATA_DATA_OFFSET))
			break;

		mdata[ioffset+0] = (float)dt[i].id;
		mdata[ioffset+1] = dt[i].translate.x;
		mdata[ioffset+2] = dt[i].translate.y;
		mdata[ioffset+3] = dt[i].translate.z;
		mdata[ioffset+4] = dt[i].quaternion.x;
		mdata[ioffset+5] = dt[i].quaternion.y;
		mdata[ioffset+6] = dt[i].quaternion.z;
		mdata[ioffset+7] = dt[i].quaternion.w;
		mdata[ioffset+8] = dt[i].centerPtOfImage.x;
		mdata[ioffset+9] = dt[i].centerPtOfImage.y;
	}

	return marklen;	//S_OK
}

// void ovARSetMarkerSize(float value)
CSHARP_EXPORT void ovARSetMarkerSize(float value)
{
	if(g_ovOvrvisionAR==NULL)
		return;

	g_ovOvrvisionAR->SetMarkerSizeMeter(value);
}

// float ovARGetMarkerSize()
CSHARP_EXPORT float ovARGetMarkerSize()
{
	if(g_ovOvrvisionAR==NULL)
		return 0;

	return g_ovOvrvisionAR->GetMarkerSizeMeter();
}

// void ov3DInstantTraking_Metaio(int value)
CSHARP_EXPORT void ov3DInstantTraking_Metaio(int value)
{
	if(g_ovOvrvisionAR==NULL)
		return;

	g_ovOvrvisionAR->SetInstantTraking((bool)value);
}

////////////// Ovrvision Calibration //////////////

CSHARP_EXPORT void ovCalibInitialize(int pattern_size_w, int pattern_size_h, double chessSizeMM)
{
	if(g_ovOvrvision == NULL)
		return;

	if(g_ovOvrvisionCalib)
		delete g_ovOvrvisionCalib;

	g_ovOvrvisionCalib = new OVR::OvrvisionCalibration(
		g_ovOvrvision->GetImageWidth(),g_ovOvrvision->GetImageHeight(),
		pattern_size_w,pattern_size_h,chessSizeMM);
}

CSHARP_EXPORT int ovCalibFindChess(int hmdType)
{
	if(g_ovOvrvisionCalib == NULL)
		return 0;

	g_ovOvrvision->PreStoreCamData();	//ReRenderer
	unsigned char* pLeft = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_LEFT, OVR::OV_PSQT_NONE);
	unsigned char* pRight = g_ovOvrvision->GetCamImage(OVR::OV_CAMEYE_RIGHT, OVR::OV_PSQT_NONE);

	if(hmdType==OVR::OV_HMD_OCULUS_DK2)
		return g_ovOvrvisionCalib->FindChessBoardCorners(pRight,pLeft);
	else
		return g_ovOvrvisionCalib->FindChessBoardCorners(pLeft,pRight);
}

CSHARP_EXPORT void ovCalibSolveStereoParameter()
{
	if(g_ovOvrvisionCalib == NULL)
		return;

	g_ovOvrvisionCalib->SolveStereoParameter();
	g_ovOvrvisionCalib->SaveCalibrationParameter(NULL);	//default

}

CSHARP_EXPORT int ovCalibGetImageCount()
{
	if(g_ovOvrvisionCalib == NULL)
		return -1;

	return g_ovOvrvisionCalib->GetImageCount();
}


#ifdef __cplusplus
}
#endif
#endif