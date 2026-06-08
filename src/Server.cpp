#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <system_error>
#include <cstring>
#include <cerrno>

namespace zenith {

Server::Server(int port) : port_(port) {
    initialize_server_socket();
}

Server::~Server() {
    if (listen_fd_ != -1) {
        close(listen_fd_);
    }
}

void Server::initialize_server_socket() {
    // 1. Criar o socket IPv4 para streaming TCP
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao criar o socket do servidor");
    }

    // 2. Otimização: SO_REUSEADDR para evitar o erro "Address already in use" ao reiniciar o servidor
    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao configurar SO_REUSEADDR");
    }

    // Otimização Adicional: SO_REUSEPORT para permitir re-bind imediato em kernels modernos
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao configurar SO_REUSEPORT");
    }

    // 3. Configurar o socket do servidor como NÃO-BLOQUEANTE
    int flags = fcntl(listen_fd_, F_GETFL, 0);
    if (flags == -1 || fcntl(listen_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao configurar socket como O_NONBLOCK");
    }

    // 4. Vincular o socket ao IP (Qualquer interface) e à Porta pretendida
    sockaddr_in address{};
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Escuta em todas as interfaces de rede
    address.sin_port = htons(port_);

    if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao fazer bind na porta");
    }

    // 5. Colocar o socket em modo de escuta ativa (O SOMAXCONN define o limite máximo da fila do kernel)
    if (listen(listen_fd_, SOMAXCONN) == -1) {
        throw std::system_error(errno, std::generic_category(), "Falha ao colocar o socket em listen");
    }
}

Connection* Server::accept_connection() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    // Aceitar a conexão pendente
    int client_fd = accept(listen_fd_, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
    
    if (client_fd == -1) {
        // Como o socket de escuta é não-bloqueante, o accept pode retornar -1 com EAGAIN 
        // se não houver conexões reais na fila no milissegundo em que foi chamado.
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return nullptr;
        }
        throw std::system_error(errno, std::generic_category(), "Erro crítico ao aceitar conexão");
    }

    // Configurar o socket do novo cliente também como NÃO-BLOQUEANTE
    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(client_fd);
        throw std::system_error(errno, std::generic_category(), "Falha ao configurar socket do cliente como O_NONBLOCK");
    }

    // Retorna a nova Connection instanciada. O ownership do ponteiro vai para quem chamou (o EventLoop)
    return new Connection(client_fd);
}

} // namespace zenith