#pragma once
//#include "stdafx.h"
#include "tbsfunc.h"
//#include <streams.h>
typedef struct{
	IEnumMoniker*  pIEnumMoniker;
	ICreateDevEnum* pICreateDevEnum;
	IMoniker *pIMoniker;
	char szName[255];
	wchar_t szNameW[255];	
} TDSEnum;


TDSEnum * enum_create(REFCLSID clsid);
HRESULT enum_next(TDSEnum *pEnum);
//HRESULT enum_next();
HRESULT enum_free(TDSEnum *pEnum);
HRESULT enum_get_filter(TDSEnum *pEnum, IBaseFilter **ppFilter);
HRESULT enum_get_name(TDSEnum *pEnum);
