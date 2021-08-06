#pragma once

#include "types.h"
#include "EasyTreeCtrl.h"

struct JNode : public Node
{
  JNode(const std::string& name, void* ptr_data = nullptr, const std::string& module = "")
    : Node(name, ptr_data), m_module(module) {}

  std::string m_module;
};

namespace utils
{

template <typename Ptr>
json* to_ptr_json(Ptr ptr)
{
  auto ptr_data = reinterpret_cast<void*>(ptr);
  auto ptr_node = static_cast<JNode*>(ptr_data);
  if (ptr_node == nullptr)
  {
    return nullptr;
  }

  return static_cast<json*>(ptr_node->m_ptr_data);
}

}; // utils