#pragma once

#include "Resource.h"
#include "3rd-mfc-numeric-edit-ctrl/CNumericEditControl.h"

class CQLoaderDlg;

// PatchDlg dialog

class PatchDlg : public CDialogEx
{
  DECLARE_DYNAMIC(PatchDlg)
  friend CQLoaderDlg;

public:
  PatchDlg(
    const std::vector<std::string>& existing_patch_names,
    const CString& selected_module_name,
    CWnd* pParent = nullptr);
  virtual ~PatchDlg();

  enum { IDD = IDD_PATCH_DIALOG };

protected:
  virtual BOOL OnInitDialog();
  virtual void OnOK();

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  DECLARE_MESSAGE_MAP()

protected:
  const std::vector<std::string>& m_existing_patch_names;
  CNumericEditControl m_patch_offset_ctrl;
  CString m_selected_module_name;
  CString m_patch_name;
  CString m_patch_pattern;
  CString m_patch_replacement;
  CString m_patch_offset;
  BOOL m_patch_enabled;
};
