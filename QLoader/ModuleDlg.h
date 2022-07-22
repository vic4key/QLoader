#pragma once

#include "Resource.h"

// ModuleDlg dialog

class ModuleDlg : public CDialogEx
{
  DECLARE_DYNAMIC(ModuleDlg)
public:
  ModuleDlg(const std::vector<std::string>& existing_module_names, CWnd* pParent = nullptr);
  virtual ~ModuleDlg();

  enum { IDD = IDD_MODULE_DIALOG };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()

protected:
  const std::vector<std::string> m_existing_module_names;
  CString m_module_name;
};
