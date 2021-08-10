
// QLoaderDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "QLoader.h"
#include "QLoaderDlg.h"
#include "afxdialogex.h"
#include "utils.h"

#include <winternl.h>
#pragma comment(lib,"ntdll")

#include <dbghelp.h>
#pragma comment(lib,"dbghelp")

#include <vu>

static json g_jdata;
static std::string EMPTY = "";
static std::string UNNAMED = "<unnamed>";
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

// CQLoaderDlg dialog

CQLoaderDlg::CQLoaderDlg(CWnd* pParent /*=nullptr*/)
  : CDialogEx(IDD_QICKLOADER_DIALOG, pParent)
  , m_pe_path(_T(""))
  , m_pe_dir(_T(""))
  , m_mp_path(_T(""))
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
  __super::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PE_PATH, m_pe_path);
  DDX_Text(pDX, IDC_PE_DIR, m_pe_dir);
  DDX_Text(pDX, IDC_MP_PATH, m_mp_path);
  DDX_Control(pDX, IDC_MP_TREE, m_mp_tree);
  DDX_Control(pDX, IDC_LAUNCH, m_launch);
  DDX_Control(pDX, IDC_LOG, m_log);
}

BEGIN_MESSAGE_MAP(CQLoaderDlg, CDialogEx)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_SIZE()
  ON_WM_DROPFILES()
  ON_BN_CLICKED(IDC_PE_OPEN, &OnBnClickedPEOpen)
  ON_BN_CLICKED(IDC_MP_OPEN, &OnBnClickedMPOpen)
  ON_BN_CLICKED(IDC_CLEAR, &OnBnClickedClear)
  ON_BN_CLICKED(IDC_LAUNCH, &OnBnClickedLaunch)
END_MESSAGE_MAP()


// CQLoaderDlg message handlers

BOOL CQLoaderDlg::OnInitDialog()
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

  this->InitializeUI();

  this->ResetUI();

  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CQLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CQLoaderDlg::OnPaint()
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
HCURSOR CQLoaderDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

void CQLoaderDlg::OnSize(UINT nType, int cx, int cy)
{
  __super::OnSize(nType, cx, cy);

  if (::IsWindow(m_log.GetSafeHwnd()))
  {
    CRect rect;
    m_log.GetClientRect(&rect);
    int w_c0 = m_log.GetColumnWidth(0);
    int w_c1 = rect.Width() - w_c0;
    m_log.SetColumnWidth(1, w_c1);
  }
}

void CQLoaderDlg::OnDropFiles(HDROP hDropInfo)
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

void CQLoaderDlg::OnBnClickedPEOpen()
{
  m_file_paths.clear();

  CString filter = L"PE Files (*.exe)|*.exe|All Files (*.*)|*.*||";
  CFileDialog dialog(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter, this);
  if (dialog.DoModal() == IDOK)
  {
    std::wstring file_path = dialog.GetPathName();
    m_file_paths.push_back(file_path);
  }

  this->UpdateUI();
}

void CQLoaderDlg::OnBnClickedMPOpen()
{
  m_file_paths.clear();

  CString filter = L"MP Files (*.json)|*.json|All Files (*.*)|*.*||";
  CFileDialog dialog(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter, this);
  if (dialog.DoModal() == IDOK)
  {
    std::wstring file_path = dialog.GetPathName();
    m_file_paths.push_back(file_path);
  }

  this->UpdateUI();
}

void CQLoaderDlg::OnBnClickedClear()
{
  this->ResetUI();
}

bool CQLoaderDlg::IsUsableFile(const CString& file_path)
{
  // if (filePath.IsEmpty())
  // {
  //   return false;
  // }

  std::wstring file_extension = PathFindExtension(file_path);
  file_extension = vu::UpperString(file_extension);

  auto it = std::find(USABLE_FILE_EXTENSIONS.cbegin(), USABLE_FILE_EXTENSIONS.cend(), file_extension);
  if (it == USABLE_FILE_EXTENSIONS.cend())
  {
    return false;
  }

  return true;
}

