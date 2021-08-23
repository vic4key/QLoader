#pragma once

namespace lnk
{

class MemoryBuffer
{
public:
  MemoryBuffer(void);
  MemoryBuffer(unsigned long iSize);
  MemoryBuffer(const MemoryBuffer & iValue);
  virtual ~MemoryBuffer(void);

  //----------------
  // public methods
  //----------------
  void clear();
  unsigned char * getBuffer();
  const unsigned char * getBuffer() const;
  bool allocate(unsigned long iSize);
  bool reallocate(unsigned long iSize);
  unsigned long getSize() const;
  bool loadFile(const char * iFilePath);

  const MemoryBuffer & operator = (const MemoryBuffer & iValue);

private:
  unsigned char* mBuffer;
  unsigned long mSize;
};

template <typename T>
inline bool serialize(const T & iValue, MemoryBuffer & ioBuffer)
{
  unsigned long dataSize = sizeof(T);
  unsigned long oldSize = ioBuffer.getSize();
  unsigned long newSize = oldSize + dataSize;
  if (ioBuffer.reallocate(newSize))
  {
    //save data
    unsigned char * buffer = ioBuffer.getBuffer();
    T * offset = (T*)&buffer[oldSize];
    *offset = iValue;

    return true;
  }
  return false;
}

inline bool serialize(const unsigned char * iData, const unsigned long & iSize, MemoryBuffer & ioBuffer)
{
  unsigned long oldSize = ioBuffer.getSize();
  unsigned long newSize = oldSize + iSize;
  if (ioBuffer.reallocate(newSize))
  {
    //save data
    unsigned char * buffer = ioBuffer.getBuffer();
    unsigned char * offset = &buffer[oldSize];
    for(unsigned long i=0; i<iSize; i++)
    {
      offset[i] = iData[i];
    }

    return true;
  }
  return false;
}

}; //lnk
