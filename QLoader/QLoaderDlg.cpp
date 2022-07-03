
// QLoaderDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "QLoaderApp.h"
#include "QLoaderDlg.h"
#include "AfxDialogEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog

class CAboutDlg : public CDialogEx
{
public:
  CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}
  virtual ~CAboutDlg() {};
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  #ifdef AFX_DESIGN_TIME
  enum { IDD = IDD_ABOUTBOX };
  #endif

protected:
  DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
  ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

afx_msg void CAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  __super::OnShowWindow(bShow, nStatus);
  this->GetDlgItem(IDOK)->SetFocus();
}

// CQLoaderDlg dialog

enum log_columns
{
  index,
  desc,
};

struct
{
  UINT id;
  PWCHAR tool_tip;
} list_ctrl_tool_tips[] = \
{
  { IDC_PE_OPEN, L"Select the target file" },
  { IDC_MP_OPEN, L"Select the pattern file" },
  { IDC_MP_SAVE, L"Save the pattern file" },
  { IDC_CLEAR, L"Clear log window" },
  { IDC_LAUNCH, L"Perform patching" },
  { IDC_EXPORT, L"Create a Loader as ..." },
  { IDCANCEL, L"Quit the application" },
  { IDC_PATCH_WHEN, L"Perform patching after the target is fully loaded into RAM" },
  { IDC_PATCH_WHEN_1, L"Perform patching when the target runs to Entry Point" },
  { IDC_PATCH_WHEN_2, L"Perform patching after the target is unpacked in RAM (for packed target)" },
};

CQLoaderDlg::CQLoaderDlg(CWnd* pParent)
  : CDialogEx(CQLoaderDlg::IDD, pParent)
  , m_pe_path(_T(""))
  , m_pe_dir(_T(""))
  , m_pe_arg(_T(""))
  , m_mp_path(_T(""))
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CQLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
  __super::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PE_PATH, m_pe_path);
  DDX_Text(pDX, IDC_PE_DIR, m_pe_dir);
  DDX_Text(pDX, IDC_PE_ARG, m_pe_arg);
  DDX_Text(pDX, IDC_MP_PATH, m_mp_path);
  DDX_Control(pDX, IDC_MP_TREE, m_mp_tree);
  DDX_Control(pDX, IDC_MP_SAVE, m_button_mp_save);
  DDX_Control(pDX, IDC_EXPORT, m_button_export);
  DDX_Control(pDX, IDC_LAUNCH, m_button_launch);
  DDX_Radio(pDX, IDC_PATCH_WHEN, m_patch_when);
  DDX_Control(pDX, IDC_LOG, m_log);
}

BEGIN_MESSAGE_MAP(CQLoaderDlg, CDialogEx)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_DROPFILES()
  ON_BN_CLICKED(IDC_PE_OPEN, OnBnClicked_PEOpen)
  ON_BN_CLICKED(IDC_MP_OPEN, OnBnClicked_MPOpen)
  ON_BN_CLICKED(IDC_MP_SAVE, OnBnClicked_MPSave)
  ON_BN_CLICKED(IDC_CLEAR, OnBnClicked_Clear)
  ON_BN_CLICKED(IDC_LAUNCH, OnBnClicked_Launch)
  ON_BN_CLICKED(IDC_EXPORT, OnBnClicked_Export)
  ON_COMMAND(IDM_EXPORT_LNK, OnBnClicked_ExportLNK)
  ON_COMMAND(IDM_EXPORT_URL, OnBnClicked_ExportURL)
  ON_COMMAND(IDM_EXPORT_HTML, OnBnClicked_ExportHTML)
  ON_EN_KILLFOCUS(IDC_PE_DIR, OnUpdate_UIData)
  ON_EN_KILLFOCUS(IDC_PE_ARG, OnUpdate_UIData)
  ON_BN_CLICKED(IDC_PATCH_WHEN, OnUpdate_UIData)
  ON_BN_CLICKED(IDC_PATCH_WHEN_1, OnUpdate_UIData)
  ON_BN_CLICKED(IDC_PATCH_WHEN_2, OnUpdate_UIData)
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

  // Custom application's title
  CString title;
  GetWindowText(title);
  title += vu::is_administrator() ? L" [ELEVATED]" : L"";
  SetWindowText(title);

  // Enable tool-tip for controls
  m_tool_tip.Create(this);
  for (auto& e : list_ctrl_tool_tips)
  {
    m_tool_tip.AddTool(GetDlgItem(e.id), e.tool_tip);
  }
  m_tool_tip.Activate(TRUE);

  // Enable Drag and Drop for an Elevated MFC application on Windows
  ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);
  ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
  DragAcceptFiles(TRUE);

  if (!m_ph.is_registered() && !m_ph.do_register())
  {
    vu::msg_box_W(this->GetSafeHwnd(), MB_OK | MB_ICONWARNING, NAME,
      L"Required to run as administrator at the first time.");
  }

  this->initialize_ui();
  this->reset_ui();

  json mp_jdata;
  auto patch_when = 0;
  std::wstring pe_path, pe_dir, pe_arg;
  if (this->parse_cmd_line(AfxGetApp()->m_lpCmdLine, patch_when, pe_path, pe_dir, pe_arg, mp_jdata))
  {
    m_patch_when = patch_when;
    m_pe_path = pe_path.c_str();
    m_pe_dir = pe_dir.c_str();
    m_pe_arg = pe_arg.c_str();
    m_mp_jdata = mp_jdata;

    UpdateData(FALSE);

    this->update_ui();

    this->SendMessage(WM_COMMAND, IDC_LAUNCH);

    ExitProcess(0);
  }

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

