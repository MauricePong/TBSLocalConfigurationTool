#include "dsenum.h"

TDSEnum *  enum_create(REFCLSID clsid)
{
	TDSEnum *p = (TDSEnum *) malloc(sizeof(TDSEnum));

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum,  (void **) &p->pICreateDevEnum);
   	if (FAILED (hr))
   	{
       	qDebug(("FAIL: enum_create(): Cannot CoCreate ICreateDevEnum"));
		free(p);
		p = NULL;
       	return NULL;
   	}

    // obtain the enumerator
    hr = p->pICreateDevEnum->CreateClassEnumerator(clsid, &p->pIEnumMoniker, 0);
    // the call can return S_FALSE if no moniker exists, so explicitly check S_OK
    if (FAILED (hr))
    {
        qDebug("FAIL: enum_create(): Cannot CreateClassEnumerator");
		free(p);
		p = NULL;
        return NULL;
    }
    if (S_OK != hr)  // Class not found
    {
        qDebug("FAIL: enum_create(): Class not found, CreateClassEnumerator returned S_FALSE");
		free(p);
		p = NULL;
        return NULL;
	}

	p->pIMoniker = NULL;
	
	return p;
	
	return NULL;
}

//HRESULT enum_next()

HRESULT enum_next(TDSEnum *pEnum)
{	
	
	if (!pEnum) return S_FALSE;	
	
	if (pEnum->pIMoniker) {
		pEnum->pIMoniker->Release();
		pEnum->pIMoniker = NULL;
	}
	return pEnum->pIEnumMoniker->Next(1, &pEnum->pIMoniker, 0);

	return S_OK;
}



HRESULT enum_free(TDSEnum *pEnum)
{	
	if (!pEnum) return S_OK;

	if (pEnum->pIMoniker) {
		pEnum->pIMoniker->Release();
		pEnum->pIMoniker = NULL;
	}


	if (pEnum->pICreateDevEnum)
		pEnum->pICreateDevEnum->Release();
	
	if (pEnum->pIEnumMoniker)
		pEnum->pIEnumMoniker->Release();

	free(pEnum);

	return S_OK;
}


HRESULT enum_get_filter(TDSEnum *pEnum, IBaseFilter **ppFilter)
{
	*ppFilter = NULL;
	
	if (!ppFilter)
			return S_FALSE;

//	IBaseFilter *pFilter = NULL;	

	// bind the filter        
    HRESULT hr = pEnum->pIMoniker->BindToObject(
                                    NULL, 
                                    NULL, 
                                    IID_IBaseFilter,
                                    reinterpret_cast<void**>(ppFilter)
                                    );
	if (!*ppFilter)
			return S_FALSE;



	

	//hr = pFilter->QueryInterface (IID_IBaseFilter, (void **) ppFilter);
	//*ppFilter = pFilter;
	return hr; 
}



HRESULT enum_get_name(TDSEnum *pEnum)
{
	// obtain filter's friendly name
    IPropertyBag *pBag;
    HRESULT hr =pEnum->pIMoniker->BindToStorage(
                                NULL, 
                                NULL, 
                                IID_IPropertyBag,
                                reinterpret_cast<void**>(&pBag)
                                );

    if(FAILED(hr))
    {

        return hr;
    }


   	VARIANT varBSTR;
	varBSTR.vt = VT_BSTR;		
	VariantInit(&varBSTR);			

	hr = pBag->Read(L"FriendlyName", &varBSTR, NULL);	

    if(FAILED(hr))
    {

		pBag->Release();		
		VariantClear(&varBSTR);
		return hr;
    }
	
	wcstombs(pEnum->szName, varBSTR.bstrVal, 255);
    wcscpy(pEnum->szNameW, varBSTR.bstrVal);
    //strcpy(pEnum->szNameW, varBSTR.bstrVal);
	
	pBag->Release();		
	VariantClear(&varBSTR);

	return S_OK;
}



