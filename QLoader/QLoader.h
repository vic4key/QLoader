#pragma once

#include <vector>
#include <string>
#include <vu>

#include <3rd-protocol-handler/protocol-handler.h>

#include "types.h"

class QLoader
{
public:
  QLoader();
  virtual ~QLoader();

  enum class status_t
  {
    none,
    success,
    warn,
    error,
  };

  enum class launch_t
  {
    fully_loaded,
    at_entry_point,
    unpacking,
  };

  const std::wstring& NAME = L"QLoader";
  const std::vector<std::wstring>& USABLE_FILE_EXTENSIONS = { L".EXE", L".JSON", L".LNK", L".URL" };

  virtual bool file_supported(const std::wstring& file_path);

  virtual void add_log(const std::wstring& line, const status_t status = status_t::none);

  virtual void launch(
    const launch_t mode,
    const std::wstring& pe_file_path,
    const std::wstring& pe_file_dir,
    const std::wstring& pe_file_arg);

  virtual vu::LNKW export_as_lnk(
    const launch_t mode,
    const std::wstring& pe_file_path,
    const std::wstring& pe_file_dir,
    const std::wstring& pe_file_arg);

  bool parse_cmd_line(
    const std::wstring& cmd_line,
    int& patch_when,
    std::wstring& pe_file_path,
    std::wstring& pe_file_dir,
    std::wstring& pe_file_arg,
    json& mp_jdata);

  std::unique_ptr<vu::LNKW> parse_shortcut(const std::wstring& file_path);

private:
  vu::ulongptr launch_with_patch_at_oep(
    vu::ProcessW& process, PROCESS_INFORMATION& process_information, std::vector<byte>& ep);

protected:
  json& m_mp_jdata;
  ProtocolHandler m_ph;
};