HCURSOR CQLoaderDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(m_hIcon);
}

BOOL CQLoaderDlg::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
  {
    return TRUE;
  }

  m_tool_tip.RelayEvent(pMsg);

  return __super::PreTranslateMessage(pMsg);
}

void CQLoaderDlg::OnDropFiles(HDROP hDropInfo)
{
  // TODO: Add your message handler code here and/or call default

  m_file_paths.clear();

  UINT nFilesDropped = DragQueryFile(hDropInfo, 0xFFFFFFFF, nullptr, 0);
  for (UINT nIndex = 0; nIndex < nFilesDropped; ++nIndex)
  {
    CString file_path;
    DWORD file_path_length = MAX_PATH; // DragQueryFile(hDropInfo, 0, nullptr, 0) + 1;
    DragQueryFile(hDropInfo, nIndex, file_path.GetBuffer(file_path_length), file_path_length);
    if (file_supported(file_path.GetBuffer(0)))
    {
      m_file_paths.push_back(file_path.GetString());
    }
    file_path.ReleaseBuffer();
  }

  DragFinish(hDropInfo);

  __super::OnDropFiles(hDropInfo);

  this->update_ui();
}

void CQLoaderDlg::OnBnClicked_PEOpen()
{
  m_file_paths.clear();

  std::wstring file_path;
  const auto file_filter = L"EXE File\0*.exe\0All Files (*.*)\0*.*\0";
  vu::Picker picker(this->GetSafeHwnd());
  if (picker.choose_file(vu::Picker::action_type::open, file_path, L"", file_filter))
  {
    m_file_paths.push_back(file_path);
  }

  this->update_ui();
}

void CQLoaderDlg::OnBnClicked_MPOpen()
{
  m_file_paths.clear();

  std::wstring file_path;
  const auto file_filter = L"JSON File\0*.json\0All Files (*.*)\0*.*\0";
  vu::Picker picker(this->GetSafeHwnd());
  if (picker.choose_file(vu::Picker::action_type::open, file_path, L"", file_filter))
  {
    m_file_paths.push_back(file_path);
  }

  this->update_ui();
}

void CQLoaderDlg::OnBnClicked_MPSave()
{
  std::wstring file_path = m_mp_path.GetBuffer(0);
  const auto file_filter = L"JSON File\0*.json\0All Files (*.*)\0*.*\0";
  vu::Picker picker(this->GetSafeHwnd());
  if (picker.choose_file(vu::Picker::action_type::save, file_path, L"", file_filter))
  {
    try
    {
      std::ofstream ofs;
      ofs.open(file_path);
      ofs << m_mp_jdata.dump(1, '\t');
      ofs.close();

      auto file_name = vu::extract_file_name(file_path);
      auto line = vu::format(L"Save MP data to `%s` succeed", file_name.c_str());
      this->add_log(line, QLoader::status_t::success);
    }
    catch (...)
    {
      auto file_name = vu::extract_file_name(file_path);
      auto line = vu::format(L"Save MP data to `%s` failed", file_name.c_str());
      this->add_log(line, QLoader::status_t::error);
    }
  }
}

