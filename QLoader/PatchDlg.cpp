// PatchDlg.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"
#include "PatchDlg.h"

// PatchDlg dialog

IMPLEMENT_DYNAMIC(PatchDlg, CDialogEx)

BEGIN_MESSAGE_MAP(PatchDlg, CDialogEx)
END_MESSAGE_MAP()

void PatchDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PATCH_NAME, m_patch_name);
  DDX_Text(pDX, IDC_PATCH_PATTERN, m_patch_pattern);
  DDX_Text(pDX, IDC_PATCH_REPLACEMENT, m_patch_replacement);
  DDX_Control(pDX, IDC_PATCH_OFFSET, m_patch_offset);
  DDX_Check(pDX, IDC_PATCH_ENABLED, m_patch_enabled);
}

PatchDlg::PatchDlg(
  const std::vector<std::string>& existing_patch_names, const CString& selected_module_name, CWnd* pParent)
  : CDialogEx(IDD_PATCH_DIALOG, pParent)
  , m_existing_patch_names(existing_patch_names)
  , m_selected_module_name(selected_module_name)
  , m_patch_name(_T(""))
  , m_patch_pattern(_T(""))
  , m_patch_replacement(_T(""))
  , m_patch_enabled(BST_CHECKED)
{
}

PatchDlg::~PatchDlg()
{
}

BOOL PatchDlg::OnInitDialog()
{
  auto result = __super::OnInitDialog();

  CString title;
  title.Format(L"Add New Patch to '%s'", m_selected_module_name);
  this->SetWindowText(title);

  return result;
}

// PatchDlg message handlers
