/***************************************************************************************************
  Author: Oso Oluwafemi Ebenezer
  Date: 19th of July, 2013
 ****************************************************************************************************/
#include "CreateShortcut.h"
int str_replace(LPCWSTR originalstring, wchar_t oldcharacter, wchar_t newcharacter, wchar_t* &newstring);
void remove_last_slash(LPCWSTR originalstring, wchar_t* &newstring);



CreateShortCut::CreateShortCut()
{
	CoInitializeEx( NULL, 0 );
	path = new wchar_t[MAX_PATH];
}

CreateShortCut::~CreateShortCut()
{
	CoUninitialize();
	delete [] path;
}

int CreateShortCut::CreateLinkFileBase(LPCSTR pathToObj, LPCSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate)
{
	return CreateLinkFileBaseA(pathToObj, pathToLink, description, hotkey, cmdLine, forceCreate);
}

int CreateShortCut::CreateLinkFileBase(LPCSTR pathToObj, LPCWSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate)
{
	char pathToLinkA[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pathToLink, -1, pathToLinkA, sizeof(pathToLinkA), NULL, NULL);
	return CreateLinkFileBaseA(pathToObj, pathToLinkA, description, hotkey, cmdLine, forceCreate);
}

int CreateShortCut::CreateLinkFileBase(LPCWSTR pathToObj, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, LPCWSTR cmdLine, BOOL forceCreate)
{
	return CreateLinkFileBaseW(pathToObj, pathToLink, description, hotkey, cmdLine, forceCreate);
}

int CreateShortCut::CreateLinkToPrinter(LPCSTR printerName, LPCSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate)
{
	return CreateLinkToPrinterA(printerName, pathToLink, description, hotkey, forceCreate);
}

int CreateShortCut::CreateLinkToPrinter(LPCSTR printerName, LPCWSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate)
{
	char pathToLinkA[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, pathToLink, -1, pathToLinkA, sizeof(pathToLinkA), NULL, NULL);
	return CreateLinkToPrinterA(printerName, pathToLinkA, description, hotkey, forceCreate);
}

int CreateShortCut::CreateLinkToPrinter(LPCWSTR printerName, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, BOOL forceCreate)
{
	return CreateLinkToPrinterW(printerName, pathToLink, description, hotkey, forceCreate);
}


/*************************************************************************************************************************************
	private members
***************************************************************************************************************************************/

//Multibyte Version of filebase shortcut
int CreateShortCut::CreateLinkFileBaseA(LPCSTR pathToObj, LPCSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate)
{
	//convert input to wide characters and call wide charater version
	wchar_t pathToObjW[MAX_PATH];
	wchar_t pathToLinkW[MAX_PATH];
	wchar_t descriptionW[MAX_PATH];
	wchar_t cmdLineW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, pathToObj, strlen(pathToObj)+1, pathToObjW, sizeof(pathToObjW)/sizeof(pathToObjW[0]));
    MultiByteToWideChar(CP_ACP, 0, pathToLink, strlen(pathToLink)+1, pathToLinkW, sizeof(pathToLinkW)/sizeof(pathToLinkW[0]));
	MultiByteToWideChar(CP_ACP, 0, description, strlen(description)+1, descriptionW, sizeof(descriptionW)/sizeof(descriptionW[0]));
	MultiByteToWideChar(CP_ACP, 0, cmdLine, strlen(cmdLine)+1, cmdLineW, sizeof(cmdLineW)/sizeof(cmdLineW[0]));

	return CreateLinkFileBaseW(pathToObjW, pathToLinkW,  descriptionW, hotkey, cmdLineW, forceCreate);
}

