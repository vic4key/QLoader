#include "pch.h"
#include "QLoader.h"

#include <map>
#include <psapi.h>

#include <winternl.h>
#pragma comment(lib, "ntdll")

#include <dbghelp.h>
#pragma comment(lib, "dbghelp")

static json g_mp_jdata;
static const std::vector<std::wstring> USABLE_FILE_EXTENSIONS = { L".EXE", L".LNK", L".JSON" };

QLoader::QLoader() : m_mp_jdata(g_mp_jdata)
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
  const std::wstring& pe_file_dir,
  const std::wstring& pe_file_name,
  const std::wstring& pe_args)
{
  // create the target process as a suspend process

  vu::PathW pe_file_path = pe_file_dir;
  pe_file_path.join(pe_file_name);
  pe_file_path.finalize();

  PROCESS_INFORMATION pi = { 0 };

  std::wstring args = pe_args;
  if (!args.empty())
  {
    std::wstring tmp = L"";
    tmp  = L'\"';
    tmp += pe_file_path.as_string();
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
    pe_file_path.as_string(), pe_file_dir, args, creation_flags, false, &pi);

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

      std::string v2 = json_get(jmodule, "name", EMPTY);
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
      const auto module_name = vu::to_string_W(json_get(jmodule, "name", EMPTY));
      if (module_name.empty())
      {
        continue;
      }

      // ignore the disable module

      bool enabled = json_get(jmodule, "enabled", true);
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

        auto name = json_get(jpatch, "name", UNNAMED);
        std::wstring patch_name = vu::to_string_W(name);
        line = vu::format(L"Try to patch `%s`", patch_name.c_str());
        this->add_log(line);

        // extract the pattern bytes of patch

        const auto pattern = json_get(jpatch, "pattern", EMPTY);

        // extract the replacement bytes of patch

        const auto replacement = json_get(jpatch, "replacement", EMPTY);
        auto l = vu::split_string_A(replacement, " ");
        std::vector<vu::byte> replacement_bytes;
        for (auto& e : l)
        {
          auto v = vu::byte(std::stoi(e, nullptr, 16));
          replacement_bytes.push_back(v);
        }

        // extract the offset of patch

        const auto offset = json_get(jpatch, "offset", 0);

        // extract the enabled of patch

        const bool enabled = json_get(jpatch, "enabled", true);
        if (!enabled)
        {
          auto line = vu::format(L"Ignore the patch `%s`", patch_name.c_str());
          this->add_log(line, status_t::warn);
          continue;
        }

        // search pattern in the module

        auto address = static_cast<const void*>(it->second.m_buffer.get());
        auto size = it->second.m_me.modBaseSize;

        auto result = vu::find_pattern_A(address, size, pattern);
        if (!result.first)
        {
          auto line = vu::format(L"Find the patch `%s` not found", patch_name.c_str());
          this->add_log(line, status_t::error);
          continue;
        }

        line = vu::format(L"Find the patch `%s` found", patch_name.c_str());
        this->add_log(line, status_t::success);

        // patch at the found address with the replacement bytes

        auto found_patch_address = vu::ulongptr(it->second.m_me.modBaseAddr);
        found_patch_address += result.second;
        found_patch_address += offset;

        bool ret = process.write_memory(
          found_patch_address, replacement_bytes.data(), replacement_bytes.size());

        line = vu::format(
          process.bit() == vu::eXBit::x64 ?
            L"Patch `%s` at %016X %s" : L"Patch `%s` at %08X %s",
          patch_name.c_str(),
          process.bit() == vu::eXBit::x64 ?
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
  vu::ProcessW& process, PROCESS_INFORMATION& pi, std::vector<byte>& ep)
{
  ep.clear();

  // get base address of the target process

  DWORD size = 0;
  PROCESS_BASIC_INFORMATION pbi = { 0 };
  NTSTATUS status = NtQueryInformationProcess(
    pi.hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &size);
  assert(NT_SUCCESS(status));

  vu::ulongptr base_address = 0;

  if (process.bit() == vu::eXBit::x64)
  {
    PEB_T<vu::pe64> peb;
    process.read_memory(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = peb.ImageBaseAddress;
  }
  else // x86
  {
    PEB_T<vu::pe32> peb;
    process.read_memory(vu::ulongptr(pbi.PebBaseAddress), &peb, sizeof(peb));
    base_address = peb.ImageBaseAddress;
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

  std::vector<byte> bp = { 0xEB, 0xFE };
  ep.resize(bp.size());
  process.read_memory(va_oep, ep.data(), ep.size());
  process.write_memory(va_oep, bp.data(), bp.size());

  ResumeThread(pi.hThread);

  WaitForInputIdle(pi.hProcess, 1000); // 1s

  auto fmt = process.bit() == vu::eXBit::x64 ?
    L"Break the process at its entry point %016X succeed" : L"Break the process at its entry point %08X succeed";
  auto line = vu::format(fmt, process.bit() == vu::eXBit::x64 ? vu::ulong64(va_oep) : vu::ulong32(va_oep));
  this->add_log(line, status_t::success);

  return va_oep;
}
