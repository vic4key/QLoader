// QLoader.cpp : implementation file
//

#include "pch.h"
#include "QLoader.h"

#include <map>
#include <vector>
#include <psapi.h>

#include <winternl.h>
#pragma comment(lib, "ntdll")

#include <dbghelp.h>
#pragma comment(lib, "dbghelp")

static json g_mp_jdata;

QLoader::QLoader() : m_mp_jdata(g_mp_jdata), m_ph(NAME)
{
}

QLoader::~QLoader()
{
}

bool QLoader::file_supported(const std::wstring& file_path)
{
  if (file_path.empty())
  {
    return false;
  }

  std::wstring file_extension = PathFindExtension(file_path.c_str());
  file_extension = vu::upper_string(file_extension);

  auto it = std::find(USABLE_FILE_EXTENSIONS.cbegin(), USABLE_FILE_EXTENSIONS.cend(), file_extension);
  if (it == USABLE_FILE_EXTENSIONS.cend())
  {
    return false;
  }

  return true;
}

void QLoader::add_log(const std::wstring& line, const status_t status)
{
  assert(0 && "missing implementation");
}

void QLoader::launch(
  const launch_t mode,
  const std::wstring& pe_file_path,
  const std::wstring& pe_file_dir,
  const std::wstring& pe_file_arg)
{
  // create the target process as a suspend process

  PROCESS_INFORMATION pi = { 0 };

  std::wstring args = pe_file_arg;
  if (!args.empty())
  {
    std::wstring tmp = L"";
    tmp  = L'\"';
    tmp += pe_file_path;
    tmp += L'\"';
    tmp += +L' ';
    args = tmp + args;
  }

  vu::uint creation_flags = NORMAL_PRIORITY_CLASS;

  if (mode == launch_t::at_entry_point)
  {
    creation_flags |= CREATE_SUSPENDED;
  }

  vu::ProcessW process;
  bool created = process.create(
    pe_file_path, pe_file_dir, args, creation_flags, false, &pi);

  auto pe_file_name = vu::extract_file_name(pe_file_path);
  auto line = vu::format(L"Create the process `%s` %s", pe_file_name.c_str(), created ? L"succeed" : L"failed");
  this->add_log(line, created ? status_t::success : status_t::error);

  if (!created)
  {
    this->add_log(L"Finished");
    return;
  }

  // select the patching mode

  vu::ulongptr va_oep = 0;
  std::vector<byte> ep;

  if (mode == launch_t::fully_loaded)
  {
    WaitForInputIdle(pi.hProcess, INFINITE); // waiting for the target process fully loaded on memory
    this->add_log(L"The created process is fully loaded on memory");
  }
  else if (mode == launch_t::at_entry_point)
  {
    va_oep = this->launch_with_patch_at_oep(process, pi, ep); // waiting for the target process break at oep
  }

  // suspend the target process

  SuspendThread(pi.hThread);

  this->add_log(L"Suspend the process succeed", status_t::success);

  // find and copy the loaded module in the target process and store them into a map

  const auto& in_target_modules = process.get_modules();

  struct module_t
  {
    vu::MODULEENTRY32W m_me;
    std::unique_ptr<vu::byte[]> m_buffer;
  };

  std::map<std::wstring, module_t> copied_modules;

  const auto& jmodules = m_mp_jdata["modules"];
  for (const auto& jmodule : jmodules)
  {
    auto it = std::find_if(in_target_modules.cbegin(), in_target_modules.cend(),
    [&](const vu::MODULEENTRY32W& me) -> bool
    {
      std::string v1 = vu::to_string_A(me.szModule);
      v1 = vu::lower_string_A(v1);

      std::string v2 = json_helper::get(jmodule, "name", EMPTY);
      v2 = vu::lower_string_A(v2);

      return v1 == v2;
    });

    if (it == in_target_modules.cend())
    {
      continue;
    }

    // store the found module in the target process

    auto module_name = vu::lower_string_W(std::wstring(it->szModule));
    auto module_base = vu::ulongptr(it->modBaseAddr);
    auto module_size = it->modBaseSize + 1;

    auto ptr_raw_buffer = new vu::byte[module_size];
    process.read_memory(module_base, ptr_raw_buffer, module_size);

    auto& module = copied_modules[module_name];
    module.m_buffer.reset(ptr_raw_buffer);
    module.m_me = *it;
  }

  if (!copied_modules.empty())
  {
    // iterate all modules

    const auto& jmodules = m_mp_jdata["modules"];
    for (const auto& jmodule : jmodules) // iterate all modules
    {
      const auto module_name = vu::to_string_W(json_helper::get(jmodule, "name", EMPTY));
      if (module_name.empty())
      {
        continue;
      }

      // ignore the disable module

      bool enabled = json_helper::get(jmodule, "enabled", true);
      if (!enabled)
      {
        auto line = vu::format(L"Ignore the module `%s`", module_name.c_str());
        this->add_log(line, status_t::warn);
        continue;
      }

      // ignore the not found module

      auto it = copied_modules.find(vu::lower_string_W(module_name));
      if (it == copied_modules.cend()) // find the module of patch
      {
        auto line = vu::format(L"Find the module `%s` not found", module_name.c_str());
        this->add_log(line, status_t::error);
        continue;
      }

      // iterate all patches

      const auto& jpatches = jmodule["patches"];
      for (const auto& jpatch : jpatches)
      {
        auto line = vu::format(L"Find the module `%s` found", module_name.c_str());
        this->add_log(line, status_t::success);

        // extract the name of patch

        auto name = json_helper::get(jpatch, "name", UNNAMED);
        std::wstring patch_name = vu::to_string_W(name);
        line = vu::format(L"Try to patch `%s`", patch_name.c_str());
        this->add_log(line);

        // extract the enabled of patch

        const bool enabled = json_helper::get(jpatch, "enabled", true);
        if (!enabled)
        {
          auto line = vu::format(L"Ignore the patch `%s`", patch_name.c_str());
          this->add_log(line, status_t::warn);
          continue;
        }

        // extract the pattern bytes of patch

        const auto pattern = json_helper::get(jpatch, "pattern", EMPTY);
        assert(!pattern.empty());

        // extract the replacement bytes of patch

        const auto replacement = json_helper::get(jpatch, "replacement", EMPTY);
        std::vector<vu::byte> replacement_bytes;
        vu::to_hex_bytes_A(replacement, replacement_bytes);
        assert(!replacement_bytes.empty());

        // extract the offset of patch

        auto offset_s = json_helper::get(jpatch, "offset", EMPTY);
        offset_s = vu::trim_string_A(offset_s);
        bool is_heximal = \
          vu::contains_string_A(offset_s, "0x", true) ||
          vu::contains_string_A(offset_s, "h", true);
        const int offset = std::stoi(offset_s, nullptr, is_heximal ? 16 : 10);

        // search pattern in the module

        auto address = static_cast<const void*>(it->second.m_buffer.get());
        auto size = it->second.m_me.modBaseSize;

        auto result = vu::find_pattern_A(address, size, pattern, true);
        if (result.empty())
        {
          auto line = vu::format(L"Find the patch `%s` not found", patch_name.c_str());
          this->add_log(line, status_t::error);
          continue;
        }

        line = vu::format(L"Find the patch `%s` found", patch_name.c_str());
        this->add_log(line, status_t::success);

        // patch at the found address with the replacement bytes

        auto found_patch_address = vu::ulongptr(it->second.m_me.modBaseAddr);
        found_patch_address += result.at(0);
        found_patch_address += offset;

        bool ret = process.write_memory(found_patch_address, replacement_bytes.data(), replacement_bytes.size());

        line = vu::format(
          process.bit() == vu::arch::x64 ?
          L"Patch `%s` at %016X %s" : L"Patch `%s` at %08X %s",
          patch_name.c_str(),
          process.bit() == vu::arch::x64 ?
          vu::ulong64(found_patch_address) : vu::ulong32(found_patch_address),
          ret ? L"succeed" : L"failed");
        this->add_log(line, ret ? status_t::success : status_t::error);
      }
    }
  }
  else
  {
    this->add_log(L"Not found any module for patching", status_t::warn);
  }

  if (mode == launch_t::at_entry_point)
  {
    process.write_memory(va_oep, ep.data(), ep.size()); // restore oep and resume the target process
  }

  ResumeThread(pi.hThread);

  this->add_log(L"Resume the process succeed", status_t::success);

  this->add_log(L"Finished");
}

