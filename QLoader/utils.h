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
  jnode_t(const std::string& name, void* ptr = nullptr, json* ptr_parent = nullptr)
    : Node(name, ptr), m_ptr_parent(ptr_parent) {}
  json* m_ptr_parent;
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
value_t json_get(const json& jobject, const std::string& name, const value_t def)
{
  return jobject.contains(name) ? jobject[name].get<value_t>() : def;
}

}; // utils