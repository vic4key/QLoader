
// QickLoaderDlg.h : header file
//

#pragma once

#include <vector>
#include <string>

#include "EasyTreeCtrl.h"

// CQickLoaderDlg dialog
class CQickLoaderDlg : public CDialogEx
{
  // Construction
public:
  CQickLoaderDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_QICKLOADER_DIALOG };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
  void ResetUI();
  void UpdateUI();
  void InitializeTree();
  void PopulateTree(const std::wstring& file_path);

  bool IsUsableFile(const CString& file_path);

  enum status_t
  {
    none,
    success,
    warn,
    error,
  };

  void AddLog(const std::wstring& line, const status_t status = none);

  // Implementation
protected:
  // Generated message map functions
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnDropFiles(HDROP hDropInfo);
  // Generated user message map functions
  afx_msg void OnBnClickedAuto();
  afx_msg void OnBnClickedLaunch();
  DECLARE_MESSAGE_MAP()

private:
  HICON m_hIcon;
  CString m_pe_path;
  CString m_pe_dir;
  BOOL m_pe_auto;
  CString m_mp_path;
  EasyTreeCtrl m_mp_tree;
  CListCtrl m_log;
  CButton m_launch;

private:
  std::vector<std::wstring> m_file_paths;
};
