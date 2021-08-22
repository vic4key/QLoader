
// QLoaderDlg.h : header file
//

#pragma once

#include <vector>
#include <string>

#include "EasyTreeCtrl.h"

// CQLoaderDlg dialog
class CQLoaderDlg : public CDialogEx
{
  // Construction
public:
  CQLoaderDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_QICKLOADER_DIALOG };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
  void reset_ui();
  void update_ui();
  void initialize_ui();
  void populate_tree();

  bool is_usable_file(const CString& file_path);

  enum status_t
  {
    none,
    success,
    warn,
    error,
  };

  void add_log(const std::wstring& line, const status_t status = none);

  // Implementation
protected:
  // Generated message map functions
  virtual BOOL OnInitDialog();
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnDropFiles(HDROP hDropInfo);
  // Generated user message map functions
  afx_msg void OnBnClickedPEOpen();
  afx_msg void OnBnClickedMPOpen();
  afx_msg void OnBnClickedClear();
  afx_msg void OnBnClickedLaunch();
  DECLARE_MESSAGE_MAP()

private:
  HICON m_hIcon;
  CString m_pe_path;
  CString m_pe_dir;
  CString m_mp_path;
  EasyTreeCtrl m_mp_tree;
  CListCtrl m_log;
  CButton m_launch;

private:
  std::vector<std::wstring> m_file_paths;
};
