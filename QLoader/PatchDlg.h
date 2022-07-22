#pragma once

#include "Resource.h"
#include "3rd-mfc-numeric-edit-ctrl/CNumericEditControl.h"

// PatchDlg dialog

class PatchDlg : public CDialogEx
{
  DECLARE_DYNAMIC(PatchDlg)
public:
  PatchDlg(const CString& module_name, CWnd* pParent = nullptr);
  virtual ~PatchDlg();

  enum { IDD = IDD_PATCH_DIALOG };

protected:
  virtual BOOL OnInitDialog();

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  DECLARE_MESSAGE_MAP()

protected:
  CString m_module_name;
  CString m_patch_name;
  CString m_patch_pattern;
  CString m_patch_replacement;
  CNumericEditControl m_patch_offset;
  BOOL m_patch_enabled;
};
