#ifndef CONSTS_H
#define CONSTS_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define CHUNK_SIZE_X 1000
#define CHUNK_SIZE_Y 1000
#define RENDER_DISTANCE 1

class BufferReader {
  public:
    virtual unsigned char getByte() {};
    virtual short getShort() {};
    virtual int getInt() {};
    virtual void setPlace(int) {};
    virtual int getPlace() {};
    virtual int getSize() {};
    virtual void read(char*, int) {};
};

#endif
