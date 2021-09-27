#pragma once

#include <vector>
#include <string>
#include <vu>

#include "types.h"

class QLoader
{
public:
  QLoader();
  virtual ~QLoader();

  enum class status_t : int
  {
    none,
    success,
    warn,
    error,
  };

  enum class launch_t : int
  {
    fully_loaded,
    at_entry_point,
    unpacking,
  };

  virtual bool file_supported(const std::wstring& file_path);

  virtual void add_log(const std::wstring& line, const status_t status = status_t::none);

  virtual void launch(
    const launch_t mode,
    const std::wstring& pe_file_path,
    const std::wstring& pe_file_dir,
    const std::wstring& pe_file_arg);

  virtual vu::sLNKW export_as_lnk(
    const launch_t mode,
    const std::wstring& pe_file_path,
    const std::wstring& pe_file_dir,
    const std::wstring& pe_file_arg);

  bool parse_app_args(
    int argc,
    wchar_t** argv,
    int& patch_when,
    std::wstring& pe_file_path,
    std::wstring& pe_file_dir,
    std::wstring& pe_file_arg,
    json& mp_jdata);

private:
  vu::ulongptr launch_with_patch_at_oep(
    vu::ProcessW& process, PROCESS_INFORMATION& pi, std::vector<byte>& ep);

protected:
  json& m_mp_jdata;
};