vu::ulongptr QLoader::launch_with_patch_at_oep(
  vu::ProcessW& process, PROCESS_INFORMATION& process_information, std::vector<byte>& backup_entry_point_bytes)
{
  backup_entry_point_bytes.clear();

  // get base address of the target process

  DWORD size = 0;
  PROCESS_BASIC_INFORMATION pbi = { 0 };
  NTSTATUS status = NtQueryInformationProcess(
    process_information.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &size);
  assert(NT_SUCCESS(status));

  vu::ulongptr base_address = 0;

  if (process.bit() == vu::arch::x64)
  {
    PEB_T<vu::pe64> peb;
    process.read_memory(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = vu::ulongptr(peb.ImageBaseAddress);
  }
  else // x86
  {
    PEB_T<vu::pe32> peb;
    process.read_memory(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = vu::ulongptr(peb.ImageBaseAddress);
  }

  // get rva original entry point of the target process

  vu::ulongptr rva_oep = 0;
  {
    std::vector<vu::byte> buffer;
    vu::read_file_binary(process.get_path(), buffer);

    auto ptr_nt_header = ImageNtHeader(buffer.data());
    assert(ptr_nt_header != nullptr);

    rva_oep = ptr_nt_header->OptionalHeader.AddressOfEntryPoint;
  }

  // break the target process at va original entry point then resume and wait for fully loaded on memory

  const auto va_oep = base_address + rva_oep;

  std::vector<byte> break_point_bytes = { 0xEB, 0xFE }; // JMP XIP - a loop jump to itself that treat as a break-point
  backup_entry_point_bytes.resize(break_point_bytes.size());
  process.read_memory(va_oep, backup_entry_point_bytes.data(), backup_entry_point_bytes.size());
  process.write_memory(va_oep, break_point_bytes.data(), break_point_bytes.size());

  ResumeThread(process_information.hThread);

  WaitForInputIdle(process_information.hProcess, 1000); // 1s

  auto fmt = process.bit() == vu::arch::x64 ?
    L"Break the process at its entry point %016X succeed" : L"Break the process at its entry point %08X succeed";
  auto line = vu::format(fmt, process.bit() == vu::arch::x64 ? vu::ulong64(va_oep) : vu::ulong32(va_oep));
  this->add_log(line, status_t::success);

  return va_oep;
}

vu::LNKW QLoader::export_as_lnk(
  const launch_t mode,
  const std::wstring& pe_file_path,
  const std::wstring& pe_file_dir,
  const std::wstring& pe_file_arg)
{
  vu::LNKW result;

  result.path = vu::get_current_file_path_W();

  result.argument = vu::format_W(L"-mode %d ", int(mode));

  json pe_jdata;
  pe_jdata["path"] = vu::to_string_A(pe_file_path);
  pe_jdata["dir"] = vu::to_string_A(pe_file_dir);
  pe_jdata["arg"] = vu::to_string_A(pe_file_arg);
  const auto pe_string = pe_jdata.dump();
  std::vector<vu::byte> pe_data(pe_string.cbegin(), pe_string.cend());
  std::wstring pe_data_encoded;
  vu::crypt_b64encode_W(pe_data, pe_data_encoded);
  result.argument += L"-pe \"" + pe_data_encoded + L"\"";

  result.argument += L" ";

  const auto mp_string = m_mp_jdata.dump();
  std::vector<vu::byte> mp_data(mp_string.cbegin(), mp_string.cend());
  std::wstring mp_data_encoded;
  vu::crypt_b64encode_W(mp_data, mp_data_encoded);
  result.argument += L"-mp \"" + mp_data_encoded + L"\"";

  result.directory = vu::get_current_directory_W();

  result.description = std::wstring(L"QLoader for ") + vu::extract_file_name_W(pe_file_path);

  result.icon.first = pe_file_path;
  result.icon.second = 0;

  return result;
}

bool QLoader::parse_cmd_line(
  const std::wstring& cmd_line,
  int& patch_when,
  std::wstring& pe_file_path,
  std::wstring& pe_file_dir,
  std::wstring& pe_file_arg,
  json& mp_jdata)
{
  patch_when = 0;
  pe_file_path = L"";
  pe_file_dir = L"";
  pe_file_arg = L"";

  // protocol handler format, convert to a command line string
  // Eg. "QLoader: -mode 0 -pe "..." -mp "...""

  auto cmd_line_tmp = vu::trim_string_W(cmd_line);
  if (m_ph.is_me(cmd_line_tmp, true))
  {
    std::wstring url_decoded;
    vu::url_decode_W(cmd_line_tmp, url_decoded);
    if (!url_decoded.empty())
    {
      cmd_line_tmp = url_decoded;
    }

    if (cmd_line_tmp.empty())
    {
      return false;
    }

    cmd_line_tmp.assign(cmd_line_tmp.cbegin() + 1, cmd_line_tmp.cend() - 1);

    if (m_ph.is_me(cmd_line_tmp))
    {
      cmd_line_tmp.assign(cmd_line_tmp.cbegin() + m_ph.name(true).size(), cmd_line_tmp.cend());
      cmd_line_tmp = vu::trim_string_W(cmd_line_tmp);
    }

    if (cmd_line_tmp.empty())
    {
      return false;
    }
  }

  // serialize a command line string to a list of arguments

  auto argc = 0;
  auto argv = CommandLineToArgvW(cmd_line_tmp.c_str(), &argc);
  if (argc != 6 || argv == nullptr) // Eg. `-mode <mode> -pe <pe_data> -mp <mp_data>`
  {
    return false;
  }

  std::vector<std::wstring> args(argc);
  for (int i = 0; i < argc; i++)
  {
    args[i].assign(argv[i]);
  }

  // parsing patching mode

  if (args[0] == L"-mode")
  {
    const auto s = args[1];
    if (s.length() == 1 && isdigit(s[0]))
    {
      patch_when = std::wcstol(s.c_str(), nullptr, 10);
    }
  }

  // parsing pe fields

  if (args[2] == L"-pe")
  {
    std::vector<vu::byte> pe_data_decoded;
    if (vu::crypt_b64decode_W(args[3], pe_data_decoded) && !pe_data_decoded.empty())
    {
      std::string pe_string(pe_data_decoded.cbegin(), pe_data_decoded.cend());
      json pe_jdata = json::parse(pe_string);
      if (pe_jdata.is_object())
      {
        auto pe_path = json_helper::get(pe_jdata, "path", EMPTY);
        auto pe_dir  = json_helper::get(pe_jdata, "dir", EMPTY);
        auto pe_arg  = json_helper::get(pe_jdata, "arg", EMPTY);

        pe_file_path = vu::to_string_W(pe_path);
        pe_file_dir = vu::to_string_W(pe_dir);
        pe_file_arg = vu::to_string_W(pe_arg);
      }
    }
  }

  // parsing mp fields

  if (args[4] == L"-mp")
  {
    std::vector<vu::byte> mp_data_decoded;
    if (vu::crypt_b64decode_W(args[5], mp_data_decoded) && !mp_data_decoded.empty())
    {
      std::string mp_string(mp_data_decoded.cbegin(), mp_data_decoded.cend());
      mp_jdata = json::parse(mp_string);
    }
  }

  return !pe_file_path.empty() && !pe_file_dir.empty() && !mp_jdata.is_null();
}

std::unique_ptr<vu::LNKW> QLoader::parse_shortcut(const std::wstring& file_path)
{
  std::unique_ptr<vu::LNKW> ptr_lnk(nullptr);

  if (vu::ends_with_W(file_path, L".LNK", true))
  {
    ptr_lnk = vu::parse_shortcut_lnk(nullptr, file_path);
  }
  else if (vu::ends_with_W(file_path, L".URL", true))
  {
    std::vector<vu::byte> url_file_content;
    vu::read_file_binary_W(file_path, url_file_content);
    std::string url_file_content_A(url_file_content.cbegin(), url_file_content.cend());
    const auto lines = vu::split_string_A(url_file_content_A, "\n", true);
    const auto url_prefix = "URL=" + vu::to_string_A(m_ph.name(true));
    for (auto& line : lines)
    {
      const auto line_tmp = vu::trim_string_A(line);
      if (vu::starts_with_A(line_tmp, url_prefix, true))
      {
        std::string argument_A(line_tmp.cbegin() + url_prefix.length(), line_tmp.cend());
        argument_A = vu::trim_string_A(argument_A);
        ptr_lnk = std::make_unique<vu::LNKW>();
        ptr_lnk->argument = vu::to_string_W(argument_A);
        break;
      }
    }
  }

  return ptr_lnk;
}

void QLoader::add_a_module(const std::string& name)
{
  assert(0 && "not yet implemented");
}

void QLoader::add_a_patch(
  json& jmodule,
  const std::string& name,
  const std::string& pattern,
  const std::string& replacement,
  const std::string& offset,
  const bool enabled)
{
  assert(0 && "not yet implemented");
}