//Wide character version of filebase shortcut
int CreateShortCut::CreateLinkFileBaseW(LPCWSTR pathToObj, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, LPCWSTR cmdLine, BOOL forceCreate)
{
	BOOL isFile = FALSE;
	
	//Find out if it is a file or a folder
	struct _stat s;
	if(_wstat(pathToObj, &s) == 0)
	{
		if( s.st_mode & S_IFDIR )
			isFile = FALSE;
		else if( s.st_mode & S_IFREG )
			isFile = TRUE;
		else
		{
			ReportError("Not file, device, or folder");
			return 0;
		}
	}
	else
	{
		ReportError("Path not found");
		return 0;
	}
	
	//Appending '/' to pathToLink if not present
	memset(path, '\0', MAX_PATH);
	_wmakepath_s(path, MAX_PATH, NULL, pathToLink, NULL, NULL);
	
	
	//***create the directory
	if(forceCreate)
	{
		createDir(pathToLink);
	}
	


	HRESULT hRes;
	IShellLink* psl;

	

	if(isFile)
		hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	else
	{
		//hRes = CoCreateInstance(CLSID_FolderShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
		hRes = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	}

	if(SUCCEEDED(hRes))
	{
		IPersistFile* ppf;

		psl->SetPath(pathToObj);
		psl->SetDescription(description);
		psl->SetHotkey(hotkey);
		psl->SetArguments(cmdLine);
		//Query IShellLink for IPersistFile interface
		hRes = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
		if(SUCCEEDED(hRes))
		{
			//Build the output path
			wchar_t* fname = new wchar_t[MAX_PATH];
			wchar_t* drive = new wchar_t[MAX_PATH];
			_wsplitpath(pathToObj, drive, NULL, fname,NULL); 

			if(wcscmp(fname,L"") != 0)//For files
			{
				wcscat_s(path, MAX_PATH, fname);
			}
			else if(wcscmp(drive,L"") != 0)//For drives
			{
				wchar_t d[2];
				d[0] = drive[0];
				d[1] = '\0';
				wcscat_s(path, MAX_PATH, d);
			}
			delete [] drive;
			delete [] fname;
			wcscat_s(path, MAX_PATH, L".lnk");
			
			ppf->Save(path, TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	else
	{
		ReportError("CoCreateInstance Error");
		return 0;
	}
	return 1;
}

//Wide character version of printer shortcut
int CreateShortCut::CreateLinkToPrinterA(LPCSTR printerName, LPCSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate)
{
	//convert input to wide characters and call wide charater version
	wchar_t printerNameW[MAX_PATH];
	wchar_t pathToLinkW[MAX_PATH];
	wchar_t descriptionW[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, printerName, strlen(printerName)+1, printerNameW, sizeof(printerNameW)/sizeof(printerNameW[0]));
    MultiByteToWideChar(CP_ACP, 0, pathToLink, strlen(pathToLink)+1, pathToLinkW, sizeof(pathToLinkW)/sizeof(pathToLinkW[0]));
	MultiByteToWideChar(CP_ACP, 0, description, strlen(description)+1, descriptionW, sizeof(descriptionW)/sizeof(descriptionW[0]));
	return CreateLinkToPrinterW(printerNameW, pathToLinkW, descriptionW, hotkey, forceCreate);
}

//Wide character version of printer shortcut
int CreateShortCut::CreateLinkToPrinterW(LPCWSTR printerName, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, BOOL forceCreate)
{
	HRESULT hr = S_OK;
	ULONG celtFetched;
	LPITEMIDLIST pidlItems = NULL;
	LPITEMIDLIST netItemIdLst = NULL;
	IShellFolder* pPrinterFolder = NULL;
	IEnumIDList* pEnumIds = NULL;
	
	IShellFolder* pDesktopFolder = NULL;
	
	LPITEMIDLIST full_pid;
	
	
	
	hr = SHGetMalloc(&pMalloc);
	hr = SHGetDesktopFolder( &pDesktopFolder );
	hr = SHGetSpecialFolderLocation( NULL, CSIDL_PRINTERS, &netItemIdLst );
	hr = pDesktopFolder->BindToObject( netItemIdLst, NULL, IID_IShellFolder, (void **)&pPrinterFolder );
	
	//Appending '/' to pathToLink if not present
	memset(path, '\0', MAX_PATH);
	_wmakepath_s(path, MAX_PATH, NULL, pathToLink, NULL, NULL);
	
	//***create the directory
	if(forceCreate)
	{
		createDir(pathToLink);
	}

	if(SUCCEEDED(hr))
	{
		hr = pPrinterFolder->EnumObjects( NULL, SHCONTF_NONFOLDERS, &pEnumIds );
		
		if(SUCCEEDED(hr))
		{
			STRRET strDisplayName;
 			
			while((hr = pEnumIds->Next( 1, &pidlItems, &celtFetched)) == S_OK && celtFetched > 0)
			{
				hr = pPrinterFolder->GetDisplayNameOf(pidlItems, SHGDN_NORMAL, &strDisplayName);
				if(SUCCEEDED(hr))
				{
					if(_wcsicmp(strDisplayName.pOleStr, printerName) == 0)
					{
						wcscat_s(path, MAX_PATH, strDisplayName.pOleStr);
						wcscat_s(path, MAX_PATH, L".lnk");
						full_pid=Append(netItemIdLst, pidlItems);
						
						//Create the shortcut
						CreateLink(full_pid, path, description, hotkey);
					}
				}
			}
		}
	}
	return 0;
}



LPITEMIDLIST CreateShortCut::GetNextItemID(LPCITEMIDLIST pidl) 
{ 
   // Check for valid pidl.
   if(pidl == NULL)
	   return NULL;

   // Get the size of the specified item identifier. 
   int cb = pidl->mkid.cb; 

   // If the size is zero, it is the end of the list. 

   if (cb == 0)
	   return NULL; 

   // Add cb to pidl (casting to increment by bytes). 
   pidl = (LPITEMIDLIST)(((LPBYTE) pidl) + cb); 

   // Return NULL if it is null-terminating, or a pidl otherwise. 
   return (pidl->mkid.cb == 0) ? NULL : (LPITEMIDLIST) pidl; 
} 


/* Get size of PIDL */
UINT CreateShortCut::GetSize(LPCITEMIDLIST pidl)
{
    UINT cbTotal = 0;
    if (pidl)
    {
        cbTotal += sizeof(pidl->mkid.cb);    // Terminating null character
        while (pidl)
        {
            cbTotal += pidl->mkid.cb;
            pidl = GetNextItemID(pidl);
        }
    }
    return cbTotal;
}

/* Appending PIDLs */
LPITEMIDLIST CreateShortCut::Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd)
{
    if(pidlBase == NULL)
        return NULL;
    if(pidlAdd == NULL)
        return (LPITEMIDLIST)pidlBase;
    
    LPITEMIDLIST pidlNew;

    UINT cb1 = GetSize(pidlBase) - sizeof(pidlBase->mkid.cb);
    UINT cb2 = GetSize(pidlAdd);

    pidlNew = (LPITEMIDLIST)pMalloc->Alloc(cb1 + cb2);
    if (pidlNew)
    {
        CopyMemory(pidlNew, pidlBase, cb1);
        CopyMemory(((LPSTR)pidlNew) + cb1, pidlAdd, cb2);
    }
    return pidlNew;
}


long CreateShortCut::CreateLink(LPCITEMIDLIST lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc, WORD hotkey) 
{ 
    HRESULT hres; 
    IShellLink* psl; 
	
    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hres)) 
    { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target and add the description. 
        psl->SetIDList(lpszPathObj);
        psl->SetDescription(lpszDesc); 
		psl->SetHotkey(hotkey);
 
        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
 
        if (SUCCEEDED(hres)) 
        { 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(lpszPathLink, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return hres; 
}

void CreateShortCut::ReportError(char* err)
{
	printf("%s\n", err);
}

int CreateShortCut::createDir(LPCWSTR pathToLink)
{
	struct _stat s;
		
	//remove any trailing slash because of _wstat
	wchar_t* paths = new wchar_t[MAX_PATH];
	remove_last_slash(pathToLink, paths);
		
	if(_wstat(paths, &s) == 0)
	{
		if(s.st_mode & S_IFREG)
		{
			ReportError("pathToLink points to a path that is existing and not a directory");
			delete [] paths;
			return 0;
		}
	}
	else
	{
		//change all '/' to '\' because of SHCreateDirectoryEx
		wchar_t* yu = new wchar_t[MAX_PATH];
		str_replace(pathToLink, '/', '\\', yu);
		
		int res = SHCreateDirectoryEx(NULL,  yu, NULL);
		if(res != ERROR_SUCCESS)
		{
			ReportError("error occurred while trying to create the pathToLink");
			delete [] paths;
			delete [] yu;
			return 0;
		}
		delete [] yu;
	}
	delete [] paths;
	return 1;
}


/**********************************************************************************************
	Some useful functions
***********************************************************************************************/
wchar_t xStr[MAX_PATH];

wchar_t* GetDeskTopPath()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetAllDeskTopPath()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetProgramFiles()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetProgramFilesX86()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetStartup()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetSendto()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_SENDTO, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetWindows()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetSystem32()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, 0, xStr);
	return xStr;
}

wchar_t* GetStartmenu()
{
	memset(xStr, '\0', MAX_PATH);
	SHGetFolderPath(NULL, CSIDL_STARTMENU, NULL, 0, xStr);
	return xStr;
}


//Other functions
int str_replace(LPCWSTR originalstring, wchar_t oldcharacter, wchar_t newcharacter, wchar_t* &newstring)
{
    int changes=0;
    int n=0;
    while(*originalstring != '\0')
    {
		if(*originalstring == oldcharacter)
		{
			newstring[n] = newcharacter;
			changes++;
		}
		else
		{
			newstring[n]=*originalstring;
		}
		originalstring++;
		n++;
	}
	newstring[n]='\0';
	return changes;
}

void remove_last_slash(LPCWSTR originalstring, wchar_t* &newstring)
{
    int n=0;
    while(*originalstring != '\0')
    {
		originalstring[n];
		newstring[n]=*originalstring;
		originalstring++;
		n++;
	}
	if(newstring[n-1] == '\\' || newstring[n-1] == '/')
		newstring[n-1]='\0';
	else
		newstring[n]='\0';
}

