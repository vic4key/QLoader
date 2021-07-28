
// QickLoaderDlg.h : header file
//

#pragma once

#include <vector>
#include <string>

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
  bool IsUsableFile(const CString& filePath);

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
  DECLARE_MESSAGE_MAP()

private:
  HICON m_hIcon;
  CString m_pe_path;
  CString m_pe_dir;
  BOOL m_pe_auto;
  CString m_mp_path;
  CTreeCtrl m_mp_tree;
  CButton m_launch;

private:
  std::vector<std::wstring> m_file_paths;
};
