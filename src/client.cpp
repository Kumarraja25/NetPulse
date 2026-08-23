#include <iostream>
#include <string>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {

    // Create TCP socket
    int client_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (client_fd < 0) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    // Server address
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    // Connect to localhost
    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &serverAddress.sin_addr
        ) <= 0) {

        std::cerr << "Invalid address.\n";
        close(client_fd);
        return 1;
    }

    // Connect to server
    if (connect(
            client_fd,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ) < 0) {

        std::cerr << "Connection failed.\n";
        close(client_fd);
        return 1;
    }

    std::cout << "\n";
    std::cout << "====================================\n";
    std::cout << "       NETPULSE TCP CLIENT\n";
    std::cout << "====================================\n";
    std::cout << "Connected to server on port 8080\n";
    std::cout << "Type 'exit' to disconnect.\n";
    std::cout << "====================================\n\n";

    char buffer[1024];
    std::string message;

    while (true) {

        std::cout << "You: ";

        std::getline(
            std::cin,
            message
        );

        // Send message
        int sent = send(
            client_fd,
            message.c_str(),
            message.length(),
            0
        );

        if (sent < 0) {
            std::cerr << "Failed to send message.\n";
            break;
        }

        // Exit requested
        if (message == "exit") {
            break;
        }

        // Clear buffer
        std::memset(
            buffer,
            0,
            sizeof(buffer)
        );

        // Receive server response
        int bytesReceived = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytesReceived <= 0) {
            std::cout << "Server disconnected.\n";
            break;
        }

        std::cout
            << "Server: "
            << std::string(
                buffer,
                bytesReceived
            )
            << "\n";
    }

    close(client_fd);

    std::cout
        << "\nDisconnected from NetPulse server.\n";

    return 0;
}

