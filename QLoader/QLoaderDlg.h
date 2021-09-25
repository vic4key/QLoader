
// QLoaderDlg.h : header file
//

#pragma once

#include "QLoader.h"
#include "EasyTreeCtrl.h"

struct jnode_t : public Node
{
  jnode_t(const std::string& name, void* ptr = nullptr, json* ptr_parent = nullptr)
    : Node(name, ptr), m_ptr_parent(ptr_parent) {}
  json* m_ptr_parent;
};

// CQLoaderDlg dialog
class CQLoaderDlg : public CDialogEx, public QLoader
{
  // Construction
public:
  enum { IDD = IDD_QLOADER_DIALOG };
  CQLoaderDlg(CWnd* pParent = nullptr);	// standard constructor

protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
  void reset_ui();
  void update_ui();
  void initialize_ui();
  void populate_tree();

  virtual void add_log(const std::wstring& line, const status_t status = status_t::none);

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
  afx_msg void OnBnClicked_PEOpen();
  afx_msg void OnBnClicked_MPOpen();
  afx_msg void OnBnClicked_MPSave();
  afx_msg void OnBnClicked_Clear();
  afx_msg void OnBnClicked_Export();
  afx_msg void OnBnClicked_Launch();
  DECLARE_MESSAGE_MAP()

private:
  HICON m_hIcon;
  CString m_pe_path;
  CString m_pe_dir;
  CString m_pe_arg;
  CString m_mp_path;
  int m_patch_when;
  CListCtrl m_log;
  EasyTreeCtrl m_mp_tree;
  CButton m_button_mp_save;
  CButton m_button_export;
  CButton m_button_launch;

private:
  std::vector<std::wstring> m_file_paths;
};
