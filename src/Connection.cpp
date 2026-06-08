#include "Connection.hpp"
#include <unistd.h>
#include <iostream>

namespace zenith {

Connection::Connection(int client_fd) : fd_(client_fd) {
    read_buffer_.resize(INITIAL_BUFFER_SIZE);
    write_buffer_.resize(INITIAL_BUFFER_SIZE);
    std::cout << "[Conexão] Novo cliente conectado no FD: " << fd_ << "\n";
}

Connection::~Connection() {
    if (fd_ != -1) {
        std::cout << "[Conexão] Fechando conexão do FD: " << fd_ << "\n";
        close(fd_);
    }
}

} // namespace zenith