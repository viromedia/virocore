//
//  VROByteBuffer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/21/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include <algorithm>

#include "VROByteBuffer.h"
#include "VROLog.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "VROMath.h"

#ifdef ANDROID_BUILD
#include <asm/unaligned.h>
#endif

// Turn on to add debug messaging (and exit) on overruns
#define k_bufferDebugOverruns 1
// Also turn on to abort on overruns
#define k_bufferAbortOverruns 1

VROByteBuffer::VROByteBuffer(size_t _capacity) :
    _pos(0),
    _capacity(_capacity),
    _buffer((char *) malloc(_capacity)),
    _freeOnDealloc(true) {

}

VROByteBuffer::VROByteBuffer(const void *bytes, size_t length, bool copy) :
    _pos(0),
    _capacity(length),
    _buffer(nullptr),
    _freeOnDealloc(copy) {
        
    if (copy) {
        _buffer = (char *) malloc(_capacity);
        memcpy(_buffer, bytes, _capacity);
    }
    else {
        _buffer = (char *)bytes;
    }

    passert(length >= 0);
    passert(length == 0 || bytes != nullptr);
}

VROByteBuffer::VROByteBuffer(const std::string &byteString) :
    _pos(0),
    _capacity(byteString.length()),
    _buffer((char *) byteString.c_str()),
    _freeOnDealloc(false) {
}

VROByteBuffer::VROByteBuffer(VROByteBuffer *toCopy) :
    _pos(0),
    _capacity(toCopy->_capacity),
    _buffer((char *) malloc(toCopy->_capacity)),
    _freeOnDealloc(true) {

    memcpy(this->_buffer, toCopy->_buffer, toCopy->_capacity);
}

VROByteBuffer::VROByteBuffer(VROByteBuffer&& moveFrom) :
    _pos(0),
    _capacity(moveFrom._capacity),
    _buffer(moveFrom._buffer),
    _freeOnDealloc(moveFrom._freeOnDealloc) {

    moveFrom._capacity = 0;
    moveFrom._buffer = nullptr;
    moveFrom._freeOnDealloc = false;
}

VROByteBuffer&
VROByteBuffer::operator=(VROByteBuffer&& moveFrom) {
    _pos = moveFrom._pos;
    _capacity = moveFrom._capacity;
    _buffer = moveFrom._buffer;
    _freeOnDealloc = moveFrom._freeOnDealloc;

    moveFrom._buffer = nullptr;
    moveFrom._capacity = 0;
    moveFrom._freeOnDealloc = false;

    return *this;
}

VROByteBuffer::~VROByteBuffer() {
    if (_freeOnDealloc) {
        free(_buffer);
    }
}

void VROByteBuffer::setPosition(size_t _position) {
    _pos = _position;

    passert(_pos <= _capacity); // it's ok to skip to the EO_buffer; though you wouldn't be able to read here.
}

void VROByteBuffer::skip(size_t numBytes) {
    passert(_pos + numBytes <= _capacity);
    _pos += numBytes;
}

bool VROByteBuffer::readBool() {
    passert(_pos + 1 <= _capacity);

    int byte = readByte();
    return byte == 1;
}

void VROByteBuffer::readStringNullTerm(char *result) {
    char *current = _buffer + _pos;
    size_t len = strlen(current);

    strcpy(result, current);
    _pos += (len + 1);
}

std::string VROByteBuffer::readStringNullTerm() {
    char *current = _buffer + _pos;
    std::string str(current);
    _pos += (str.size() + 1);

    return str;
}

float VROByteBuffer::readHalf() {
    return VROFloat16ToFloat(readShort());
}

float VROByteBuffer::readFloat() {
    passert(_pos + 4 <= _capacity);

    float value[1];
    memcpy(value, _buffer + _pos, 4);
    _pos += 4;

    return *value;
}

double VROByteBuffer::readDouble() {
    passert(_pos + 8 <= _capacity);

    double value[1];
    memcpy(value, _buffer + _pos, 8);
    _pos += 8;

    return *value;
}