void CQLoaderDlg::AddLog(const std::wstring& line, const status_t status)
{
  LVITEM lvi = { 0 };
  lvi.mask = LVIF_IMAGE;
  lvi.iItem = m_log.GetItemCount();
  lvi.iSubItem = 0;
  lvi.iImage = status;
  lvi.pszText  = const_cast<wchar_t*>(line.c_str());
  int index = m_log.InsertItem(&lvi);
  m_log.SetItemText(index, 1, line.c_str());
}

void CQLoaderDlg::ResetUI()
{
  m_file_paths.clear();

  m_pe_path = _T("");
  m_pe_dir  = _T("");
  m_mp_path = _T("");

  g_jdata.clear();

  m_mp_tree.DeleteAllItems();
  m_log.DeleteAllItems();

  m_launch.EnableWindow(FALSE);

  UpdateData(FALSE);
}

void CQLoaderDlg::UpdateUI()
{
  // if (m_file_paths.empty())
  // {
  //   return;
  // }

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

  m_file_paths.clear();

  if (found_json)
  {
    std::wstring mp_path = m_mp_path.GetBuffer(0);
    assert(vu::IsFileExists(mp_path));
    std::string s = vu::ToStringA(mp_path);
    std::ifstream fs(s);
    g_jdata = json::parse(fs);
  }

  this->PopulateTree();

  m_launch.EnableWindow(!m_pe_dir.IsEmpty() && !m_pe_path.IsEmpty() && !m_mp_path.IsEmpty());

  UpdateData(FALSE);
  RedrawWindow();
}

