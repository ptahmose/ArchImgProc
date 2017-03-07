#pragma once

#include <Wincodec.h>
#include <atlbase.h>

namespace ArchImgProc
{
	inline void ThrowIfFailed(const char* function, HRESULT hr)
	{
		if (FAILED(hr))
		{
			char errorMsg[255];
			_snprintf_s(errorMsg, _TRUNCATE, "COM-ERROR hr=0x%08X (%s)", hr, function);
			throw std::runtime_error(errorMsg);
		}
	}

	inline CComPtr<IWICBitmapSource> Convert(IWICImagingFactory* pImgFactory, CComPtr<IWICBitmapFrameDecode> src)
	{
		WICPixelFormatGUID fmt;
		HRESULT hr = src->GetPixelFormat(&fmt);
		if (IsEqualGUID(fmt, GUID_WICPixelFormat24bppBGR))
		{
			return CComPtr<IWICBitmapSource>(src);
		}

		CComPtr<IWICFormatConverter> cpConvertedFrame;
		hr = pImgFactory->CreateFormatConverter(&cpConvertedFrame);
		ThrowIfFailed("IWICImagingFactory::CreateFormatConverter", hr);

		hr = cpConvertedFrame->Initialize(src, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);

		return CComPtr<IWICBitmapSource>(cpConvertedFrame);
	}

	inline void CopyToRgb24Bitmap(IWICBitmapSource* pSrc, std::shared_ptr<ArchImgProc::IBitmapData> dest)
	{
		// precondition: pSrc is either 24bppBGR, and dest is float
		UINT nWidth, nHeight;
		HRESULT hr = pSrc->GetSize(&nWidth, &nHeight);
		WICPixelFormatGUID pixelFormat;
		hr = pSrc->GetPixelFormat(&pixelFormat);
		UINT bytesPerPel;
		if (!IsEqualGUID(pixelFormat, GUID_WICPixelFormat24bppBGR))
		{
			throw std::runtime_error("Illegal pixeltype.");
		}

		ArchImgProc::BitmapLockInfo lockInfo = dest->Lock();
		WICRect rect = { 0, 0, nWidth, nHeight };
		hr = pSrc->CopyPixels(&rect, lockInfo.pitch, lockInfo.pitch*nHeight, (BYTE*)lockInfo.ptrDataRoi);
		dest->Unlock();
	}

	inline std::shared_ptr<ArchImgProc::IBitmapData> LoadBitmapFromFile(const wchar_t* szwFilename)
	{
		CComPtr<IWICImagingFactory> cpWicImagingFactory;
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&cpWicImagingFactory);
		ThrowIfFailed("Creating WICImageFactory", hr);

		CComPtr<IWICBitmapDecoder> cpDecoder;
		CComPtr<IWICBitmapFrameDecode> cpFrame;

		// Create a decoder for the given image file
		hr = cpWicImagingFactory->CreateDecoderFromFilename(szwFilename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &cpDecoder);
		ThrowIfFailed("IWICImagingFactory::CreateDecoderFromFilename", hr);

		// Retrieve the first frame of the image from the decoder
		hr = cpDecoder->GetFrame(0, &cpFrame);
		ThrowIfFailed("IWICBitmapDecoder::GetFrame", hr);

		/*IWICColorContext* colorContexts[1];
		cpWicImagingFactory->CreateColorContext(&(colorContexts[0]));
		UINT actualCount;
		cpFrame->GetColorContexts(1, colorContexts, &actualCount);*/

		UINT nWidth, nHeight;
		// Retrieve the image dimensions
		hr = cpFrame->GetSize(&nWidth, &nHeight);
		ThrowIfFailed("IWICBitmapFrameDecode::GetSize", hr);

		// How WIC (IWICFormatConverter in particular) handles the conversion from an integer
		// format to 32bppGrayFloat is beyond me. I have no idea how the result is to be interpreted,
		// the only piece of information I could find is here: http://directxtex.codeplex.com/workitem/643
		// "Furthermore, Use of IWICFormatConverter implicitly assumes all integer content is sRGB and all fixed / float
		// content is scRGB so there's an implicit conversion of color space which is not desired."
		// Therefore we have to do the conversion by hand.

		WICRect rect = { 0, 0, nWidth, nHeight };
		CComPtr<IWICBitmapSource> spFormatConverted = Convert(cpWicImagingFactory, cpFrame);

		// Pixeltype of "spFormatConverted" is now guaranteed to be either GUID_WICPixelFormat8bppGray or GUID_WICPixelFormat16bppGray,
		// other types need to be added later.

		//hr = cpFrame->CopyPixels(&rect, nWidth, nHeight*nWidth, pbTest);

		//auto bd = func(PixelType::GrayFloat32, nWidth, nHeight);
		auto bd = CBitmapData<CHeapAllocator>::Create(PixelType::BGR24, nWidth, nHeight);

		CopyToRgb24Bitmap(spFormatConverted, bd);

		return bd;
	}
}