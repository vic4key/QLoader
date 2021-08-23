#pragma once

#include <string>
#include <vector>
#include <stdint.h>

namespace lnk
{

enum LNK_HOTKEY_CODES
{
  LNK_HK_NONE = 0x00,
  LNK_HK_0 = 0x30,
  LNK_HK_1 = 0x31,
  LNK_HK_2 = 0x32,
  LNK_HK_3 = 0x33,
  LNK_HK_4 = 0x34,
  LNK_HK_5 = 0x35,
  LNK_HK_6 = 0x36,
  LNK_HK_7 = 0x37,
  LNK_HK_8 = 0x38,
  LNK_HK_9 = 0x39,
  LNK_HK_A = 0x41,
  LNK_HK_B = 0x42,
  LNK_HK_C = 0x43,
  LNK_HK_D = 0x44,
  LNK_HK_E = 0x45,
  LNK_HK_F = 0x46,
  LNK_HK_G = 0x47,
  LNK_HK_H = 0x48,
  LNK_HK_I = 0x49,
  LNK_HK_J = 0x4a,
  LNK_HK_K = 0x4b,
  LNK_HK_L = 0x4c,
  LNK_HK_M = 0x4d,
  LNK_HK_N = 0x4e,
  LNK_HK_O = 0x4f,
  LNK_HK_P = 0x50,
  LNK_HK_Q = 0x51,
  LNK_HK_R = 0x52,
  LNK_HK_S = 0x53,
  LNK_HK_T = 0x54,
  LNK_HK_U = 0x55,
  LNK_HK_V = 0x56,
  LNK_HK_W = 0x57,
  LNK_HK_X = 0x58,
  LNK_HK_Y = 0x59,
  LNK_HK_Z = 0x5a,
  LNK_HK_F1 = 0x70,
  LNK_HK_F2 = 0x71,
  LNK_HK_F3 = 0x72,
  LNK_HK_F4 = 0x73,
  LNK_HK_F5 = 0x74,
  LNK_HK_F6 = 0x75,
  LNK_HK_F7 = 0x76,
  LNK_HK_F8 = 0x77,
  LNK_HK_F9 = 0x78,
  LNK_HK_F10 = 0x79,
  LNK_HK_F11 = 0x7a,
  LNK_HK_F12 = 0x7b,
  LNK_HK_NUMLOCK = 0x90,
  LNK_HK_SCROLL = 0x91,
};
enum LNK_HOTKEY_MODIFIERS
{
  LNK_HK_MOD_NONE     = 0x00,
  LNK_HK_MOD_SHIFT    = 0x01,
  LNK_HK_MOD_CONTROL  = 0x02,
  LNK_HK_MOD_ALT      = 0x04,
};
struct LNK_HOTKEY
{
  uint8_t keyCode;    //LNK_HOTKEY_CODES
  uint8_t modifiers;  //LNK_HOTKEY_MODIFIERS
};
extern const LNK_HOTKEY LNK_NO_HOTKEY;

struct LNK_ICON
{
  std::string filename;
  unsigned long index;
};

struct LinkInfo
{
  std::string target;
  std::string networkPath;
  std::string arguments;
  std::string description;
  std::string workingDirectory;
  LNK_ICON customIcon;
  LNK_HOTKEY hotKey;
};

const char * getVersionString();
bool isLink(const char * iFilePath);
bool getLinkInfo(const char * iFilePath, LinkInfo & oLinkInfo);
bool createLink(const char * iFilePath, const LinkInfo & iLinkInfo);
bool printLinkInfo(const char * iFilePath);
std::string getLinkCommand(const char * iFilePath);

}; //lnk
