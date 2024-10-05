#pragma once

#include <fstream>
#include <3rd-json-fifo-nlohmann/json.hpp>

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

static const std::string EMPTY = "";
static const std::string UNNAMED = "<unnamed>";