void CQLoaderDlg::InitializeUI()
{
  m_mp_tree.OnNotify([&](EasyTreeCtrl::eNotifyType action, Node* pNode) -> bool
  {
    switch (action)
    {
    case EasyTreeCtrl::eNotifyType::BEFORE_MODIFYING:
    {
      return true; // processing
    }
    break;

    case EasyTreeCtrl::eNotifyType::AFTER_MODIFYING:
    {
      if (pNode != nullptr &&
          pNode->m_ptr_data != nullptr &&
          pNode->m_ptr_tv != nullptr &&
          pNode->m_ptr_tv->pszText)
      {
        auto value = vu::ToStringA(pNode->m_ptr_tv->pszText);

        auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
        if (ptr_jobject != nullptr)
        {
          auto& jobject = static_cast<json&>(*ptr_jobject);
          if (jobject.is_string())
          {
            jobject = value;
          }
          else if (jobject.is_number())
          {
            jobject = std::atoi(value.c_str());
          }
        }
      }
    }
    break;

    case EasyTreeCtrl::eNotifyType::BEFORE_DELETING:
    {
      auto pJNode = static_cast<jnode_t*>(pNode);
      if (pJNode == nullptr || pJNode->m_module.empty())
      {
        return false;
      }

      return true; // processing
    }
    break;

    case EasyTreeCtrl::eNotifyType::AFTER_DELETING:
    {
      auto pJNode = static_cast<jnode_t*>(pNode);
      if (pJNode == nullptr || pJNode->m_module.empty())
      {
        return false;
      }

      std::string  module_name = pJNode->m_module;
      std::wstring wmodule_name = vu::ToStringW(module_name);

      bool deleted = false;

      if (pNode->m_ptr_data != nullptr) // json object patch
      {
        auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
        if (ptr_jobject != nullptr)
        {
          auto& jpatch = static_cast<json&>(*ptr_jobject);

          auto it = g_jdata.find(module_name);
          if (it != g_jdata.end())
          {
            auto& jpatches = *it;
            for (auto it = jpatches.begin(); it != jpatches.end(); it++)
            {
              auto v1 = utils::json_get(*it, "pattern", EMPTY);
              auto v2 = utils::json_get(jpatch, "pattern", EMPTY);
              if (v1 == v2)
              {
                auto patch_name  = utils::json_get(jpatch, "name", EMPTY);
                auto wpatch_name = vu::ToStringW(patch_name);
                auto line = vu::FormatW(L"Delete the patch `%s` succeed", wpatch_name.c_str());
                this->AddLog(line, status_t::success);
                jpatches.erase(it);
                deleted = true;
                break;
              }
            }
          }
        }
      }
      else if (!pNode->m_name.IsEmpty()) // json object module
      {
        auto line = vu::FormatW(L"Delete the module `%s` succeed", wmodule_name.c_str());
        this->AddLog(line, status_t::success);
        g_jdata.erase(module_name);
        deleted = true;
      }

      if (deleted)
      {
        this->UpdateUI();
      }
    }
    break;

    case EasyTreeCtrl::eNotifyType::BOX_CHECKING:
    {
      if (pNode != nullptr && pNode->m_ptr_data != nullptr && pNode->m_ptr_tv != nullptr)
      {
        auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
        if (ptr_jobject != nullptr)
        {
          auto& jpatch = static_cast<json&>(*ptr_jobject);
          bool checked = m_mp_tree.GetCheck(pNode->m_ptr_tv->hItem) == BST_CHECKED;
          jpatch["enabled"] = !checked; // add to json object path if not exists
          m_mp_tree.SetCheck(pNode->m_ptr_tv->hItem, checked ? BST_UNCHECKED : BST_CHECKED);
        }
      }
    }
    break;

    default: // no handler
      return false;
    }

    #ifdef _DEBUG

    const wchar_t* actions[] =
    {
      L"SELECTING",
      L"BOX_CHECKING",

      L"BEFORE_INSERTING",
      L"AFTER_INSERTING",

      L"BEFORE_MODIFYING",
      L"AFTER_MODIFYING",

      L"BEFORE_DELETING",
      L"AFTER_DELETING",
    };

    CString s;
    s.Format(L"`%s` -> %s", actions[int(action)], pNode != nullptr ? pNode->m_name.GetBuffer(0) : L"<empty>");
    OutputDebugStringW(s.GetBuffer());
    
    std::ofstream file("test\\dump.json");
    file << g_jdata.dump(1, '\t');

    #endif // _DEBUG

    return true;
  });

  // for log ctrl

  m_log.SetExtendedStyle(m_log.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  auto ptr_header = m_log.GetHeaderCtrl();
  assert(ptr_header != nullptr);

  static CImageList image_list;
  image_list.Create(IDB_LOG_SMALL, 16, 1, 0xFFFFFF);
  m_log.SetImageList(&image_list, LVSIL_SMALL);

  const int DEFAULT_COLUMN_WIDTH = 20;

  LVCOLUMN lvc = { 0 };

  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = DEFAULT_COLUMN_WIDTH;
  lvc.pszText = L"?";
  m_log.InsertColumn(0, &lvc);

  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = DEFAULT_COLUMN_WIDTH;
  lvc.pszText = L"Description";
  m_log.InsertColumn(1, &lvc);

  this->SendMessage(WM_SIZE); // request to re-calculate size for window and its controls
}

void CQLoaderDlg::PopulateTree()
{
  if (g_jdata.is_null())
  {
    return;
  }

  m_mp_tree.DeleteAllItems();

  m_mp_tree.ModifyStyle(0, TVS_CHECKBOXES);

  m_mp_tree.Populate([&](HTREEITEM& root) -> void
  {
    m_mp_tree.SetItemState(root, 0, TVIS_STATEIMAGEMASK);

    auto fn_tree_add_node_str = [&](HTREEITEM& hparent, json& jobject, std::string key) -> HTREEITEM
    {
      auto hitem = m_mp_tree.InsertNode(hparent, new jnode_t(key));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      auto string = utils::json_get(jobject, key, EMPTY);
      hitem = m_mp_tree.InsertNode(hitem, new jnode_t(string, &jobject[key]));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      return hitem;
    };

    auto fn_tree_add_node_int = [&](HTREEITEM& hparent, json& jobject, std::string key) -> HTREEITEM
    {
      auto hitem = m_mp_tree.InsertNode(hparent, new jnode_t(key));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      auto number = utils::json_get(jobject, key, 0);
      hitem = m_mp_tree.InsertNode(hitem, new jnode_t(std::to_string(number), &jobject[key]));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      return hitem;
    };

    for (auto& module : g_jdata.items())
    {
      auto hmodule = m_mp_tree.InsertNode(root, new jnode_t(module.key(), nullptr, module.key()));
      m_mp_tree.SetItemState(hmodule, 0, TVIS_STATEIMAGEMASK);

      for (auto& jpatch : module.value())
      {
        auto name = utils::json_get(jpatch, "name", UNNAMED);
        if (auto hpatch = m_mp_tree.InsertNode(hmodule, new jnode_t(name, &jpatch, module.key())))
        {
          m_mp_tree.SetCheck(hpatch, utils::json_get(jpatch, "enabled", true));
          fn_tree_add_node_str(hpatch, jpatch, "pattern");
          fn_tree_add_node_str(hpatch, jpatch, "replacement");
          fn_tree_add_node_int(hpatch, jpatch, "offset");
        }
      }

      m_mp_tree.Expand(hmodule, TVE_EXPAND);
    }
  });
}

void CQLoaderDlg::OnBnClickedLaunch()
{
  // create the target process as a suspend process

  STARTUPINFOW si = { 0 };
  si.cb = sizeof(si);

  PROCESS_INFORMATION pi = { 0 };

  BOOL created = CreateProcessW(
    m_pe_path.GetBuffer(0),
    nullptr, nullptr, nullptr,
    FALSE,
    NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
    nullptr,
    m_pe_dir.GetBuffer(0),
    &si, &pi);

  auto process_name = vu::ExtractFileName(m_pe_path.GetBuffer(0));
  auto line = vu::FormatW(L"Create the process `%s` %s", process_name.c_str(), created ? L"succeed" : L"failed");
  this->AddLog(line, created ? status_t::success : status_t::error);

  if (!created)
  {
    this->AddLog(L"Finished");
    return;
  }

  vu::CProcessA process;
  process.Attach(pi.hProcess);

  // get base address of the target process

  DWORD size = 0;
  PROCESS_BASIC_INFORMATION pbi = { 0 };
  NTSTATUS status = NtQueryInformationProcess(pi.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &size);
  assert(NT_SUCCESS(status));

  vu::ulongptr base_address = 0;

  if (process.Bits() == vu::eXBit::x64)
  {
    PEB_T<vu::pe64> peb;
    process.Read(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = peb.ImageBaseAddress;
  }
  else // x86
  {
    PEB_T<vu::pe32> peb;
    process.Read(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = peb.ImageBaseAddress;
  }

  // get rva original entry point of the target process

  vu::ulongptr rva_oep = 0;
  {
    std::vector<byte> file;
    utils::read_file(m_pe_path.GetBuffer(0), file);
    auto pNTHeaders = ImageNtHeader(file.data());
    assert(pNTHeaders != nullptr);
    rva_oep = pNTHeaders->OptionalHeader.AddressOfEntryPoint;
  }

  // break the target process at va original entry point then resume and wait for fully loaded on memory

  auto va_oep = base_address + rva_oep;

  std::vector<byte> bp = { 0xEB, 0xFE }, ep(2);
  process.Read(va_oep, ep.data(), ep.size());
  process.Write(va_oep, bp.data(), bp.size());

  ResumeThread(pi.hThread);

  WaitForInputIdle(pi.hProcess, 1000); // 1s

  this->AddLog(L"Break the process at its entry point succeed", status_t::success);

  // suspend the target process

  SuspendThread(pi.hThread);

  this->AddLog(L"Suspend the process succeed", status_t::success);

  // copy the memory image of modules and store to a map

  const auto& modules = process.GetModules();

  struct module_t
  {
    vu::MODULEENTRY32 m_me;
    std::unique_ptr<vu::byte[]> m_buffer;
  };

  std::map<std::string, module_t> copied_modules;

  for (auto& item : g_jdata.items())
  {
    auto it = std::find_if(modules.cbegin(), modules.cend(), [&](const vu::MODULEENTRY32& me) -> bool
    {
      std::string v1 = me.szModule;
      v1 = vu::LowerStringA(v1);

      std::string v2 = item.key();
      v2 = vu::LowerStringA(v2);

      return v1 == v2;
    });

    if (it == modules.cend())
    {
      continue;
    }

    auto module_name = vu::LowerStringA(it->szModule);
    auto module_base = vu::ulongptr(it->modBaseAddr);
    auto module_size = it->modBaseSize + 1;

    auto ptr_raw_buffer = new vu::byte[module_size];
    process.Read(module_base, ptr_raw_buffer, module_size);

    auto& module = copied_modules[module_name];
    module.m_buffer.reset(ptr_raw_buffer);
    module.m_me = *it;

    // line = vu::ToStringW(item.key());
    // line = vu::FormatW(L"Find the module `%s` found", line.c_str());
    // this->AddLog(line, status_t::success);
  }

  if (!copied_modules.empty())
  {
    // iterate all patches and patch them all in the target process

    m_mp_tree.Iterate(m_mp_tree.GetRootItem(), [&](HTREEITEM pItem) -> void
    {
      auto ptr_jnode = reinterpret_cast<jnode_t*>(m_mp_tree.GetItemData(pItem));
      auto ptr_json = utils::to_ptr_json(ptr_jnode);
      if (ptr_json == nullptr) // must be json binding
      {
        return;
      }

      if (!ptr_json->is_object()) // must be json object (json object patch)
      {
        return;
      }

      auto& jpatch = *ptr_json;

      // find the module of patch

      const std::string module_name = vu::LowerStringA(ptr_jnode->m_module);
      const std::wstring wmodule_name = vu::ToStringW(module_name);
      auto it = copied_modules.find(module_name);
      if (it == copied_modules.cend())
      {
        line = vu::FormatW(L"Find the module `%s` not found", wmodule_name.c_str());
        this->AddLog(line, status_t::error);
        return;
      }

      line = vu::FormatW(L"Find the module `%s` found", wmodule_name.c_str());
      this->AddLog(line, status_t::success);

      // extract the name of patch

      auto name = utils::json_get(jpatch, "name", UNNAMED);
      std::wstring patch_name = vu::ToStringW(name);
      line = vu::FormatW(L"Try to patch `%s`", patch_name.c_str());
      this->AddLog(line);

      // extract the pattern bytes of patch

      const auto pattern = utils::json_get(jpatch, "pattern", EMPTY);

      // extract the replacement bytes of patch

      const auto replacement = utils::json_get(jpatch, "replacement", EMPTY);
      auto l = vu::SplitStringA(replacement, " ");
      std::vector<vu::byte> replacement_bytes;
      for (auto& e : l)
      {
        auto v = vu::byte(std::stoi(e, nullptr, 16));
        replacement_bytes.push_back(v);
      }

      // extract the offset of patch

      const auto offset = utils::json_get(jpatch, "offset", 0);

      // search pattern in the module

      auto address = static_cast<const void*>(it->second.m_buffer.get());
      auto size = it->second.m_me.modBaseSize;

      auto result = vu::FindPatternA(address, size, pattern);
      if (!result.first)
      {
        line = vu::FormatW(L"Find the patch `%s` not found", patch_name.c_str());
        this->AddLog(line, status_t::error);
        return;
      }

      line = vu::FormatW(L"Find the patch `%s` found", patch_name.c_str());
      this->AddLog(line, status_t::success);

      // patch at the found address with the replacement bytes

      auto found_patch_address = vu::ulongptr(it->second.m_me.modBaseAddr);
      found_patch_address += result.second;
      found_patch_address += offset;

      bool ret = process.Write(found_patch_address, replacement_bytes.data(), replacement_bytes.size());

      line = vu::FormatW(
        process.Bits() == vu::eXBit::x64 ?
          L"Patch `%s` at %016X %s" : L"Patch `%s` at %08X %s",
        patch_name.c_str(),
        process.Bits() == vu::eXBit::x64 ?
          vu::ulong64(found_patch_address) : vu::ulong32(found_patch_address),
        ret ? L"succeed" : L"failed");
      this->AddLog(line, ret ? status_t::success : status_t::error);
    });
  }
  else
  {
    this->AddLog(L"Not found any module for patching", status_t::warn);
  }

  // unbeak and resume the target process

  process.Write(va_oep, ep.data(), ep.size());

  ResumeThread(pi.hThread);

  this->AddLog(L"Resume the process succeed", status_t::success);

  this->AddLog(L"Finished");
}
