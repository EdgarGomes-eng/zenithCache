#pragma once

#include "NonCopyable.hpp"
#include <vector>
#include <cstdint>

namespace zenith {

enum class EntityType {
    Listener,
    Client
};

/// @brief Interface base para qualquer objeto que lide com I/O no epoll.
class NetworkEntity : public NonCopyable {
public:
    virtual ~NetworkEntity() = default;
    virtual int fd() const = 0;
    virtual EntityType type() const = 0;
};

/// @brief Encapsula o estado completo de um cliente conectado.
class Connection : public NetworkEntity {
public:
    explicit Connection(int client_fd);
    ~Connection() override;

    int fd() const override { return fd_; }
    EntityType type() const override { return EntityType::Client; }

    // Buffers de alta performance para evitar alocações repetidas na Heap
    std::vector<uint8_t>& read_buffer() { return read_buffer_; }
    std::vector<uint8_t>& write_buffer() { return write_buffer_; }

private:
    int fd_;
    std::vector<uint8_t> read_buffer_;
    std::vector<uint8_t> write_buffer_;
    static constexpr size_t INITIAL_BUFFER_SIZE = 4096; // 4KB inicial
};

} // namespace zenith