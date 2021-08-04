
// QickLoaderDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "QickLoader.h"
#include "QickLoaderDlg.h"
#include "afxdialogex.h"

#include <vu>

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

static std::vector<std::wstring> USABLE_FILE_EXTENSIONS = { L".EXE", L".JSON" };

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
  CAboutDlg();

  // Dialog Data
#ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_ABOUTBOX };
#endif

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  __super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CQickLoaderDlg dialog



CQickLoaderDlg::CQickLoaderDlg(CWnd* pParent /*=nullptr*/)
  : CDialogEx(IDD_QICKLOADER_DIALOG, pParent)
  , m_pe_path(_T(""))
  , m_pe_dir(_T(""))
  , m_pe_auto(BST_UNCHECKED)
  , m_mp_path(_T(""))
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQickLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
  __super::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PE_PATH, m_pe_path);
  DDX_Text(pDX, IDC_PE_DIR, m_pe_dir);
  DDX_Check(pDX, IDC_PE_AUTO, m_pe_auto);
  DDX_Text(pDX, IDC_MP_PATH, m_mp_path);
  DDX_Control(pDX, IDC_MP_TREE, m_mp_tree);
  DDX_Control(pDX, ID_LAUNCH, m_launch);
}

BEGIN_MESSAGE_MAP(CQickLoaderDlg, CDialogEx)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_DROPFILES()
  ON_BN_CLICKED(IDC_PE_AUTO, &OnBnClickedAuto)
END_MESSAGE_MAP()


// CQickLoaderDlg message handlers

BOOL CQickLoaderDlg::OnInitDialog()
{
  __super::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != nullptr)
  {
    BOOL bNameValid;
    CString strAboutMenu;
    bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
    ASSERT(bNameValid);
    if (!strAboutMenu.IsEmpty())
    {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);			// Set big icon
  SetIcon(m_hIcon, FALSE);		// Set small icon

  // TODO: Add extra initialization here

  // ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  // ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  // __super::DragAcceptFiles();

  this->InitializeTree();

  this->ResetUI();

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CQickLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else
  {
    __super::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CQickLoaderDlg::OnPaint()
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    __super::OnPaint();
  }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CQickLoaderDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CQickLoaderDlg::OnDropFiles(HDROP hDropInfo)
{
  // TODO: Add your message handler code here and/or call default

  m_file_paths.clear();

  UINT nFilesDropped = DragQueryFile(hDropInfo, 0xFFFFFFFF, nullptr, 0);
  for (UINT nIndex = 0; nIndex < nFilesDropped; ++nIndex)
  {
    CString filePath;
    DWORD filePathLength = MAX_PATH; // DragQueryFile(hDropInfo, 0, nullptr, 0) + 1;
    DragQueryFile(hDropInfo, nIndex, filePath.GetBuffer(filePathLength), filePathLength);
    if (IsUsableFile(filePath))
    {
      m_file_paths.push_back(filePath.GetString());
    }
    filePath.ReleaseBuffer();
  }

  DragFinish(hDropInfo);

  __super::OnDropFiles(hDropInfo);

  this->UpdateUI();
}

void CQickLoaderDlg::OnBnClickedAuto()
{
  UpdateData();

  m_launch.EnableWindow(m_pe_auto == BST_UNCHECKED);

  this->ResetUI();
}

bool CQickLoaderDlg::IsUsableFile(const CString& filePath)
{
  // if (filePath.IsEmpty())
  // {
  //   return false;
  // }

  std::wstring file_extension = PathFindExtension(filePath);
  file_extension = vu::UpperString(file_extension);

  auto it = std::find(USABLE_FILE_EXTENSIONS.cbegin(), USABLE_FILE_EXTENSIONS.cend(), file_extension);
  if (it == USABLE_FILE_EXTENSIONS.cend())
  {
    return false;
  }

  return true;
}

void CQickLoaderDlg::ResetUI()
{
  m_file_paths.clear();

  m_pe_path = _T("");
  m_pe_dir  = _T("");
  m_mp_path = _T("");

  m_mp_tree.DeleteAllItems();

  m_launch.EnableWindow(FALSE);

  UpdateData(FALSE);
}

void CQickLoaderDlg::UpdateUI()
{
  if (m_file_paths.empty())
  {
    return;
  }

  bool found_exe  = false;
  bool found_json = false;

  for (auto& file_path: m_file_paths)
  {
    auto file_path_tmp = file_path;
    file_path_tmp = vu::UpperString(file_path_tmp);

    if (!found_exe && vu::EndsWith(file_path_tmp, L".EXE"))
    {
      m_pe_path = file_path.c_str();
      m_pe_dir  = vu::ExtractFileDirectory(file_path.c_str()).c_str();
      found_exe = true;
    }

    if (!found_json && vu::EndsWith(file_path_tmp, L".JSON"))
    {
      m_mp_path  = file_path.c_str();
      found_json = true;
    }
  }

  if (found_json)
  {
    std::wstring mp_path = m_mp_path.GetBuffer(0);
    assert(vu::IsFileExists(mp_path));
    this->PopulateTree(mp_path);
  }

  UpdateData(FALSE);
  RedrawWindow();
}

void CQickLoaderDlg::InitializeTree()
{
  m_mp_tree.OnNotify([&](EasyTreeCtrl::eNotifyType action, Node* pNode) -> bool
  {
    wchar_t* actions[] =
    {
      L"SELECTING",

      L"BEFORE_INSERTING",
      L"AFTER_INSERTING",

      L"BEFORE_MODIFYING",
      L"AFTER_MODIFYING",

      L"BEFORE_DELETING",
      L"AFTER_DELETING",
    };

    CString s;
    s.Format(L"`%s` -> %s", actions[int(action)], pNode != nullptr ? pNode->m_Name : L"<empty>");
    OutputDebugStringW(s.GetBuffer());

    return true;
  });
}

void CQickLoaderDlg::PopulateTree(const std::wstring& file_path)
{
  std::string s = vu::ToStringA(file_path);
  std::ifstream fs(s);

  static json jmodules;
  jmodules = json::parse(fs);

  m_mp_tree.Populate([&](HTREEITEM& root) -> void
  {
    auto fn_tree_add_node_str = [&](HTREEITEM& hitem, json& jobject, std::string name) -> HTREEITEM
    {
      auto h_item = m_mp_tree.InsertNode(hitem, new Node(name));
      auto& jitem = jobject[name];
      return m_mp_tree.InsertNode(h_item, new Node(jitem.get<std::string>(), &jitem));
    };

    auto fn_tree_add_node_int = [&](HTREEITEM& hitem, json& jobject, std::string name) -> HTREEITEM
    {
      auto h_item = m_mp_tree.InsertNode(hitem, new Node(name));
      auto& jitem = jobject[name];
      return m_mp_tree.InsertNode(h_item, new Node(std::to_string(jitem.get<int>()), &jitem));
    };

    for (auto& module : jmodules.items())
    {
      auto h_module = m_mp_tree.InsertNode(root, new Node(module.key(), nullptr));
      assert(h_module != nullptr);

      for (auto& patch : module.value())
      {
        auto h_patch = m_mp_tree.InsertNode(h_module, new Node(patch["name"].get<std::string>()));
        assert(h_patch != nullptr);

        fn_tree_add_node_str(h_patch, patch, "pattern");
        fn_tree_add_node_str(h_patch, patch, "replacement");
        fn_tree_add_node_int(h_patch, patch, "offset");
      }
    }
  });
}