int VROByteBuffer::readInt() {
    passert(_pos + 4 <= _capacity);

    int value;
    memcpy(&value, _buffer + _pos, 4);

    _pos += 4;

    return value;
}

short VROByteBuffer::readShort() {
    passert(_pos + 2 <= _capacity);

    short value;
    memcpy(&value, _buffer + _pos, 2);

    _pos += 2;

    return value;
}

unsigned short VROByteBuffer::readUnsignedShort() {
    passert(_pos + 2 <= _capacity);

    unsigned short value;
    memcpy(&value, _buffer + _pos, 2);

    _pos += 2;

    return value;
}

signed char VROByteBuffer::readByte() {
    passert(_pos + 1 <= _capacity);

    signed char c = _buffer[_pos];
    ++_pos;

    return c;
}

unsigned char VROByteBuffer::readUnsignedByte() {
    passert(_pos + 1 <= _capacity);

    unsigned char c = _buffer[_pos];
    ++_pos;

    return c;
}

uint64_t VROByteBuffer::readLong() {
    // TODO -- rename this function:  long in C++ is 4 bytes, not the 8 (which is long long) that this assumes.
    passert(_pos + 8 <= _capacity);

    uint64_t value[1];
    memcpy(value, _buffer + _pos, 8);
    _pos += 8;

    return *value;
}

std::string VROByteBuffer::readSTLString() {
    short numChars = readShort();
    passert(numChars >= 0);
    if (numChars == 0) {
        return {};
    }

    std::string s(numChars, '\0');
    for (int i = 0; i < numChars; i++) {
        s[i] = _buffer[_pos + i * 2];
    }
    _pos += numChars * 2;
    return s;
}

std::string VROByteBuffer::readSTLStringUTF8() {
    short numBytes = readShort();
    if (numBytes == 0) {
        return {};
    }

    std::string s(pointer(), numBytes);
    _pos += numBytes;
    return s;
}

std::string VROByteBuffer::readSTLStringUTF8NullTerm() {
    std::string s(pointer());
    _pos += s.size() + 1;
    return s;
}

std::string VROByteBuffer::readSTLText() {
    int numChars = readInt();
    passert(numChars >= 0);
    if (numChars == 0) {
        std::string empty;
        return empty;
    }

    char characters[numChars + 1];
    for (int i = 0; i < numChars; i++) {
        characters[i] = (char) *(_buffer + _pos);
        _pos += 2;
    }
    characters[numChars] = '\0';

    std::string s = std::string(characters);
    return s;
}

std::string VROByteBuffer::readSTLTextUTF8() {
    int numChars = readInt();
    passert(numChars >= 0);
    if (numChars == 0) {
        std::string empty;
        return empty;
    }

    char characters[numChars + 1];
    memcpy(characters, pointer(), numChars);
    characters[numChars] = '\0';
    _pos += numChars;

    std::string s = std::string(characters);
    return s;
}

signed char VROByteBuffer::peekByte() {
    return _buffer[_pos];
}

int VROByteBuffer::peekInt() {
    char *__p = _buffer + _pos;
    return (__p[0] | __p[1] << 8 | __p[2] << 16 | __p[3] << 24);
}

char* VROByteBuffer::pointer() {
    return _buffer + _pos;
}

char* VROByteBuffer::pointerAtPosition(size_t _position) {
    return _buffer + _position;
}

