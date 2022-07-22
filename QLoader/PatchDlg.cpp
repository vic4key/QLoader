// PatchDlg.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"
#include "PatchDlg.h"

// PatchDlg dialog

IMPLEMENT_DYNAMIC(PatchDlg, CDialogEx)

PatchDlg::PatchDlg(const CString& module_name, CWnd* pParent)
  : CDialogEx(IDD_PATCH_DIALOG, pParent), m_module_name(module_name)
{
}

PatchDlg::~PatchDlg()
{
}

BOOL PatchDlg::OnInitDialog()
{
  auto result = __super::OnInitDialog();

  CString title;
  title.Format(L"Add New Patch to '%s'", m_module_name);
  this->SetWindowText(title);

  return result;
}

void PatchDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(PatchDlg, CDialogEx)
END_MESSAGE_MAP()

// PatchDlg message handlers
