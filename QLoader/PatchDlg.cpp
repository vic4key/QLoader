// PatchDlg.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"
#include "PatchDlg.h"

// PatchDlg dialog

IMPLEMENT_DYNAMIC(PatchDlg, CDialogEx)

PatchDlg::PatchDlg(CWnd* pParent) : CDialogEx(IDD_PATCH_DIALOG, pParent)
{
}

PatchDlg::~PatchDlg()
{
}

void PatchDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(PatchDlg, CDialogEx)
END_MESSAGE_MAP()

// PatchDlg message handlers
