#include"device.h"
#include <dshow.h>
using namespace std;

namespace kc2{

device_info_list_t* device_list = nullptr;

bool is_read = false;

void to_wchar_from_bstr(BSTR str,wchar_t** wstr) {
    UINT length = SysStringLen(str) + 1;
    *wstr = new WCHAR[length + 1];
    wcscpy_s(*wstr, length, str);
}


// Release the format block for a media type.
void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // pUnk should not be used.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

// Delete a media type structure that was allocated on the heap.
void _DeleteMediaType(AM_MEDIA_TYPE* pmt)
{
    if (pmt != NULL)
    {
        _FreeMediaType(*pmt);
        CoTaskMemFree(pmt);
    }
}

HRESULT pin_info(IBaseFilter* pbf,video_info_list_t& vl) {
    HRESULT hr;

    IEnumPins* pEnum = NULL;
    IPin* pPin = NULL;


    hr = pbf->EnumPins(&pEnum);
    if (SUCCEEDED(hr))
        while (pEnum->Next(1, &pPin, 0) == S_OK)
        {

            IEnumMediaTypes* emtype = NULL;
            AM_MEDIA_TYPE* pmt = NULL;
            PIN_INFO pin_info;
            hr = pPin->QueryPinInfo(&pin_info);


            if (SUCCEEDED(hr)) {
                //pin_info
                
            }

            hr = pPin->EnumMediaTypes(&emtype);
            if (SUCCEEDED(hr) && pin_info.dir==PINDIR_OUTPUT)
                while (hr = emtype->Next(1, &pmt, NULL), hr == S_OK)
                {
                    
                    if (pmt->majortype == MEDIATYPE_Video) {

                        if ((pmt->formattype == FORMAT_VideoInfo)
                            && (pmt->cbFormat >= sizeof(VIDEOINFOHEADER))
                            && (pmt->pbFormat != NULL)) {

                            VIDEOINFOHEADER* pVih2 = (VIDEOINFOHEADER*)pmt->pbFormat;
                            
                            VideoInfo vinfo;
                            vinfo.width = pVih2->bmiHeader.biWidth;
                            vinfo.height = pVih2->bmiHeader.biHeight;
                            vinfo.fps = 1e7 / pVih2->AvgTimePerFrame;
                            vinfo.format = (pmt->subtype == MEDIASUBTYPE_MJPG);
                            vl.push_back(vinfo);

                        }

                    }
                    else
                    if (pmt->majortype == MEDIATYPE_Audio) {

                        if ((pmt->formattype == FORMAT_WaveFormatEx)
                            && (pmt->cbFormat >= sizeof(WAVEFORMATEX))
                            && (pmt->pbFormat != NULL)) {

                            WAVEFORMATEX* pVih2 = (WAVEFORMATEX*)pmt->pbFormat;
                        }

                    }
                    _DeleteMediaType(pmt);
                }
            emtype->Release();
            pPin->Release();
        }

    pEnum->Release();


    return S_OK;
}

HRESULT EnumerateDevices(REFGUID category, IEnumMoniker** ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum* pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}

void DeviceInformation(IEnumMoniker* pEnum)
{
    IMoniker* pMoniker = NULL;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag* pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue;
        }

        VARIANT var;
        VariantInit(&var);
        wchar_t* name=NULL;
        wchar_t* path=NULL;
        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr))
        {
            to_wchar_from_bstr( var.bstrVal,&name);
            VariantClear(&var);
        }

        hr = pPropBag->Write(L"FriendlyName", &var);




        hr = pPropBag->Read(L"DevicePath", &var, 0);
        if (SUCCEEDED(hr))
        {
            to_wchar_from_bstr(var.bstrVal, &path);

            VariantClear(&var);
        }

        DeviceInfo dinfo;
        dinfo.name = name;
        dinfo.path = path;
        dinfo.video_info_list = new video_info_list_t;

        IBaseFilter* pbf = NULL;
        pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pbf);


        pin_info(pbf,*dinfo.video_info_list);
        dinfo.video_info_list_size = dinfo.video_info_list->size();
        if(dinfo.video_info_list_size)
        device_list->push_back(dinfo);
        pbf->Release();

        pPropBag->Release();
        pMoniker->Release();
    }

}






EXPORT int kc2np_init_device_info_list(device_info_list_t** dlist) {
    
    if (device_list != nullptr) {
        *dlist = device_list;
        return device_list->size();
    }
    
    device_list = new device_info_list_t;

    HRESULT hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        IEnumMoniker* pEnum;

        hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);

        if (SUCCEEDED(hr))
        {

            DeviceInformation(pEnum);

            pEnum->Release();
        }

        hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);

        if (SUCCEEDED(hr))
        {

            DeviceInformation(pEnum);

            pEnum->Release();
        }

        CoUninitialize();

        is_read = true;

    }
    *dlist = device_list;
    return device_list->size();

}

EXPORT DeviceInfo kc2np_device_info_list_get_device_info(device_info_list_t* d,int index) {
    return d->at(index);
}

EXPORT VideoInfo kc2np_video_info_list_get_video_info(video_info_list_t* v, int index) {
    return v->at(index);
}


EXPORT int kc2np_device_info_get_near_size_video_info_index(DeviceInfo device_info,int width,int height) {
    
    if(device_info.video_info_list == nullptr)return -1;
    if(!device_info.video_info_list->size())return -1;

    int ans_index=0;
    size_t min_d = ULLONG_MAX;
    for (int i = 0; i < device_info.video_info_list_size; i++) {
        const VideoInfo& vi = device_info.video_info_list->at(i);

        size_t dw = (vi.width - width);
        size_t dh = (vi.height - height);
        size_t d = dw*dw + dh*dh;
        bool is_min = false;

        if (d < min_d) {
            is_min = true;
        }
        else if (d == min_d) {
            const VideoInfo& ans_vi = device_info.video_info_list->at(ans_index);
            if (ans_vi.fps < vi.fps) {
                is_min = true;
            }
            else if (ans_vi.fps == vi.fps) {
                
                if (vi.format == 0) {
                    is_min = true;
                }

            }
        }

        if (is_min) {
            min_d = d;
            ans_index = i;
        }
    }

    return ans_index;
}


EXPORT void test_d2() {
}


}