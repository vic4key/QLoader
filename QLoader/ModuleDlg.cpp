// ModuleDlg.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"
#include "ModuleDlg.h"

// ModuleDlg dialog

IMPLEMENT_DYNAMIC(ModuleDlg, CDialogEx)

ModuleDlg::ModuleDlg(CWnd* pParent) : CDialogEx(IDD_MODULE_DIALOG, pParent)
{
}

ModuleDlg::~ModuleDlg()
{
}

void ModuleDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ModuleDlg, CDialogEx)
END_MESSAGE_MAP()

// ModuleDlg message handlers