void CQLoaderDlg::OnBnClicked_Clear()
{
  m_log.DeleteAllItems();
}

void CQLoaderDlg::OnBnClicked_Launch()
{
  UpdateData(TRUE);

  this->launch(
    launch_t(m_patch_when), m_pe_path.GetBuffer(0), m_pe_dir.GetBuffer(0), m_pe_arg.GetBuffer(0));
}

void CQLoaderDlg::OnBnClicked_Export()
{
  this->SendMessage(WM_COMMAND, IDM_EXPORT_LNK);
}

void CQLoaderDlg::OnBnClicked_ExportLNK()
{
  UpdateData(TRUE);

  vu::PickerW picker;
  std::wstring file_dir;
  if (!picker.choose_directory(file_dir))
  {
    return;
  }

  std::wstring file_name = vu::extract_file_name_W(m_pe_path.GetBuffer(0), false);
  file_name += L" (QLoader).LNK";

  vu::PathW path;
  path.join(file_dir).join(file_name).finalize();
  const std::wstring file_path = path.as_string();

  const auto lnk = this->export_as_lnk(
    launch_t(m_patch_when), m_pe_path.GetBuffer(0), m_pe_dir.GetBuffer(0), m_pe_arg.GetBuffer(0));

  const bool succeed = vu::create_shortcut_lnk_W(file_path, lnk) == vu::VU_OK;

  auto line = vu::format(L"Export as Windows Shortcut to `%s` %s",
    file_path.c_str(), succeed ? L"succeed" : L"failed");
  this->add_log(line, succeed ? QLoader::status_t::success : QLoader::status_t::error);
}

void CQLoaderDlg::OnBnClicked_ExportURL()
{
  UpdateData(TRUE);

  vu::PickerW picker;
  std::wstring file_dir;
  if (!picker.choose_directory(file_dir))
  {
    return;
  }

  std::wstring file_name = vu::extract_file_name_W(m_pe_path.GetBuffer(0), false);
  file_name += L" (QLoader).URL";

  vu::PathW path;
  path.join(file_dir).join(file_name).finalize();
  const std::wstring file_path = path.as_string();

  const auto lnk = this->export_as_lnk(
    launch_t(m_patch_when), m_pe_path.GetBuffer(0), m_pe_dir.GetBuffer(0), m_pe_arg.GetBuffer(0));

  const wchar_t* url_file_content_template =
    L"[{000214A0-0000-0000-C000-000000000046}]\n"
    L"Prop3=19,0\n"
    L"[InternetShortcut]\n"
    L"URL=%s %s\n"
    L"IDList=\n"
    L"HotKey=0\n"
    L"IconFile=%s\n"
    L"IconIndex=%d\n";

  auto url_file_content_W = vu::format_W(url_file_content_template,
    m_ph.name(true).c_str(), lnk.argument.c_str(), lnk.icon.first.c_str(), lnk.icon.second);
  auto url_file_content_A = vu::to_string_A(url_file_content_W);

  std::vector<vu::byte> url_file_content(url_file_content_A.cbegin(), url_file_content_A.cend());
  const bool succeed = vu::write_file_binary_W(file_path, url_file_content);

  auto line = vu::format(L"Export as Internet Shortcut to `%s` %s",
    file_path.c_str(), succeed ? L"succeed" : L"failed");
  this->add_log(line, succeed ? QLoader::status_t::success : QLoader::status_t::error);
}

void CQLoaderDlg::OnBnClicked_ExportHTML()
{
  UpdateData(TRUE);

  const auto lnk = this->export_as_lnk(
    launch_t(m_patch_when), m_pe_path.GetBuffer(0), m_pe_dir.GetBuffer(0), m_pe_arg.GetBuffer(0));

  std::wstring file_name = vu::extract_file_name_W(m_pe_path.GetBuffer(0), false);
  file_name += L" (QLoader)";

  std::wstring lnk_argument_url_encoded;
  vu::url_encode_W(lnk.argument, lnk_argument_url_encoded);

  const auto html_hyperlink = vu::format_W(L"<a href=\"%s %s\">%s</a>",
    m_ph.name(true).c_str(), lnk_argument_url_encoded.c_str(), file_name.c_str());

  const bool succeed = vu::copy_to_clipboard_W(html_hyperlink);

  auto line = vu::format(L"Export as HTML Hyper-link to %s %s",
    L"Clipboard", succeed ? L"succeed" : L"failed");
  this->add_log(line, succeed ? QLoader::status_t::success : QLoader::status_t::error);
}

