#include "EventLoop.hpp"
#include "Server.hpp"
#include <iostream>
#include <csignal>
#include <stop_token>
#include <thread>

// Stop source global temporário para o signal handler conseguir aceder
std::stop_source global_stop_source;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n[Sinal] Ctrl+C detetado! Encerrando o ZenithCache graciosamente...\n";
        global_stop_source.request_stop();
    }
}

int main() {
    std::signal(SIGINT, signal_handler);

    std::cout << "=========================================\n";
    std::cout << "   ZenithCache - In-Memory Store (v0.1)  \n";
    std::cout << "=========================================\n";

    try {
        int port = 6379; // Porta padrão estilo Redis
        zenith::Server server(port);
        zenith::EventLoop loop;

        // Registar o servidor no EventLoop para escutar conexões (EPOLLIN)
        // Usamos Edge-Triggered (EPOLLET) para performance máxima
        // Passamos o ponteiro da instância 'server' para que o loop identifique a entidade nos eventos
        loop.register_fd(server.fd(), EPOLLIN | EPOLLET, &server);
        
        std::cout << "[Sucesso] Servidor escutando na porta " << port << "...\n";

        // Rodar o loop de eventos passando o stop_token do C++20
        loop.run(global_stop_source.get_token());

    } catch (const std::exception& e) {
        std::cerr << "[CRÍTICO] Exceção no main: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Fim] Servidor terminado com sucesso.\n";
    return 0;
}