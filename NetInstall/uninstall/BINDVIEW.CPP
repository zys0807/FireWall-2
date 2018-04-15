//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 2001.
//
//  File:       B I N D V I E W . C P P
//
//  Contents:  
//
//  Notes:      
//
//  Author:     Alok Sinha    15-Amy-01
//
//----------------------------------------------------------------------------


#include "BindView.h"

//----------------------------------------------------------------------------
// Globals
//

//
// Image list for devices of various setup class.
//

SP_CLASSIMAGELIST_DATA ClassImageListData;

HINSTANCE              hInstance;
HMENU                  hMainMenu;
HMENU                  hComponentSubMenu;
HMENU                  hBindingPathSubMenu;

//
// Network components whose bindings are enumerated.
//

LPWSTR   lpszNetClass[] = {
                    L"All Clients",
                    L"All Services",
                    L"All Protocols"
         };

//
// GUIDs of network components.
//

const GUID     *pguidNetClass [] = {
                     &GUID_DEVCLASS_NETCLIENT,
                     &GUID_DEVCLASS_NETSERVICE,
                     &GUID_DEVCLASS_NETTRANS,
                     &GUID_DEVCLASS_NET
         };



DWORD GetServiceInfFilePath_Passthru( 
									 LPTSTR lpFilename,
									 DWORD nSize
									 )
{
	// Get Path to This Module
	DWORD    nResult;
	TCHAR    szDrive[ _MAX_DRIVE ];
	TCHAR    szDir[ _MAX_DIR ];
	
	nResult = GetModuleFileName( NULL, lpFilename, nSize );
	
	if( nResult == 0 )
	{
		return 0;
	}
	
	_wsplitpath( lpFilename, szDrive, szDir, NULL, NULL );
	
	_wmakepath( lpFilename, szDrive, szDir, L"netsf", L".inf");
	
	return (DWORD )wcslen( lpFilename );
}

DWORD GetServiceInfFilePath_PassthruMP( 
									   LPTSTR lpFilename,
									   DWORD nSize
									   )
{
	// Get Path to This Module
	DWORD    nResult;
	TCHAR    szDrive[ _MAX_DRIVE ];
	TCHAR    szDir[ _MAX_DIR ];
	
	nResult = GetModuleFileName( NULL, lpFilename, nSize );	
	if( nResult == 0 )
	{
		return 0;
	}
	
    _wsplitpath( lpFilename, szDrive, szDir, NULL, NULL );
	
    _wmakepath( lpFilename, szDrive, szDir, L"netsf_m", L".inf");
	
	return (DWORD )wcslen( lpFilename );
}



DWORD UninstallDriver_Passthru()
{	
	INetCfg               *pnc;
	INetCfgComponent      *pncc;
	INetCfgClass          *pncClass;
	INetCfgClassSetup     *pncClassSetup;
	LPTSTR                lpszApp;
	GUID                  guidClass;
	OBO_TOKEN             obo;
	HRESULT               hr;
	int					  flag=0;
	
	hr = HrGetINetCfg( TRUE, APP_NAME, &pnc, &lpszApp );
	if ( hr == S_OK ) 
	{
		//
	    // Get a reference to the network component to uninstall.
		//
		hr = pnc->FindComponent( L"ms_passthru", &pncc );	  
		if ( hr == S_OK )
		{
			//
			// Get the class GUID.
			//
			hr = pncc->GetClassGuid( &guidClass );
			
			if ( hr == S_OK )
			{
				//
				// Get a reference to component's class.
				//
				
				hr = pnc->QueryNetCfgClass( &guidClass,
					IID_INetCfgClass,
					(PVOID *)&pncClass );
				if ( hr == S_OK )
				{
					//
					// Get the setup interface.
					//
					
					hr = pncClass->QueryInterface( IID_INetCfgClassSetup,
						(LPVOID *)&pncClassSetup );
					
					if ( hr == S_OK )
					{
						//
						// Uninstall the component.
						//
						
						ZeroMemory( &obo,
							sizeof(OBO_TOKEN) );
						
						obo.Type = OBO_USER;
						
						hr = pncClassSetup->DeInstall( pncc,
							&obo,
							NULL );
						if ( (hr == S_OK) || (hr == NETCFG_S_REBOOT) )
						{
							hr = pnc->Apply();
							
							if ( (hr != S_OK) && (hr != NETCFG_S_REBOOT) )
							{
								//MessageBoxW(NULL, L"ж������֮���޷��ɹ�Ӧ��!", L"Passthru��װ���������ʾ",MB_OK);
								flag =  -1;
							}else
							{
								//MessageBoxW(NULL, L"�ɹ�ж������!", L"Passthru��װ������ʾ", MB_OK);
								flag = 1;
							}
						}
						else
						{
							//MessageBoxW(NULL, L"ж���������ʧ��!", L"Passthru��װ���������ʾ", MB_OK);
							flag = -2;
						}
						
						ReleaseRef( pncClassSetup );
					}
					else
					{
						//MessageBoxW(NULL, L"�޷��õ���װ��ӿ�!", L"Passthru��װ���������ʾ", MB_OK);
						flag =  -3;
					}
					
					ReleaseRef( pncClass );
				}
				else
				{
					//MessageBoxW(NULL, L"�޷��õ���װ��ӿ�!", L"Passthru��װ���������ʾ", MB_OK);
					flag = -4;
				}
			}
			else
			{
				//MessageBoxW(NULL, L"�޷��õ���װ��ӿڵ� GUID!", L"Passthru��װ���������ʾ",MB_OK);
				flag = -5;
			}
			
			ReleaseRef( pncc );
		}
		else
		{			
			//MessageBoxW(NULL, L"�޷��õ�һ���ӿ�ָ�룡", L"��װ���������ʾ", MB_OK);
			flag = -6;
		}
		
		HrReleaseINetCfg( pnc,
			TRUE );
	}
	else
	{
		if ( (hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp )
		{
			//   ErrMsg( hr,
			//        L"%s currently holds the lock, try later.",
			//       lpszApp );
			//MessageBoxW(NULL, L"�����������⣬�Ժ����ԣ�", L"Passthru��װ���������ʾ", MB_OK);
			CoTaskMemFree( lpszApp );
			flag = -7;
		}
		else
		{
			//MessageBoxW(NULL, L"�޷��õ�֪ͨ����ӿڣ�", L"Passthru��װ���������ʾ", MB_OK);
			flag =  -8;
		}
	}
										
	return flag;
}


//
// Program entry point.
//

int __cdecl main()
{
     return UninstallDriver_Passthru();
}