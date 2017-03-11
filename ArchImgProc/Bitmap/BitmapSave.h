#pragma once

#include <memory>
#include "utils.h"

namespace ArchImgProc
{
	class CSaveBitmap
	{
	private:
		static void SaveWithWIC(IWICImagingFactory* pFactory, IWICStream* destStream, const GUID encoder, const WICPixelFormatGUID& wicPixelFmt, std::shared_ptr<ArchImgProc::IBitmapData> spBitmap)
		{
			// cf. http://msdn.microsoft.com/en-us/library/windows/desktop/ee719797(v=vs.85).aspx
			/*CComPtr<IWICImagingFactory> cpWicImagingFactor;
			HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&cpWicImagingFactor);
			ThrowIfFailed("Creating WICImageFactory", hr);*/

			CComPtr<IWICBitmapEncoder> wicBitmapEncoder;
			HRESULT hr = pFactory->CreateEncoder(
				/*GUID_ContainerFormatPng*//*GUID_ContainerFormatTiff*//*GUID_ContainerFormatWmp*/encoder,
				nullptr,    // No preferred codec vendor.
				&wicBitmapEncoder);
			Utils::ThrowIfFailed("Creating IWICImagingFactory::CreateEncoder", hr);

			// Create encoder to write to image file
			hr = wicBitmapEncoder->Initialize(destStream, WICBitmapEncoderNoCache);
			Utils::ThrowIfFailed("IWICBitmapEncoder::Initialize", hr);

			CComPtr<IWICBitmapFrameEncode> frameEncode;
			hr = wicBitmapEncoder->CreateNewFrame(&frameEncode, nullptr);
			Utils::ThrowIfFailed("IWICBitmapEncoder::CreateNewFrame", hr);

			hr = frameEncode->Initialize(nullptr);
			Utils::ThrowIfFailed("IWICBitmapFrameEncode::CreateNewFrame", hr);

			hr = frameEncode->SetSize(spBitmap->GetWidth(), spBitmap->GetHeight());
			Utils::ThrowIfFailed("IWICBitmapFrameEncode::SetSize", hr);

			WICPixelFormatGUID pixelFormat = wicPixelFmt;/*GUID_WICPixelFormat32bppGrayFloat;*/
			hr = frameEncode->SetPixelFormat(&pixelFormat);
			Utils::ThrowIfFailed("IWICBitmapFrameEncode::SetPixelFormat", hr);

			//const char* pixelFormatActual = Utils::WICPixelFormatToInformalString(pixelFormat);

			auto bitmapData = spBitmap->Lock();
			hr = frameEncode->WritePixels(spBitmap->GetHeight(), bitmapData.pitch, spBitmap->GetHeight()* bitmapData.pitch, (BYTE*)bitmapData.ptrDataRoi);
			spBitmap->Unlock();
			Utils::ThrowIfFailed("IWICBitmapFrameEncode::WritePixels", hr);

			hr = frameEncode->Commit();
			Utils::ThrowIfFailed("IWICBitmapFrameEncode::Commit", hr);

			hr = wicBitmapEncoder->Commit();
			Utils::ThrowIfFailed("IWICBitmapEncoder::Commit", hr);
		}

		static void SaveWithWIC(const wchar_t* filename, const GUID encoder, const WICPixelFormatGUID& wicPixelFmt, std::shared_ptr<ArchImgProc::IBitmapData> bitmap)
		{
			CComPtr<IWICImagingFactory> cpWicImagingFactor;
			HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&cpWicImagingFactor);
			Utils::ThrowIfFailed("Creating WICImageFactory", hr);

			CComPtr<IWICStream> stream;
			// Create a stream for the encoder
			hr = cpWicImagingFactor->CreateStream(&stream);
			Utils::ThrowIfFailed("IWICImagingFactory::CreateStream", hr);

			// Initialize the stream using the output file path
			hr = stream->InitializeFromFilename(filename, GENERIC_WRITE);
			Utils::ThrowIfFailed("IWICStream::InitializeFromFilename", hr);

			SaveWithWIC(cpWicImagingFactor, stream, encoder, wicPixelFmt, bitmap);

			hr = stream->Commit(STGC_DEFAULT);
			Utils::ThrowIfFailed("IWICStream::Commit", hr, [](HRESULT ec) {return SUCCEEDED(ec) || ec == E_NOTIMPL; });
		}
	public:
		static void SaveBitmapAsPng(std::shared_ptr<ArchImgProc::IBitmapData> bitmap, const wchar_t* szwFilename)
		{
			if (bitmap->GetPixelType() == PixelType::BGR24)
			{
				SaveWithWIC(szwFilename, GUID_ContainerFormatPng, GUID_WICPixelFormat24bppBGR, bitmap);
			}
			else if (bitmap->GetPixelType() == PixelType::Gray8)
			{
				SaveWithWIC(szwFilename, GUID_ContainerFormatPng, GUID_WICPixelFormat8bppGray, bitmap);
			}
			else
			{
				throw std::invalid_argument("Unsupported pixeltype");
			}
		}
	};
}