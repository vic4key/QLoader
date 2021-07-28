
// QickLoaderDlg.h : header file
//

#pragma once


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
	void UpdateUI();

// Implementation
protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	// Generated user message map functions
	afx_msg void OnBnClickedAuto();
	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIcon;
	CString m_path;
	CString m_dir;
	BOOL m_auto;
	CTreeCtrl m_tree;
	CButton m_launch;
};