void CQLoaderDlg::OnUpdate_UIData()
{
  UpdateData(TRUE);
}

void CQLoaderDlg::add_log(const std::wstring& line, const status_t status)
{
  LVITEM lvi = { 0 };
  lvi.mask = LVIF_IMAGE;
  lvi.iItem = m_log.GetItemCount();
  lvi.iSubItem = log_columns::index;
  lvi.iImage = int(status);
  lvi.pszText = LPWSTR(line.c_str());
  int index = m_log.InsertItem(&lvi);
  m_log.SetItemText(index, log_columns::desc, line.c_str());
  m_log.EnsureVisible(index, TRUE);
  m_log.SetColumnWidth(log_columns::desc, LVSCW_AUTOSIZE_USEHEADER);
}

void CQLoaderDlg::reset_ui()
{
  m_file_paths.clear();

  m_pe_path = _T("");
  m_pe_dir = _T("");
  m_pe_arg = _T("");
  m_mp_path = _T("");
  m_patch_when = int(launch_t::fully_loaded);

  m_mp_jdata.clear();
  m_mp_tree.Clear();
  m_log.DeleteAllItems();
  m_button_mp_save.EnableWindow(FALSE);
  m_button_export.EnableWindow(FALSE);
  m_button_launch.EnableWindow(FALSE);

  m_button_export.SetDropDownMenu(IDR_EXPORT, 0);

  UpdateData(FALSE);
}

