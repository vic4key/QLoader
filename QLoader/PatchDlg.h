#pragma once

#include "Resource.h"

// PatchDlg dialog

class PatchDlg : public CDialogEx
{
  DECLARE_DYNAMIC(PatchDlg)
public:
  PatchDlg(CWnd* pParent = nullptr);
  virtual ~PatchDlg();

  enum { IDD = IDD_PATCH_DIALOG };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);

  DECLARE_MESSAGE_MAP()
};
