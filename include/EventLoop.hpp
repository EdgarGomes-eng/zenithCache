# pragma once

# include "NonCopyable.hpp"
# include <sys/epoll.h>
# include <vector>
# include <cstdint>
# include <stop_token>

namespace zenith
{
    /// @brief Encapsula o mecanismo epoll do Linux para I/O Multiplexing assíncrono.
    class EventLoop : public NonCopyable
    {
        public:
            /// @brief Inicializa o epoll. Lança std::system_error se falhar.
            explicit EventLoop();
            /// @brief Fecha o file descriptor do epoll de forma segura.
            ~EventLoop();
            /// @brief Adiciona um file descriptor ao monitoramento do epoll.
            /// @param fd O socket ou recurso a monitorizar.
            /// @param events Máscara de eventos (ex: EPOLLIN | EPOLLET).
            void register_fd(int fd, uint32_t events, void* ptr = nullptr);

            /// @brief Modifica os eventos monitorizados de um file descriptor já registado.
            void modify_fd(int fd, uint32_t events);

            /// @brief Remove um file descriptor do monitoramento do epoll.
            void unregister_fd(int fd);

            /// @brief Executa o loop de eventos de forma contínua até o stop_token ser ativado.
            /// @param stop_token Token do C++20 para paragem cooperativa entre threads.
            void run(std::stop_token stop_token);

        private:
            static constexpr int MAX_EVENTS = 1024;
            static constexpr int EPOLL_TIMEOUT_MS = 100;

            int epoll_fd_{-1};
            std::vector<epoll_event> events_buffer_;

            /// @brief Trata internamente os eventos que dispararam.
            void handle_triggered_events(int num_events);
    };
}