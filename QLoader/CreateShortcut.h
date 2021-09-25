#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>

#include <Shobjidl.h>
#include "shlobj.h"
#include "stdio.h"
#include "winnls.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"

class CreateShortCut
{
private:
	wchar_t* path;
	IMalloc* pMalloc;
	long CreateLink(LPCITEMIDLIST lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc, WORD hotkey);
	LPITEMIDLIST Append(LPCITEMIDLIST pidlBase, LPCITEMIDLIST pidlAdd);
	UINT GetSize(LPCITEMIDLIST pidl);
	LPITEMIDLIST GetNextItemID(LPCITEMIDLIST pidl) ;
	void ReportError(char* err);
	int createDir(LPCWSTR pathToLink);
	
	int CreateLinkFileBaseA(LPCSTR pathToObj, LPCSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate);
	int CreateLinkFileBaseW(LPCWSTR pathToObj, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, LPCWSTR cmdLine, BOOL forceCreate);

	int CreateLinkToPrinterA(LPCSTR printerName, LPCSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate);
	int CreateLinkToPrinterW(LPCWSTR printerName, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, BOOL forceCreate);

protected:
	

public:
	//Constructor
	CreateShortCut();

	//Destructor
	~CreateShortCut();

	//Create shortcut to a file base object (e.g local files and folders, network files and folders, drives, etc)
	int CreateLinkFileBase(LPCSTR pathToObj, LPCSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate = FALSE);
	//Create shortcut to a file base object (e.g local files and folders, network files and folders, drives, etc)
	int CreateLinkFileBase(LPCSTR pathToObj, LPCWSTR pathToLink, LPCSTR description, WORD hotkey, LPCSTR cmdLine, BOOL forceCreate = FALSE);//For the CSC constants
	//Create shortcut to a file base object (e.g local files and folders, network files and folders, drives, etc)
	int CreateLinkFileBase(LPCWSTR pathToObj, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, LPCWSTR cmdLine, BOOL forceCreate = FALSE);
	
	//create shortcut to a printer
	int CreateLinkToPrinter(LPCSTR printerName, LPCSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate = FALSE);
	//create shortcut to a printer
	int CreateLinkToPrinter(LPCSTR printerName, LPCWSTR pathToLink, LPCSTR description, WORD hotkey, BOOL forceCreate = FALSE);//For the CSC constants
	//create shortcut to a printer
	int CreateLinkToPrinter(LPCWSTR printerName, LPCWSTR pathToLink, LPCWSTR description, WORD hotkey, BOOL forceCreate = FALSE);
};


wchar_t* GetDeskTopPath();
//Get path to the file system directory used to physically store file objects on the desktop
#define CSC_DESKTOP GetDeskTopPath()

wchar_t* GetAllDeskTopPath();
//Get path to the file system directory that contains files and folders that appear on the desktop for all users
#define CSC_ALL_DESKTOP GetAllDeskTopPath()

wchar_t* GetProgramFiles();
//Get path to the Program Files folder
#define CSC_PROGRAMFILES GetProgramFiles()

wchar_t* GetProgramFilesX86();
//Get path to the Program Files(x86) folder
#define CSC_PROGRAMFILESX86 GetProgramFilesX86()

wchar_t* GetStartup();
//Get path to the file system directory that corresponds to the user's Startup program group
#define CSC_STARTUP GetStartup()

wchar_t* GetSendto();
//Get path to the file system directory that contains Send To menu items
#define CSC_SENDTO GetSendto()

wchar_t* GetWindows();
//Get path to the Windows directory
#define CSC_WINDOWS GetWindows()

wchar_t* GetSystem32();
//Get path to the System32 directory
#define CSC_SYSTEM32 GetSystem32()

wchar_t* GetStartmenu();
//Get path to the file system directory that contains Start menu items
#define CSC_STARTMENU GetStartmenu()