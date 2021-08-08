#pragma once

#include "types.h"
#include "EasyTreeCtrl.h"

template <class T>
struct PEB_T
{
  union
  {
    struct
    {
      BYTE InheritedAddressSpace;
      BYTE ReadImageFileExecOptions;
      BYTE BeingDebugged;
      BYTE BitField;
    };
    T dummy01;
  };
  T Mutant;
  T ImageBaseAddress;
};

struct jnode_t : public Node
{
  jnode_t(const std::string& name, void* ptr_data = nullptr, const std::string& module = "")
    : Node(name, ptr_data), m_module(module) {}

  std::string m_module;
};

namespace utils
{

template <typename ptr_t>
json* to_ptr_json(ptr_t ptr)
{
  auto ptr_data = reinterpret_cast<void*>(ptr);
  auto ptr_node = static_cast<jnode_t*>(ptr_data);
  if (ptr_node == nullptr)
  {
    return nullptr;
  }

  return static_cast<json*>(ptr_node->m_ptr_data);
}

template <typename value_t>
value_t json_get(json& jobject, const std::string& name, const value_t def)
{
  return jobject.contains(name) ? jobject[name].get<value_t>() : def;
}

void read_file(const std::wstring& file_path, std::vector<byte>& data)
{
  FILE* file = nullptr;
  _wfopen_s(&file, file_path.c_str(), L"rb");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  data.resize(file_size);

  fread(&data[0], 1, data.size(), file);

  fclose(file);
}

}; // utils