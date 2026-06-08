#pragma once

#include "Connection.hpp"
#include <string>
#include <system_error>

namespace zenith 
{

    /// @brief Responsável por gerir o socket de escuta principal (Listening Socket) do servidor.
    class Server : public NetworkEntity
    {
        public:
            /// @brief Cria o servidor e vincula-o a uma porta específica.
            /// @param port A porta onde o ZenithCache vai correr (ex: 6379).
            explicit Server(int port);

            /// @brief Garante o fecho seguro do socket de escuta (RAII).
            ~Server() override;

            // Implementação da interface NetworkEntity
            int fd() const override { return listen_fd_; }
            EntityType type() const override { return EntityType::Listener; }

            /// @brief Aceita uma nova conexão de um cliente pendente na fila.
            /// @note Configura o socket do cliente automaticamente como Não-Bloqueante (O_NONBLOCK).
            /// @return Ponteiro para a nova Connection alocada.
            Connection* accept_connection();

        private:
            int listen_fd_{-1};
            int port_;

            /// @brief Cria, configura (SO_REUSEADDR, O_NONBLOCK), faz bind e listen no socket.
            void initialize_server_socket();
    };

} // namespace zenith