// ModuleDlg.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"
#include "ModuleDlg.h"

// ModuleDlg dialog

IMPLEMENT_DYNAMIC(ModuleDlg, CDialogEx)

BEGIN_MESSAGE_MAP(ModuleDlg, CDialogEx)
END_MESSAGE_MAP()

void ModuleDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_MODULE_NAME, m_module_name);
}

ModuleDlg::ModuleDlg(const std::vector<std::string>& existing_module_names, CWnd* pParent)
  : CDialogEx(IDD_MODULE_DIALOG, pParent)
  , m_existing_module_names(existing_module_names)
  , m_module_name(_T(""))
{
}

ModuleDlg::~ModuleDlg()
{
}

// ModuleDlg message handlers