void CQLoaderDlg::update_ui()
{
  // if (m_file_paths.empty())
  // {
  //   return;
  // }

  bool found_target  = false;
  bool found_pattern = false;

  for (auto& file_path : m_file_paths)
  {
    auto file_path_tmp = file_path;
    file_path_tmp = vu::upper_string(file_path_tmp);

    if (!found_target)
    {
      if (auto ptr_lnk = this->parse_shortcut(file_path_tmp))
      {
        json mp_jdata;
        auto patch_when = 0;
        std::wstring pe_path, pe_dir, pe_arg;
        if (this->parse_cmd_line(ptr_lnk->argument, patch_when, pe_path, pe_dir, pe_arg, mp_jdata) &&
            AfxMessageBox(L"Detected QLoader file shortcut.\nWould you like to parse it?",
              MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
          m_patch_when = patch_when;
          m_pe_path = pe_path.c_str();
          m_pe_dir = pe_dir.c_str();
          m_pe_arg = pe_arg.c_str();
          m_mp_jdata = mp_jdata;
        }
        else
        {
          m_pe_path = ptr_lnk->path.c_str();
          m_pe_dir = ptr_lnk->directory.c_str();
          m_pe_arg = ptr_lnk->argument.c_str();
        }

        found_target = true;
      }
    }

    if (!found_target && vu::ends_with(file_path_tmp, L".EXE", true))
    {
      m_pe_path = file_path.c_str();
      m_pe_dir = vu::extract_file_directory(file_path.c_str()).c_str();
      found_target = true;
    }

    if (!found_pattern && vu::ends_with(file_path_tmp, L".JSON", true))
    {
      m_mp_path = file_path.c_str();
      found_pattern = true;
    }
  }

  m_file_paths.clear();

  if (found_pattern)
  {
    std::wstring mp_path = m_mp_path.GetBuffer(0);
    if (vu::is_file_exists(mp_path))
    {
      std::string s = vu::to_string_A(mp_path);
      std::ifstream fs(s);
      m_mp_jdata = json::parse(fs, nullptr, true, true);
    }
  }

  this->populate_tree();

  m_button_mp_save.EnableWindow(!m_mp_jdata.is_null());
  m_button_export.EnableWindow(!m_pe_dir.IsEmpty() && !m_pe_path.IsEmpty() && !m_mp_jdata.is_null());
  m_button_launch.EnableWindow(!m_pe_dir.IsEmpty() && !m_pe_path.IsEmpty() && !m_mp_jdata.is_null());

  UpdateData(FALSE);

  RedrawWindow();
}

void CQLoaderDlg::initialize_ui()
{
  m_mp_tree.OnNotify([&](EasyTreeCtrl::eNotifyType action, Node* pNode) -> bool
  {
    switch (action)
    {
    case EasyTreeCtrl::eNotifyType::BEFORE_MODIFYING:
    {
      auto ptr_jnode = static_cast<jnode_t*>(pNode);
      return // continue processing if data node
        ptr_jnode != nullptr &&
        ptr_jnode->m_ptr_data != nullptr &&
        ptr_jnode->m_ptr_tv->state & TVIF_IMAGE;
    }
    break;

    case EasyTreeCtrl::eNotifyType::AFTER_MODIFYING:
    {
      if (pNode != nullptr &&
          pNode->m_ptr_data != nullptr &&
          pNode->m_ptr_tv != nullptr &&
          pNode->m_ptr_tv->pszText)
      {
        auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
        if (ptr_jobject != nullptr)
        {
          auto new_value = vu::to_string_A(pNode->m_ptr_tv->pszText);
          auto old_value = EMPTY;

          bool modified = false;

          auto& jobject = static_cast<json&>(*ptr_jobject);
          if (jobject.is_string())
          {
            old_value = jobject.get<std::string>();
            jobject = new_value;
            modified = true;
          }
          else if (jobject.is_number())
          {
            old_value = std::to_string(jobject.get<int>());
            jobject = std::atoi(new_value.c_str());
            modified = true;
          }

          if (modified)
          {
            auto line = vu::format(L"Modify `%S` to `%S` succeed", old_value.c_str(), new_value.c_str());
            this->add_log(line, status_t::success);
          }
        }
      }
    }
    break;

    case EasyTreeCtrl::eNotifyType::BEFORE_DELETING:
    {
      auto ptr_jnode = static_cast<jnode_t*>(pNode);
      return // continue processing if data node
        ptr_jnode != nullptr &&
        ptr_jnode->m_ptr_data != nullptr &&
        ptr_jnode->m_ptr_tv->state & TVIS_STATEIMAGEMASK;
    }
    break;

    case EasyTreeCtrl::eNotifyType::AFTER_DELETING:
    {
      auto ptr_jnode = static_cast<jnode_t*>(pNode);
      if (ptr_jnode == nullptr)
      {
        return false;
      }

      assert(pNode->m_ptr_data != nullptr);

      auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
      if (ptr_jobject != nullptr)
      {
        auto  ptr_jparent = static_cast<jnode_t*>(pNode)->m_ptr_parent;
        auto& jparent = static_cast<json&>(*ptr_jparent);
        auto& jobject = static_cast<json&>(*ptr_jobject);

        bool  deleted = false;
        json* ptr_jitems = nullptr;

        auto& jmodules = m_mp_jdata["modules"];
        auto  item_name = jobject.get<std::string>();

        const auto fn_find_and_delete = [&](json& jitems, const std::string& item, bool patch)
        {
          for (auto it = jitems.begin(); it != jitems.end(); it++)
          {
            auto& jitem = *it;

            bool checked = true;
            if (patch)
            {
              checked &= jitem["pattern"] == jparent["pattern"];
              checked &= jitem["replacement"] == jparent["replacement"];
              checked &= jitem["offset"] == jparent["offset"];
            }

            auto key = json_get(jitem, "name", EMPTY);
            if (key != EMPTY && item != EMPTY && item == key && checked)
            {
              jitems.erase(it);
              return true;
            }
          }

          return false;
        };

        if (jparent.find("offset") != jparent.end()) // delete a patch
        {
          for (auto& jmodule : jmodules)
          {
            if (fn_find_and_delete(jmodule["patches"], item_name, true))
            {
              deleted = true;
              break;
            }
          }
        }
        else // delete a module
        {
          deleted = fn_find_and_delete(jmodules, item_name, false);
        }

        if (deleted)
        {
          auto line = vu::format(L"Delete `%S` succeed", item_name.c_str());
          this->add_log(line, status_t::success);

          this->update_ui();
        }
      }
    }
    break;

    case EasyTreeCtrl::eNotifyType::BOX_CHECKING:
    {
      if (pNode != nullptr &&
          pNode->m_ptr_data != nullptr &&
          pNode->m_ptr_tv != nullptr)
      {
        auto ptr_jobject = static_cast<json*>(pNode->m_ptr_data);
        if (ptr_jobject != nullptr)
        {
          auto  ptr_jparent = static_cast<jnode_t*>(pNode)->m_ptr_parent;
          auto& jparent = static_cast<json&>(*ptr_jparent);

          bool checked = m_mp_tree.GetCheck(pNode->m_ptr_tv->hItem) == BST_CHECKED;
          jparent["enabled"] = !checked; // this will add to json object if not exists
          m_mp_tree.SetCheck(pNode->m_ptr_tv->hItem, checked ? BST_UNCHECKED : BST_CHECKED);

          auto line = vu::to_string_W(json_get(jparent, "name", EMPTY));
          line = vu::format(L"%s `%s` succeed", checked ? L"Disable" : L"Enable", line.c_str());
          this->add_log(line, status_t::success);
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
    file << m_mp_jdata.dump(1, '\t');

    #endif // _DEBUG

    return true;
  });

  // for log ctrl

  m_log.SetExtendedStyle(m_log.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  static CImageList image_list;
  image_list.Create(IDB_LOG_SMALL, 16, 1, 0xFFFFFF);
  m_log.SetImageList(&image_list, LVSIL_SMALL);

  const int DEFAULT_COLUMN_WIDTH = 20;

  LVCOLUMN lvc = { 0 };

  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = DEFAULT_COLUMN_WIDTH;
  lvc.pszText = L"#";
  m_log.InsertColumn(log_columns::index, &lvc);

  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = DEFAULT_COLUMN_WIDTH;
  lvc.pszText = L"Description";
  m_log.InsertColumn(log_columns::desc, &lvc);

  this->SendMessage(WM_SIZE); // request to re-calculate size for window and its controls
}

void CQLoaderDlg::populate_tree()
{
  if (m_mp_jdata.is_null())
  {
    return;
  }

  m_mp_tree.Clear();

  m_mp_tree.ModifyStyle(0, TVS_CHECKBOXES);

  m_mp_tree.Populate([&](HTREEITEM& root) -> void
  {
    m_mp_tree.SetItemState(root, 0, TVIS_STATEIMAGEMASK);

    auto fn_tree_add_node_str = [&](HTREEITEM& hparent, json& jobject, std::string key) -> HTREEITEM
    {
      auto hitem = m_mp_tree.InsertNode(hparent, new jnode_t(key));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      auto string = json_get(jobject, key, EMPTY);
      hitem = m_mp_tree.InsertNode(hitem, new jnode_t(string, &jobject[key], &jobject));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      return hitem;
    };

    auto fn_tree_add_node_int = [&](HTREEITEM& hparent, json& jobject, std::string key) -> HTREEITEM
    {
      auto hitem = m_mp_tree.InsertNode(hparent, new jnode_t(key));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      auto number = json_get(jobject, key, 0);
      hitem = m_mp_tree.InsertNode(hitem, new jnode_t(std::to_string(number), &jobject[key], &jobject));
      m_mp_tree.SetItemState(hitem, 0, TVIS_STATEIMAGEMASK);
      return hitem;
    };

    for (auto& jmodule : m_mp_jdata["modules"])
    {
      auto module_name = json_get(jmodule, "name", EMPTY);
      auto hmodule = m_mp_tree.InsertNode(root, new jnode_t(module_name, &jmodule["name"], &jmodule));
      m_mp_tree.SetCheck(hmodule, json_get(jmodule, "enabled", true));

      for (auto& jpatch : jmodule["patches"])
      {
        auto patch_name = json_get(jpatch, "name", UNNAMED);
        if (auto hpatch = m_mp_tree.InsertNode(hmodule, new jnode_t(patch_name, &jpatch["name"], &jpatch)))
        {
          m_mp_tree.SetCheck(hpatch, json_get(jpatch, "enabled", true));
          fn_tree_add_node_str(hpatch, jpatch, "pattern");
          fn_tree_add_node_str(hpatch, jpatch, "replacement");
          fn_tree_add_node_str(hpatch, jpatch, "offset");
        }
      }

      m_mp_tree.Expand(hmodule, TVE_EXPAND);
    }
  });
}
