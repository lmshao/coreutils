/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_CORE_UTILS_DATA_BUFFER_H
#define LMSHAO_CORE_UTILS_DATA_BUFFER_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

namespace lmshao::coreutils {
/**
 * @brief A dynamic data buffer for binary data.
 */
class DataBuffer {
public:
    /**
     * @brief Constructs a DataBuffer with an initial capacity.
     * @param len The initial capacity of the buffer.
     */
    explicit DataBuffer(size_t len = 0);
    /**
     * @brief Copy constructor.
     * @param other The DataBuffer to copy from.
     */
    DataBuffer(const DataBuffer &other) noexcept;
    /**
     * @brief Copy assignment operator.
     * @param other The DataBuffer to copy from.
     * @return A reference to this DataBuffer.
     */
    DataBuffer &operator=(const DataBuffer &other) noexcept;
    /**
     * @brief Move constructor.
     * @param other The DataBuffer to move from.
     */
    DataBuffer(DataBuffer &&other) noexcept;
    /**
     * @brief Move assignment operator.
     * @param other The DataBuffer to move from.
     * @return A reference to this DataBuffer.
     */
    DataBuffer &operator=(DataBuffer &&other) noexcept;

    /**
     * @brief Destructor.
     */
    virtual ~DataBuffer();

    /**
     * @brief Creates a new DataBuffer as a shared pointer.
     * @param len The initial capacity of the buffer.
     * @return A shared pointer to the new DataBuffer.
     */
    static std::shared_ptr<DataBuffer> Create(size_t len = 0);
    /**
     * @brief Allocates a DataBuffer from a memory pool.
     * @param len The requested size of the buffer.
     * @return A shared pointer to the allocated DataBuffer.
     */
    static std::shared_ptr<DataBuffer> PoolAlloc(size_t len = 4096);
    /**
     * @brief Frees a DataBuffer back to the memory pool.
     * @param buf Pointer to the DataBuffer to free.
     */
    static void PoolFree(DataBuffer *buf);

    /**
     * @brief Assigns a null pointer (no-op).
     */
    void Assign(std::nullptr_t) {}
    /**
     * @brief Assigns data from a raw pointer.
     * @param p Pointer to the data.
     * @param len Length of the data.
     */
    void Assign(const void *p, size_t len);
    /**
     * @brief Assigns a single byte.
     * @param c The byte to assign.
     */
    void Assign(uint8_t c) { Assign((uint8_t *)&c, 1); }
    /**
     * @brief Assigns a 16-bit unsigned integer.
     * @param u16 The integer to assign.
     */
    void Assign(uint16_t u16);
    /**
     * @brief Assigns a 32-bit unsigned integer.
     * @param u32 The integer to assign.
     */
    void Assign(uint32_t u32);
    /**
     * @brief Assigns a C-style string.
     * @param str The string to assign.
     */
    void Assign(const char *str) { Assign(str, strlen(str)); }
    /**
     * @brief Assigns a std::string.
     * @param s The string to assign.
     */
    void Assign(const std::string &s) { Assign(s.c_str(), s.size()); }

    /**
     * @brief Appends a null pointer (no-op).
     */
    void Append(std::nullptr_t) {}
    /**
     * @brief Appends data from a raw pointer.
     * @param p Pointer to the data.
     * @param len Length of the data.
     */
    void Append(const void *p, size_t len);
    /**
     * @brief Appends a single byte.
     * @param c The byte to append.
     */
    void Append(uint8_t c) { Append((uint8_t *)&c, 1); }
    /**
     * @brief Appends a 16-bit unsigned integer.
     * @param u16 The integer to append.
     */
    void Append(uint16_t u16);
    /**
     * @brief Appends a 32-bit unsigned integer.
     * @param u32 The integer to append.
     */
    void Append(uint32_t u32);
    /**
     * @brief Appends a C-style string.
     * @param str The string to append.
     */
    void Append(const char *str) { Append(str, strlen(str)); }
    /**
     * @brief Appends a std::string.
     * @param s The string to append.
     */
    void Append(const std::string &s) { Append(s.c_str(), s.size()); }
    /**
     * @brief Appends another DataBuffer.
     * @param b A shared pointer to the DataBuffer to append.
     */
    void Append(std::shared_ptr<DataBuffer> b) { Append(b->Data(), b->Size()); }

    /**
     * @brief Gets a pointer to the buffer's data.
     * @return A pointer to the data.
     */
    uint8_t *Data() { return data_; }
    /**
     * @brief Gets a const pointer to the buffer's data.
     * @return A const pointer to the data.
     */
    const uint8_t *Data() const { return data_; }

    /**
     * @brief Accesses a byte at a specific index.
     * @param index The index of the byte.
     * @return A reference to the byte.
     */
    uint8_t &operator[](size_t index) { return data_[index]; }
    /**
     * @brief Accesses a byte at a specific index (const version).
     * @param index The index of the byte.
     * @return A const reference to the byte.
     */
    const uint8_t &operator[](size_t index) const { return data_[index]; }

    /**
     * @brief Compares this DataBuffer with another for equality.
     * @param other The DataBuffer to compare with.
     * @return true if the buffers are equal, false otherwise.
     */
    bool operator==(const DataBuffer &other) const;
    /**
     * @brief Compares this DataBuffer with another for inequality.
     * @param other The DataBuffer to compare with.
     * @return true if the buffers are not equal, false otherwise.
     */
    bool operator!=(const DataBuffer &other) const { return !(*this == other); }

    /**
     * @brief Gets the current size of the data in the buffer.
     * @return The size of the data in bytes.
     */
    size_t Size() const { return size_; }
    /**
     * @brief Sets the size of the data in the buffer.
     * @param len The new size.
     */
    void SetSize(size_t len);

    /**
     * @brief Gets the total capacity of the buffer.
     * @return The capacity of the buffer in bytes.
     */
    size_t Capacity() const { return capacity_; }
    /**
     * @brief Sets the capacity of the buffer.
     * @param len The new capacity.
     */
    void SetCapacity(size_t len);

    /**
     * @brief Checks if the buffer is empty.
     * @return true if the buffer is empty, false otherwise.
     */
    bool Empty() const { return size_ == 0; }
    /**
     * @brief Clears the buffer's content.
     */
    void Clear();
    /**
     * @brief Prints a hex dump of the buffer's content.
     * @param len The number of bytes to dump (0 for all).
     */
    void HexDump(size_t len = 0);
    /**
     * @brief Converts the buffer's content to a string.
     * @return The content as a std::string.
     */
    std::string ToString();

private:
    uint8_t *data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

} // namespace lmshao::coreutils

#endif // LMSHAO_CORE_UTILS_DATA_BUFFER_H