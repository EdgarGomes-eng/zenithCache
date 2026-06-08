#include "EventLoop.hpp"
#include "Server.hpp"
#include "Connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include <system_error>
#include <cerrno>

namespace zenith
{

    EventLoop::EventLoop()
    {
        epoll_fd_ = epoll_create1(0);
        if (epoll_fd_ == -1)
        {
            throw std::system_error(errno, std::generic_category(), "Falha ao criar o epoll file descriptor");
        }
        events_buffer_.resize(MAX_EVENTS);
    }

    EventLoop::~EventLoop()
    {
        if (epoll_fd_ != -1)
        {
            close(epoll_fd_);
        }
    }

    void EventLoop::register_fd(int fd, uint32_t events, void* ptr)
    {
        epoll_event ev{};
        ev.events = events;
        if (ptr) ev.data.ptr = ptr;
        else ev.data.fd = fd;

        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            throw std::system_error(errno, std::generic_category(), "Erro ao adicionar FD ao epoll");
        }
    }

    void EventLoop::modify_fd(int fd, uint32_t events)
    {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1)
        {
            throw std::system_error(errno, std::generic_category(), "Erro ao modificar FD no epoll");
        }
    }

    void EventLoop::unregister_fd(int fd)
    {
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            throw std::system_error(errno, std::generic_category(), "Erro ao remover FD do epoll");
        }
    }

    void EventLoop::run(std::stop_token stop_token)
    {
        while (!stop_token.stop_requested())
        {
            int num_events = epoll_wait(epoll_fd_, events_buffer_.data(), MAX_EVENTS, EPOLL_TIMEOUT_MS);
            if (num_events == -1)
            {
                if (errno == EINTR) continue;
                throw std::system_error(errno, std::generic_category(), "Erro no epoll_wait");
            }
            handle_triggered_events(num_events);
        }
    }

    void EventLoop::handle_triggered_events(int num_events) 
    {
        for (int i = 0; i < num_events; ++i) 
        {
            const auto& event = events_buffer_[i];
            auto* entity = static_cast<NetworkEntity*>(event.data.ptr);

            if (!entity) continue;

            // Tratar erros ou desconexões brutas
            if (event.events & (EPOLLERR | EPOLLHUP)) 
            {
                unregister_fd(entity->fd());
                delete entity; 
                continue;
            }

            // Se for uma nova conexão no Servidor
            if (entity->type() == EntityType::Listener) 
            {
                auto* server = static_cast<Server*>(entity);
                while (Connection* new_conn = server->accept_connection()) 
                {
                    register_fd(new_conn->fd(), EPOLLIN | EPOLLET);
                    
                    epoll_event ev{};
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.ptr = new_conn;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, new_conn->fd(), &ev);
                }
            } 
            // Se forem dados de um cliente existente
            else if (entity->type() == EntityType::Client) 
            {
                auto* client = static_cast<Connection*>(entity);
                if (event.events & EPOLLIN) 
                {
                    char dummy_buf[1024];
                    ssize_t bytes_received = recv(client->fd(), dummy_buf, sizeof(dummy_buf) - 1, 0);
                    
                    if (bytes_received <= 0) {
                        unregister_fd(client->fd());
                        delete client;
                        continue; // Passa para o próximo evento
                    }

                    dummy_buf[bytes_received] = '\0';

                    // --- TOQUE DE MESTRE: Limpeza de quebras de linha e espaços ---
                    std::string_view msg(dummy_buf, bytes_received);

                    // Remove espaços, \n e \r do fim da string
                    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r' || msg.back() == ' ')) {
                        msg.remove_suffix(1);
                    }
                    // Remove espaços do início da string
                    while (!msg.empty() && (msg.front() == ' ' || msg.front() == '\t')) {
                        msg.remove_prefix(1);
                    }

                    // Se depois da limpeza a mensagem ficou vazia, ignoramos completamente!
                    if (msg.empty()) {
                        continue; 
                    }
                    // -------------------------------------------------------------

                    // Agora só imprime se tiver conteúdo real
                    std::cout << "[Recebido do FD " << client->fd() << "]: " << msg << "\n";
                }
            }
        }
    }

} // namespace zenith