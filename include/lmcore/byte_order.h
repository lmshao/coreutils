/**
 * @author SHAO Liming <lmshao@163.com>
 * @copyright Copyright (c) 2025 SHAO Liming
 * @license MIT
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LMSHAO_LMCORE_BYTE_ORDER_H
#define LMSHAO_LMCORE_BYTE_ORDER_H

#include <cstdint>
#include <type_traits>

namespace lmshao::lmcore {

/**
 * @brief Byte order conversion utilities
 */
class ByteOrder {
public:
    /**
     * @brief Check if system is little endian
     * @return true if little endian, false if big endian
     */
    static constexpr bool IsSystemLittleEndian() { return IsLittleEndian(); }

    /**
     * @brief Convert 16-bit value from host to network byte order
     * @param value Host byte order value
     * @return Network byte order value
     */
    static uint16_t HostToNetwork16(uint16_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
        } else {
            return value;
        }
    }

    /**
     * @brief Convert 32-bit value from host to network byte order
     * @param value Host byte order value
     * @return Network byte order value
     */
    static uint32_t HostToNetwork32(uint32_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) |
                   ((value >> 24) & 0xFF);
        } else {
            return value;
        }
    }

    /**
     * @brief Convert 64-bit value from host to network byte order
     * @param value Host byte order value
     * @return Network byte order value
     */
    static uint64_t HostToNetwork64(uint64_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFFULL) << 56) | (((value >> 8) & 0xFFULL) << 48) | (((value >> 16) & 0xFFULL) << 40) |
                   (((value >> 24) & 0xFFULL) << 32) | (((value >> 32) & 0xFFULL) << 24) |
                   (((value >> 40) & 0xFFULL) << 16) | (((value >> 48) & 0xFFULL) << 8) | ((value >> 56) & 0xFFULL);
        } else {
            return value;
        }
    }

    /**
     * @brief Convert 16-bit value from network to host byte order
     * @param value Network byte order value
     * @return Host byte order value
     */
    static uint16_t NetworkToHost16(uint16_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
        } else {
            return value;
        }
    }

    /**
     * @brief Convert 32-bit value from network to host byte order
     * @param value Network byte order value
     * @return Host byte order value
     */
    static uint32_t NetworkToHost32(uint32_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) |
                   ((value >> 24) & 0xFF);
        } else {
            return value;
        }
    }

    /**
     * @brief Convert 64-bit value from network to host byte order
     * @param value Network byte order value
     * @return Host byte order value
     */
    static uint64_t NetworkToHost64(uint64_t value)
    {
        if constexpr (IsLittleEndian()) {
            return ((value & 0xFFULL) << 56) | (((value >> 8) & 0xFFULL) << 48) | (((value >> 16) & 0xFFULL) << 40) |
                   (((value >> 24) & 0xFFULL) << 32) | (((value >> 32) & 0xFFULL) << 24) |
                   (((value >> 40) & 0xFFULL) << 16) | (((value >> 48) & 0xFFULL) << 8) | ((value >> 56) & 0xFFULL);
        } else {
            return value;
        }
    }

    /**
     * @brief Read 16-bit big-endian value from buffer
     * @param buffer Buffer to read from
     * @return Host byte order value
     */
    static uint16_t ReadBE16(const uint8_t *buffer)
    {
        uint16_t value = (static_cast<uint16_t>(buffer[0]) << 8) | buffer[1];
        if constexpr (IsLittleEndian()) {
            return NetworkToHost16(value);
        } else {
            return value;
        }
    }

    /**
     * @brief Read 32-bit big-endian value from buffer
     * @param buffer Buffer to read from
     * @return Host byte order value
     */
    static uint32_t ReadBE32(const uint8_t *buffer)
    {
        uint32_t value = (static_cast<uint32_t>(buffer[0]) << 24) | (static_cast<uint32_t>(buffer[1]) << 16) |
                         (static_cast<uint32_t>(buffer[2]) << 8) | buffer[3];
        if constexpr (IsLittleEndian()) {
            return NetworkToHost32(value);
        } else {
            return value;
        }
    }

    /**
     * @brief Read 64-bit big-endian value from buffer
     * @param buffer Buffer to read from
     * @return Host byte order value
     */
    static uint64_t ReadBE64(const uint8_t *buffer)
    {
        uint64_t value = (static_cast<uint64_t>(buffer[0]) << 56) | (static_cast<uint64_t>(buffer[1]) << 48) |
                         (static_cast<uint64_t>(buffer[2]) << 40) | (static_cast<uint64_t>(buffer[3]) << 32) |
                         (static_cast<uint64_t>(buffer[4]) << 24) | (static_cast<uint64_t>(buffer[5]) << 16) |
                         (static_cast<uint64_t>(buffer[6]) << 8) | buffer[7];
        if constexpr (IsLittleEndian()) {
            return NetworkToHost64(value);
        } else {
            return value;
        }
    }

    /**
     * @brief Write 16-bit value to buffer in big-endian format
     * @param buffer Buffer to write to
     * @param value Host byte order value
     */
    static void WriteBE16(uint8_t *buffer, uint16_t value)
    {
        uint16_t be_value;
        if constexpr (IsLittleEndian()) {
            be_value = HostToNetwork16(value);
        } else {
            be_value = value;
        }
        buffer[0] = (be_value >> 8) & 0xFF;
        buffer[1] = be_value & 0xFF;
    }

    /**
     * @brief Write 32-bit value to buffer in big-endian format
     * @param buffer Buffer to write to
     * @param value Host byte order value
     */
    static void WriteBE32(uint8_t *buffer, uint32_t value)
    {
        uint32_t be_value;
        if constexpr (IsLittleEndian()) {
            be_value = HostToNetwork32(value);
        } else {
            be_value = value;
        }
        buffer[0] = (be_value >> 24) & 0xFF;
        buffer[1] = (be_value >> 16) & 0xFF;
        buffer[2] = (be_value >> 8) & 0xFF;
        buffer[3] = be_value & 0xFF;
    }

    /**
     * @brief Write 64-bit value to buffer in big-endian format
     * @param buffer Buffer to write to
     * @param value Host byte order value
     */
    static void WriteBE64(uint8_t *buffer, uint64_t value)
    {
        uint64_t be_value;
        if constexpr (IsLittleEndian()) {
            be_value = HostToNetwork64(value);
        } else {
            be_value = value;
        }
        buffer[0] = (be_value >> 56) & 0xFF;
        buffer[1] = (be_value >> 48) & 0xFF;
        buffer[2] = (be_value >> 40) & 0xFF;
        buffer[3] = (be_value >> 32) & 0xFF;
        buffer[4] = (be_value >> 24) & 0xFF;
        buffer[5] = (be_value >> 16) & 0xFF;
        buffer[6] = (be_value >> 8) & 0xFF;
        buffer[7] = be_value & 0xFF;
    }

private:
    /**
     * @brief Detect if system is little endian at compile time
     * @return true if little endian, false if big endian
     */
    static constexpr bool IsLittleEndian()
    {
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
        return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
        return true; // x86/x64 architectures are little endian
#else
        // Compile-time endianness detection using constexpr
        constexpr uint32_t test = 0x01020304;
        return static_cast<const char *>(static_cast<const void *>(&test))[0] == 0x04;
#endif
    }
};

} // namespace lmshao::lmcore

#endif // LMSHAO_LMCORE_BYTE_ORDER_H