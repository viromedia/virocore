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

#ifdef ANDROID_BUILD
#include <asm/unaligned.h>
#endif

// Turn on to add debug messaging (and exit) on overruns
#define kBufferDebugOverruns 1
// Also turn on to abort on overruns
#define kBufferAbortOverruns 1

VROByteBuffer::VROByteBuffer(size_t capacity) :
    pos(0),
    capacity(capacity),
    limit(capacity),
    buffer((char *) malloc(capacity)),
    freeOnDealloc(true) {

}

VROByteBuffer::VROByteBuffer(const void *bytes, size_t length, bool copy) :
    pos(0),
    capacity(length),
    limit(length),
    buffer(nullptr),
    freeOnDealloc(copy) {
        
    if (copy) {
        buffer = (char *) malloc(capacity);
        memcpy(buffer, bytes, capacity);
    }
    else {
        buffer = (char *)bytes;
    }

    passert(length >= 0);
    passert(length == 0 || bytes != nullptr);
}

VROByteBuffer::VROByteBuffer(const std::string &byteString) :
    pos(0),
    capacity(byteString.length()),
    limit(byteString.length()),
    buffer((char *) byteString.c_str()),
    freeOnDealloc(false) {
}

VROByteBuffer::VROByteBuffer(VROByteBuffer *toCopy) :
    pos(0),
    capacity(toCopy->capacity),
    limit(toCopy->capacity),
    buffer((char *) malloc(toCopy->capacity)),
    freeOnDealloc(true) {

    memcpy(this->buffer, toCopy->buffer, toCopy->capacity);
}

VROByteBuffer::VROByteBuffer(VROByteBuffer&& moveFrom) :
    pos(0),
    capacity(moveFrom.capacity),
    limit(moveFrom.capacity),
    buffer(moveFrom.buffer),
    freeOnDealloc(moveFrom.freeOnDealloc) {

    moveFrom.capacity = 0;
    moveFrom.buffer = nullptr;
    moveFrom.freeOnDealloc = false;
}

VROByteBuffer&
VROByteBuffer::operator=(VROByteBuffer&& moveFrom) {
    pos = moveFrom.pos;
    capacity = moveFrom.capacity;
    limit = moveFrom.limit;
    buffer = moveFrom.buffer;
    freeOnDealloc = moveFrom.freeOnDealloc;

    moveFrom.buffer = nullptr;
    moveFrom.capacity = 0;
    moveFrom.freeOnDealloc = false;

    return *this;
}

VROByteBuffer::~VROByteBuffer() {
    if (freeOnDealloc) {
        free(buffer);
    }
}

void VROByteBuffer::setPosition(size_t position) {
    pos = position;

    passert(pos <= capacity); // it's ok to skip to the EOBuffer; though you wouldn't be able to read here.
}

void VROByteBuffer::skip(size_t numBytes) {
    passert(pos + numBytes <= capacity);
    pos += numBytes;
}

bool VROByteBuffer::readBool() {
    passert(pos + 1 <= capacity);

    int byte = readByte();
    return byte == 1;
}

void VROByteBuffer::readStringNullTerm(char *result) {
    char *current = buffer + pos;
    size_t len = strlen(current);

    strcpy(result, current);
    pos += (len + 1);
}

std::string VROByteBuffer::readStringNullTerm() {
    char *current = buffer + pos;
    std::string str(current);
    pos += (str.size() + 1);

    return str;
}

float VROByteBuffer::readFloat() {
    passert(pos + 4 <= capacity);

    float value[1];
    memcpy(value, buffer + pos, 4);
    pos += 4;

    return *value;
}

double VROByteBuffer::readDouble() {
    passert(pos + 8 <= capacity);

    double value[1];
    memcpy(value, buffer + pos, 8);
    pos += 8;

    return *value;
}

int VROByteBuffer::readInt() {
    passert(pos + 4 <= capacity);

    int value;
    memcpy(&value, buffer + pos, 4);

    pos += 4;

    return value;
}

short VROByteBuffer::readShort() {
    passert(pos + 2 <= capacity);

    short value;
    memcpy(&value, buffer + pos, 2);

    pos += 2;

    return value;
}

unsigned short VROByteBuffer::readUnsignedShort() {
    passert(pos + 2 <= capacity);

    unsigned short value;
    memcpy(&value, buffer + pos, 2);

    pos += 2;

    return value;
}

signed char VROByteBuffer::readByte() {
    passert(pos + 1 <= capacity);

    signed char c = buffer[pos];
    ++pos;

    return c;
}

unsigned char VROByteBuffer::readUnsignedByte() {
    passert(pos + 1 <= capacity);

    unsigned char c = buffer[pos];
    ++pos;

    return c;
}

uint64_t VROByteBuffer::readLong() {
    // TODO -- rename this function:  long in C++ is 4 bytes, not the 8 (which is long long) that this assumes.
    passert(pos + 8 <= capacity);

    uint64_t value[1];
    memcpy(value, buffer + pos, 8);
    pos += 8;

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
        s[i] = buffer[pos + i * 2];
    }
    pos += numChars * 2;
    return s;
}

