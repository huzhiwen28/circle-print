//机器主板序列号和硬盘序列号实现
#define _WIN32_WINNT 0x0400
#define _WIN32_DCOM

//#include <Windows.h>


#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include "pcfingerprint.h"
#pragma comment (lib, "comsuppw.lib")
#pragma comment (lib, "wbemuuid.lib")

using namespace std;

pcfingerprint::pcfingerprint()
{

}

pcfingerprint::~pcfingerprint()
{

}

//取得机器指纹
int pcfingerprint::getpcfingerprint(char* cpfingerprint,int* len)
{
	if (cpfingerprint == NULL)
	{
		//防御式编程，参数为NULL
		return 1;
	}

	HRESULT hres;

	*len = 0;

	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	/*
	hres =  CoInitializeEx(0, COINIT_MULTITHREADED); 

	if (hres == S_OK)
	{
		std::cout << "S_OK" << std::endl;
    }
	else if (hres == S_FALSE)
	{
		std::cout << "S_FALSE" << std::endl;
	}
	else if( hres == RPC_E_CHANGED_MODE)
	{
		std::cout << "RPC_E_CHANGED_MODE" << std::endl;
	}
	else
	{
		std::cout << "donot know reason" << std::endl;
	}

	if (FAILED(hres))
	{
		cout << "Failed to initialize COM library. Error code = 0x" 
			<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
	// Note: If you are using Windows 2000, you need to specify -
	// the default authentication credentials for a user by using
	// a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
	// parameter of CoInitializeSecurity ------------------------

	hres =  CoInitializeSecurity(
		NULL, 
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);


	if (FAILED(hres))
	{
		cout << "Failed to initialize security. Error code = 0x" 
			<< hex << hres << endl;
		CoUninitialize();
		return 1;                    // Program has failed.
	}
	*/

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,             
		0, 
		CLSCTX_INPROC_SERVER, 
		IID_IWbemLocator, (LPVOID *) &pLoc);

	if (FAILED(hres))
	{
		cout << "Failed to create IWbemLocator object."
			<< " Err code = 0x"
			<< hex << hres << endl;
		//CoUninitialize();
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
		cout << "Could not connect. Error code = 0x" 
			<< hex << hres << endl;
		pLoc->Release();     
		//CoUninitialize();
		return 1;                // Program has failed.
	}


	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
		cout << "Could not set proxy blanket. Error code = 0x" 
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();     
		//CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t("SELECT * FROM Win32_BaseBoard WHERE (SerialNumber IS NOT NULL)"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x" 
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		//CoUninitialize();
		return 1;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	if (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn)
		{
			;
		}
		else
		{
			VARIANT vtProp;

			// Get the value of the Name property
			hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
			lstrcpy((LPWSTR)cpfingerprint,vtProp.bstrVal);
			*len += lstrlen(vtProp.bstrVal)*2;

			VariantClear(&vtProp);

			pclsObj->Release();

		}
	}

	hres = pSvc->ExecQuery(
		bstr_t("WQL"), 
		bstr_t("SELECT * FROM Win32_DiskDrive WHERE (Signature IS NOT NULL)"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		cout << "Query for operating system name failed."
			<< " Error code = 0x" 
			<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		//CoUninitialize();
		return 1;               // Program has failed.
	}

	//第一个盘的就是
	if (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
			&pclsObj, &uReturn);

		if(0 == uReturn)
		{
			;
		}
		else
		{
			VARIANT vtProp;

			// Get the value of the Name property
			hr = pclsObj->Get(L"Signature", 0, &vtProp, 0, 0);
			int * ip = (int*) (cpfingerprint + (*len));
			*ip = vtProp.uintVal;
			*len += 4;

			VariantClear(&vtProp);

			pclsObj->Release();
		}
	}

	// Cleanup
	// ========

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	//CoUninitialize();
	return 0;
}

