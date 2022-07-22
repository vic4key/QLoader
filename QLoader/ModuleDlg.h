#pragma once

#include "Resource.h"

// ModuleDlg dialog

class ModuleDlg : public CDialogEx
{
  DECLARE_DYNAMIC(ModuleDlg)
public:
  ModuleDlg(CWnd* pParent = nullptr);
  virtual ~ModuleDlg();

  enum { IDD = IDD_MODULE_DIALOG };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()

protected:
  CString m_module_name;
};