std::string VROByteBuffer::readSTLStringUTF8() {
    short numBytes = readShort();
    if (numBytes == 0) {
        return {};
    }

    std::string s(pointer(), numBytes);
    pos += numBytes;
    return s;
}

std::string VROByteBuffer::readSTLStringUTF8NullTerm() {
    std::string s(pointer());
    pos += s.size() + 1;
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
        characters[i] = (char) *(buffer + pos);
        pos += 2;
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
    pos += numChars;

    std::string s = std::string(characters);
    return s;
}

signed char VROByteBuffer::peekByte() {
    return buffer[pos];
}

int VROByteBuffer::peekInt() {
    char *__p = buffer + pos;
    return (__p[0] | __p[1] << 8 | __p[2] << 16 | __p[3] << 24);
}

char* VROByteBuffer::pointer() {
    return buffer + pos;
}

char* VROByteBuffer::pointerAtPosition(size_t position) {
    return buffer + position;
}

void VROByteBuffer::copyBytes(void *dest, int length) {
    passert(length >= 0);
#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyBytes newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

void VROByteBuffer::copyChars(char *dest, int length) {
    passert(length >= 0);
    length *= sizeof(char);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyChar newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

void VROByteBuffer::copyFloats(float *dest, int length) {
    passert(length >= 0);
    length *= sizeof(float);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyFloats newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

void VROByteBuffer::copyLongs(uint64_t *dest, int length) {
    passert(length >= 0);
    length *= sizeof(uint64_t);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyLongs newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

void VROByteBuffer::copyShorts(short *dest, int length) {
    passert(length >= 0);
    length *= sizeof(short);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyShorts newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

void VROByteBuffer::copyInts(int *dest, int length) {
    passert(length >= 0);
    length *= sizeof(int);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! copyInts newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest, buffer + pos, length);
    pos += length;
}

signed char *VROByteBuffer::readNumChars(int numChars) {
    passert(numChars >= 0);
    signed char *dest = (signed char *) (buffer + pos);
    pos += (numChars * sizeof(signed char));

#if kBufferDebugOverruns
    if (pos > capacity) {
        perr("Overflow! readNumChars() pos=%zu capacity=%zu", pos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
    }
#endif

    return dest;
}

short* VROByteBuffer::readNumShorts(int numShorts) {
    passert(numShorts >= 0);
    short *dest = (short*) (buffer + pos);
    pos += (numShorts * sizeof(short));

#if kBufferDebugOverruns
    if (pos > capacity) {
        perr("Overflow! readNumShorts() pos=%zu capacity=%zu", pos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
    }
#endif

    return dest;
}

void VROByteBuffer::grow(size_t additionalBytesRequired) {
    passert(additionalBytesRequired >= 0);

    size_t requiredCapacity = pos + additionalBytesRequired;
    if (requiredCapacity <= capacity) {
        return;
    }

    size_t oldCapacity = capacity;
    size_t bytesToAdd = std::max(oldCapacity / 2, requiredCapacity - capacity);

    buffer = (char *) realloc(buffer, oldCapacity + bytesToAdd);
    passert(buffer != nullptr);

    if (!freeOnDealloc) {
        perr("Cannot expand a byte-buffer that does not own its underlying data!");
#if kBufferAbortOverruns
        pabort();
#endif
    }

    capacity += bytesToAdd;
    limit = capacity;
}

void VROByteBuffer::shrink(size_t size) {
    passert(size >= 0);
    if (capacity <= size) {
        return;
    }

    size_t __attribute__((__unused__)) bytesSubtracted = capacity - size;
    buffer = (char *) realloc(buffer, size);
    passert (buffer != nullptr);

    capacity = size;
    limit = size;
    pos = 0;
}

void VROByteBuffer::writeBytes(const void *bytes, size_t length) {
    passert(length >= 0);
#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! writeBytes newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, bytes, length);
    pos += length;
}

void VROByteBuffer::writeBuffer(VROByteBuffer *src, size_t length) {
    passert(length >= 0);
#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! writeBuffer newPos=%zu > capacity=%zu",
                newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }

    const size_t srcNewPos = src->pos + length;
    if (srcNewPos > src->capacity) {
        perr("Source overrun! writeBuffer() srcNewPos=%zu > src->capacity=%zu", srcNewPos, src->capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, src->buffer + src->pos, length);
    pos += length;
    src->pos += length;
}

void VROByteBuffer::writeChars(const char *value) {
    int length = (int) strlen(value);

#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! writeChars newPos=%zu > capacity=%zu",
                newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, value, length);
    pos += length;
}

void VROByteBuffer::fill(unsigned char value, size_t numBytes) {
#if kBufferDebugOverruns
    if (pos > capacity) {
        perr("Overrun! fill [value %d, bytes %zu]!", value, numBytes);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memset(buffer + pos, value, numBytes);
    pos += numBytes;
}

void VROByteBuffer::writeStringNullTerm(const char *value) {
    int length = (int) strlen(value);

#if kBufferDebugOverruns
    const size_t newPos = pos + length + 1;
    if (newPos > capacity) {
        perr("Overrun! writeStringNullTerm newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    strcpy(buffer + pos, value);

    pos += (length + 1);
}

void VROByteBuffer::rewind() {
    pos = 0;
}

void VROByteBuffer::clear() {
    memset(buffer, 0x0, capacity);
    pos = 0;
}

void VROByteBuffer::writeBool(bool value) {
    writeByte((char) (value ? 1 : 0));
}

void VROByteBuffer::writeByte(char value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 1;
    if (newPos > capacity) {
        perr("Overrun! writeByte newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    buffer[pos] = value;
    pos++;
}

void VROByteBuffer::writeShort(short value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 2;
    if (newPos > capacity) {
        perr("Overrun! writeShort newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, &value, 2);
    pos += 2;
}

void VROByteBuffer::writeInt(int value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 4;
    if (newPos > capacity) {
        perr("Overrun! writeInt newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, &value, 4);
    pos += 4;
}

void VROByteBuffer::writeFloat(float value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 4;
    if (newPos > capacity) {
        perr("Overrun! writeFloat newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, &value, 4);
    pos += 4;
}

void VROByteBuffer::writeFloats(float *pValues, const int numFloats) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 4 * numFloats;
    if (newPos > capacity) {
        perr("Overrun! writeFloat newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    passert(pValues != nullptr);
    memcpy(buffer + pos, pValues, 4 * numFloats);
    pos += 4 * numFloats;
}

void VROByteBuffer::writeDouble(double value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 8;
    if (newPos > capacity) {
        perr("Overrun! writeDouble newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, (unsigned char *) &value, 8);
    pos += 8;
}

void VROByteBuffer::writeLong(uint64_t value) {
#if kBufferDebugOverruns
    const size_t newPos = pos + 8;
    if (newPos > capacity) {
        perr("Overrun! writeLong newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, &value, 8);
    pos += 8;
}

void VROByteBuffer::flip() {
    limit = pos;
    pos = 0;
}

int VROByteBuffer::remaining() const {
    return (int) (limit - pos);
}

bool VROByteBuffer::hasAvailable() const {
    return pos < limit;
}

void *VROByteBuffer::readPointer() {
    passert(pos + (int) sizeof(void *) <= capacity);

    size_t value;
    memcpy(&value, buffer + pos, sizeof(void *));

    pos += sizeof(void *);

    return (void *) value;
}

void VROByteBuffer::writePointer(void *pointer) {
#if kBufferDebugOverruns
    const size_t newPos = pos + sizeof(void *);
    if (newPos > capacity) {
        perr("Overrun! writePointer newPos=%zu > capacity=%zu", newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(buffer + pos, &pointer, sizeof(pointer));
    pos += sizeof(void *);
}

void VROByteBuffer::writeToBuffer(VROByteBuffer *dest, size_t length) {
    writeToBufferAndRewind(dest, length);
    pos += length;
}

void VROByteBuffer::writeToBufferAndRewind(VROByteBuffer *dest, size_t length) const {
    passert(length >= 0);
#if kBufferDebugOverruns
    const size_t newPos = pos + length;
    if (newPos > capacity) {
        perr("Overrun! writeToBuffer newPos=%zu > capacity=%zu",
                newPos, capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }

    const size_t destNewPos = dest->pos + length;
    if (destNewPos > dest->capacity) {
        perr("Overrun! writeToBuffer() destNewPos=%zu > dest->capacity=%zu", destNewPos, dest->capacity);
#if kBufferAbortOverruns
        pabort();
#endif
        return;
    }
#endif

    memcpy(dest->buffer + dest->pos, buffer + pos, length);
    dest->pos += length;
}

void VROByteBuffer::writeToFile(const char *path) {
    FILE *fd = fopen(path, "wb");
    fwrite(buffer, 1, capacity, fd);
    fclose(fd);
}

void
VROByteBuffer::writeToFile(const std::string &path) {
    writeToFile(path.c_str());
}

void VROByteBuffer::writeToFile(const char *path, size_t offset, size_t length) {
    passert(length >= 0);
    passert_msg(buffer != nullptr, "buffer is null");
    FILE *fd = fopen(path, "wb");
    passert_msg(fd != nullptr, "fd is null, path is %s", path);
    fwrite(buffer + offset, 1, length, fd);
    fclose(fd);
}

void VROByteBuffer::writeToFile(const std::string &path, size_t offset, size_t length) {
    writeToFile(path.c_str(), offset, length);
}

VROByteBuffer *VROByteBuffer::split(size_t offset, size_t length) {
    passert(length >= 0);
    char *copy = (char *) malloc(length);
    memcpy(copy, buffer + offset, length);

    return new VROByteBuffer(copy, length, true);
}

