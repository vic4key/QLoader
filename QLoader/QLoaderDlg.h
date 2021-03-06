
// QLoaderDlg.h : header file
//

#pragma once

#include "QLoader.h"
#include <3rd-mfc-easy-tree-ctrl/EasyTreeCtrl.h>

enum jnode_e
{
  root,
  module_name,
  patch_name,
  patch_key,
  patch_value,
};

struct jnode_t : public Node
{
  jnode_t(const jnode_e type, const std::string& name, void* ptr = nullptr, json* ptr_parent = nullptr)
    : Node(name, ptr), m_ptr_parent(ptr_parent), m_type(type) {}
  json* m_ptr_parent;
  jnode_e m_type;
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
  virtual BOOL PreTranslateMessage(MSG* pMsg);

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
  afx_msg void OnDropFiles(HDROP hDropInfo);
  // Generated user message map functions
  afx_msg void OnBnClicked_PEOpen();
  afx_msg void OnBnClicked_MPOpen();
  afx_msg void OnBnClicked_MPSave();
  afx_msg void OnBnClicked_Clear();
  afx_msg void OnBnClicked_Export();
  afx_msg void OnBnClicked_ExportLNK();
  afx_msg void OnBnClicked_ExportURL();
  afx_msg void OnBnClicked_ExportHTML();
  afx_msg void OnBnClicked_Launch();
  afx_msg void OnUpdate_UIData();
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
  CSplitButton m_button_export;
  CButton m_button_launch;
  CToolTipCtrl m_tool_tip;
  std::vector<std::wstring> m_file_paths;
};