void VROByteBuffer::copyBytes(void *dest, int length) {
    passert(length >= 0);
#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyBytes newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

void VROByteBuffer::copyChars(char *dest, int length) {
    passert(length >= 0);
    length *= sizeof(char);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyChar newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

void VROByteBuffer::copyFloats(float *dest, int length) {
    passert(length >= 0);
    length *= sizeof(float);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyFloats newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

void VROByteBuffer::copyLongs(uint64_t *dest, int length) {
    passert(length >= 0);
    length *= sizeof(uint64_t);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyLongs newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

void VROByteBuffer::copyShorts(short *dest, int length) {
    passert(length >= 0);
    length *= sizeof(short);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyShorts newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

void VROByteBuffer::copyInts(int *dest, int length) {
    passert(length >= 0);
    length *= sizeof(int);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! copyInts newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, _buffer + _pos, length);
    _pos += length;
}

signed char *VROByteBuffer::readNumChars(int numChars) {
    passert(numChars >= 0);
    signed char *dest = (signed char *) (_buffer + _pos);
    _pos += (numChars * sizeof(signed char));

#if k_bufferDebugOverruns
    if (_pos > _capacity) {
        perr("Overflow! readNumChars() _pos=%zu _capacity=%zu", _pos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
    }
#endif

    return dest;
}

short* VROByteBuffer::readNumShorts(int numShorts) {
    passert(numShorts >= 0);
    short *dest = (short*) (_buffer + _pos);
    _pos += (numShorts * sizeof(short));

#if k_bufferDebugOverruns
    if (_pos > _capacity) {
        perr("Overflow! readNumShorts() _pos=%zu _capacity=%zu", _pos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
    }
#endif

    return dest;
}

void VROByteBuffer::grow(size_t additionalBytesRequired) {
    passert(additionalBytesRequired >= 0);

    size_t required_capacity = _pos + additionalBytesRequired;
    if (required_capacity <= _capacity) {
        return;
    }

    size_t old_capacity = _capacity;
    size_t bytesToAdd = std::max(old_capacity / 2, required_capacity - _capacity);

    _buffer = (char *) realloc(_buffer, old_capacity + bytesToAdd);
    passert(_buffer != nullptr);

    if (!_freeOnDealloc) {
        perr("Cannot expand a byte-_buffer that does not own its underlying data!");
#if k_bufferAbortOverruns
        pabort();
#endif
    }

    _capacity += bytesToAdd;
}

void VROByteBuffer::shrink(size_t size) {
    passert(size >= 0);
    if (_capacity <= size) {
        return;
    }

    size_t __attribute__((__unused__)) bytesSubtracted = _capacity - size;
    _buffer = (char *) realloc(_buffer, size);
    passert (_buffer != nullptr);

    _capacity = size;
    _pos = 0;
}

void VROByteBuffer::writeBytes(const void *bytes, size_t length) {
    passert(length >= 0);
#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! writeBytes newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, bytes, length);
    _pos += length;
}

void VROByteBuffer::writeBuffer(VROByteBuffer *src, size_t length) {
    passert(length >= 0);
#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! write_buffer newPos=%zu > _capacity=%zu",
                newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }

    const size_t srcNewPos = src->_pos + length;
    if (srcNewPos > src->_capacity) {
        perr("Source overrun! write_buffer() srcNewPos=%zu > src->_capacity=%zu", srcNewPos, src->_capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, src->_buffer + src->_pos, length);
    _pos += length;
    src->_pos += length;
}

void VROByteBuffer::writeChars(const char *value) {
    int length = (int) strlen(value);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! writeChars newPos=%zu > _capacity=%zu",
                newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, value, length);
    _pos += length;
}

void VROByteBuffer::fill(unsigned char value, size_t numBytes) {
#if k_bufferDebugOverruns
    if (_pos > _capacity) {
        perr("Overrun! fill [value %d, bytes %zu]!", value, numBytes);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memset(_buffer + _pos, value, numBytes);
    _pos += numBytes;
}

void VROByteBuffer::writeStringNullTerm(const char *value) {
    int length = (int) strlen(value);

#if k_bufferDebugOverruns
    const size_t newPos = _pos + length + 1;
    if (newPos > _capacity) {
        perr("Overrun! writeStringNullTerm newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    strcpy(_buffer + _pos, value);

    _pos += (length + 1);
}

void VROByteBuffer::rewind() {
    _pos = 0;
}

void VROByteBuffer::clear() {
    memset(_buffer, 0x0, _capacity);
    _pos = 0;
}

void VROByteBuffer::writeBool(bool value) {
    writeByte((char) (value ? 1 : 0));
}

void VROByteBuffer::writeByte(char value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 1;
    if (newPos > _capacity) {
        perr("Overrun! writeByte newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    _buffer[_pos] = value;
    _pos++;
}

void VROByteBuffer::writeShort(short value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 2;
    if (newPos > _capacity) {
        perr("Overrun! writeShort newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, &value, 2);
    _pos += 2;
}

void VROByteBuffer::writeInt(int value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 4;
    if (newPos > _capacity) {
        perr("Overrun! writeInt newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, &value, 4);
    _pos += 4;
}

void VROByteBuffer::writeHalf(float value) {
    writeShort(VROFloatToFloat16(value));
}

void VROByteBuffer::writeFloat(float value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 4;
    if (newPos > _capacity) {
        perr("Overrun! writeFloat newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, &value, 4);
    _pos += 4;
}

void VROByteBuffer::writeFloats(float *pValues, const int numFloats) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 4 * numFloats;
    if (newPos > _capacity) {
        perr("Overrun! writeFloat newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    passert(pValues != nullptr);
    memcpy(_buffer + _pos, pValues, 4 * numFloats);
    _pos += 4 * numFloats;
}

void VROByteBuffer::writeDouble(double value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 8;
    if (newPos > _capacity) {
        perr("Overrun! writeDouble newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, (unsigned char *) &value, 8);
    _pos += 8;
}

void VROByteBuffer::writeLong(uint64_t value) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + 8;
    if (newPos > _capacity) {
        perr("Overrun! writeLong newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, &value, 8);
    _pos += 8;
}

void *VROByteBuffer::readPointer() {
    passert(_pos + (int) sizeof(void *) <= _capacity);

    size_t value;
    memcpy(&value, _buffer + _pos, sizeof(void *));

    _pos += sizeof(void *);

    return (void *) value;
}

void VROByteBuffer::writePointer(void *pointer) {
#if k_bufferDebugOverruns
    const size_t newPos = _pos + sizeof(void *);
    if (newPos > _capacity) {
        perr("Overrun! writePointer newPos=%zu > _capacity=%zu", newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(_buffer + _pos, &pointer, sizeof(pointer));
    _pos += sizeof(void *);
}

void VROByteBuffer::writeToBuffer(VROByteBuffer *dest, size_t length) {
    writeToBufferAndRewind(dest, length);
    _pos += length;
}

void VROByteBuffer::writeToBufferAndRewind(VROByteBuffer *dest, size_t length) const {
    passert(length >= 0);
#if k_bufferDebugOverruns
    const size_t newPos = _pos + length;
    if (newPos > _capacity) {
        perr("Overrun! writeTo_buffer newPos=%zu > _capacity=%zu",
                newPos, _capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }

    const size_t destNewPos = dest->_pos + length;
    if (destNewPos > dest->_capacity) {
        perr("Overrun! writeTo_buffer() destNewPos=%zu > dest->_capacity=%zu", destNewPos, dest->_capacity);
#if k_bufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest->_buffer + dest->_pos, _buffer + _pos, length);
    dest->_pos += length;
}

void VROByteBuffer::writeToFile(const char *path) {
    FILE *fd = fopen(path, "wb");
    fwrite(_buffer, 1, _capacity, fd);
    fclose(fd);
}

void
VROByteBuffer::writeToFile(const std::string &path) {
    writeToFile(path.c_str());
}

void VROByteBuffer::writeToFile(const char *path, size_t offset, size_t length) {
    passert(length >= 0);
    passert_msg(_buffer != nullptr, "_buffer is null");
    FILE *fd = fopen(path, "wb");
    passert_msg(fd != nullptr, "fd is null, path is %s", path);
    fwrite(_buffer + offset, 1, length, fd);
    fclose(fd);
}

void VROByteBuffer::writeToFile(const std::string &path, size_t offset, size_t length) {
    writeToFile(path.c_str(), offset, length);
}

VROByteBuffer *VROByteBuffer::split(size_t offset, size_t length) {
    passert(length >= 0);
    char *copy = (char *) malloc(length);
    memcpy(copy, _buffer + offset, length);

    return new VROByteBuffer(copy, length, true);
}

