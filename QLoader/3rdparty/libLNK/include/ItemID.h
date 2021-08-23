#pragma once

#include "MemoryBuffer.h"
#include <string>
#include <vector>
#include <stdint.h>

namespace lnk
{

#pragma pack(push)
#pragma pack(1)

  struct ItemIDHeader
  {
    uint16_t size;
    uint8_t type;
    uint8_t unknown1[9];
    uint16_t fileAttributes;
  };

  struct ItemIDEx
  {
    uint16_t size;
    uint32_t type;
  };

#pragma pack(pop)

  enum FILE_ATTRIBUTES
  {
    FA_NORMAL,
    FA_DIRECTORY
  };

  typedef std::vector<MemoryBuffer> ItemIDList;

  MemoryBuffer getLinkTargetIDList(const ItemIDList & iItemIDList);
  MemoryBuffer getTerminalItemId();
  MemoryBuffer getComputerItemId();
  MemoryBuffer getDriveItemId(char iDriveLetter);
  MemoryBuffer getFileItemId(const std::string & iShortName, const std::string & iLongName, const FILE_ATTRIBUTES & iAttributes);
  MemoryBuffer getWinXpItemIdEx(const std::string & iLongName, const FILE_ATTRIBUTES & iAttributes);

}; //lnk
