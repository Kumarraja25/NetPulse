#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <mutex>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// =========================================
// CLIENT INFORMATION
// =========================================

struct ClientInfo {
    int id;
    int socket;
    std::string ip;
    int port;
};

// =========================================
// GLOBAL STATISTICS
// =========================================

std::atomic<int> activeClients(0);
std::atomic<int> nextClientId(1);
std::atomic<int> totalClients(0);

std::atomic<long long> totalMessages(0);
std::atomic<long long> totalBytesReceived(0);
std::atomic<long long> totalBytesSent(0);

// =========================================
// SHARED DATA
// =========================================

std::vector<ClientInfo> clients;

std::mutex clientsMutex;
std::mutex logMutex;
std::mutex consoleMutex;

std::atomic<bool> serverRunning(true);

// =========================================
// GET CURRENT TIME
// =========================================

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();

    std::time_t currentTime =
        std::chrono::system_clock::to_time_t(now);

    char timeBuffer[100];

    std::strftime(
        timeBuffer,
        sizeof(timeBuffer),
        "%Y-%m-%d %H:%M:%S",
        std::localtime(&currentTime)
    );

    return std::string(timeBuffer);
}

// =========================================
// LOG EVENT
// =========================================

void logEvent(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);

    std::ofstream logFile(
        "../logs/netpulse.log",
        std::ios::app
    );

    if (logFile.is_open()) {
        logFile
            << "["
            << getCurrentTime()
            << "] "
            << message
            << "\n";
    }
}

// =========================================
// DISPLAY HELP
// =========================================

void displayHelp() {
    std::lock_guard<std::mutex> lock(consoleMutex);

    std::cout << "\n";
    std::cout << "========== NETPULSE COMMANDS ==========\n";
    std::cout << "help     - Show available commands\n";
    std::cout << "stats    - Show server statistics\n";
    std::cout << "clients  - Show active clients\n";
    std::cout << "clearlog - Clear the log file\n";
    std::cout << "exit     - Stop the server\n";
    std::cout << "========================================\n";
}

// =========================================
// DISPLAY STATISTICS
// =========================================

void displayStatistics() {
    std::lock_guard<std::mutex> lock(consoleMutex);

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "        NETPULSE SERVER STATISTICS\n";
    std::cout << "========================================\n";

    std::cout
        << "Total Clients Connected : "
        << totalClients.load()
        << "\n";

    std::cout
        << "Active Clients          : "
        << activeClients.load()
        << "\n";

    std::cout
        << "Total Messages Received : "
        << totalMessages.load()
        << "\n";

    std::cout
        << "Total Bytes Received    : "
        << totalBytesReceived.load()
        << "\n";

    std::cout
        << "Total Bytes Sent        : "
        << totalBytesSent.load()
        << "\n";

    std::cout << "========================================\n";
}

// =========================================
// DISPLAY ACTIVE CLIENTS
// =========================================

void displayClients() {
    std::lock_guard<std::mutex> lock(clientsMutex);

    std::lock_guard<std::mutex> consoleLock(consoleMutex);

    std::cout << "\n";
    std::cout << "========== ACTIVE CLIENTS ==========\n";

    if (clients.empty()) {
        std::cout << "No active clients.\n";
    }
    else {
        std::cout
            << "ID\tIP Address\t\tPort\n";

        std::cout
            << "--------------------------------------\n";

        for (const auto& client : clients) {

            std::cout
                << client.id
                << "\t"
                << client.ip
                << "\t\t"
                << client.port
                << "\n";
        }
    }

    std::cout
        << "Total Active Clients: "
        << clients.size()
        << "\n";

    std::cout
        << "====================================\n";
}

// =========================================
// CLEAR LOG FILE
// =========================================

void clearLog() {
    std::lock_guard<std::mutex> lock(logMutex);

    std::ofstream logFile(
        "../logs/netpulse.log",
        std::ios::trunc
    );

    std::lock_guard<std::mutex> consoleLock(consoleMutex);

    std::cout
        << "Log file cleared successfully.\n";
}

// =========================================
// HANDLE ONE CLIENT
// =========================================

void handleClient(
    int client_fd,
    int clientId,
    std::string clientIP,
    int clientPort
) {
    char buffer[1024];

    long long clientMessages = 0;
    long long clientBytesReceived = 0;
    long long clientBytesSent = 0;

    auto connectionStart =
        std::chrono::steady_clock::now();

    // Update global counters
    activeClients++;
    totalClients++;

    // Add client to registry
    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        clients.push_back({
            clientId,
            client_fd,
            clientIP,
            clientPort
        });
    }

    // Display connection information
    {
        std::lock_guard<std::mutex> lock(consoleMutex);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "[Client #" << clientId << " CONNECTED]\n";
        std::cout << "IP Address     : " << clientIP << "\n";
        std::cout << "Port           : " << clientPort << "\n";
        std::cout << "Time           : "
                  << getCurrentTime() << "\n";
        std::cout << "Active Clients : "
                  << activeClients.load() << "\n";
        std::cout << "========================================\n";
    }

    logEvent(
        "Client #" + std::to_string(clientId)
        + " CONNECTED | IP: "
        + clientIP
        + " | Port: "
        + std::to_string(clientPort)
    );

    // =====================================
    // CLIENT COMMUNICATION LOOP
    // =====================================

    while (serverRunning) {

        std::memset(
            buffer,
            0,
            sizeof(buffer)
        );

        int bytesReceived = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        // Client disconnected
        if (bytesReceived <= 0) {
            break;
        }

        // Update statistics
        clientMessages++;
        clientBytesReceived += bytesReceived;

        totalMessages++;
        totalBytesReceived += bytesReceived;

        std::string message(
            buffer,
            bytesReceived
        );

        {
            std::lock_guard<std::mutex> lock(consoleMutex);

            std::cout
                << "[Client #"
                << clientId
                << "] "
                << message
                << "\n";
        }

        logEvent(
            "Client #"
            + std::to_string(clientId)
            + " MESSAGE: "
            + message
        );

        // Create server reply
        std::string reply =
            "Server received: " + message;

        int bytesSent = send(
            client_fd,
            reply.c_str(),
            reply.length(),
            0
        );

        if (bytesSent > 0) {

            clientBytesSent += bytesSent;

            totalBytesSent += bytesSent;
        }

        // Client requested exit
        if (message == "exit") {
            break;
        }
    }

    // =====================================
    // CLIENT DISCONNECT
    // =====================================

    close(client_fd);

    activeClients--;

    // Remove from registry
    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        clients.erase(
            std::remove_if(
                clients.begin(),
                clients.end(),

                [clientId](const ClientInfo& client) {
                    return client.id == clientId;
                }
            ),

            clients.end()
        );
    }

    // Calculate connection duration
    auto connectionEnd =
        std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<
            std::chrono::seconds
        >(
            connectionEnd - connectionStart
        ).count();

    {
        std::lock_guard<std::mutex> lock(consoleMutex);

        std::cout << "\n";
        std::cout << "----------------------------------------\n";
        std::cout
            << "[Client #"
            << clientId
            << " DISCONNECTED]\n";

        std::cout
            << "Connection Duration : "
            << duration
            << " seconds\n";

        std::cout
            << "Messages            : "
            << clientMessages
            << "\n";

        std::cout
            << "Bytes Received      : "
            << clientBytesReceived
            << "\n";

        std::cout
            << "Bytes Sent          : "
            << clientBytesSent
            << "\n";

        std::cout
            << "Active Clients      : "
            << activeClients.load()
            << "\n";

        std::cout
            << "----------------------------------------\n";
    }

    logEvent(
        "Client #"
        + std::to_string(clientId)
        + " DISCONNECTED"
        + " | Duration: "
        + std::to_string(duration)
        + "s"
        + " | Messages: "
        + std::to_string(clientMessages)
        + " | Bytes Received: "
        + std::to_string(clientBytesReceived)
        + " | Bytes Sent: "
        + std::to_string(clientBytesSent)
    );
}

// =========================================
// COMMAND CONSOLE
// =========================================

void commandConsole() {

    std::string command;

    while (serverRunning) {

        {
            std::lock_guard<std::mutex> lock(consoleMutex);

            std::cout << "\nNetPulse> ";
        }

        if (!std::getline(std::cin, command)) {
            break;
        }

        if (command == "help") {

            displayHelp();

        }
        else if (command == "stats") {

            displayStatistics();

        }
        else if (command == "clients") {

            displayClients();

        }
        else if (command == "clearlog") {

            clearLog();

        }
        else if (command == "exit") {

            {
                std::lock_guard<std::mutex> lock(consoleMutex);

                std::cout
                    << "Stopping NetPulse server...\n";
            }

            serverRunning = false;

            logEvent(
                "NetPulse Server STOPPED"
            );

            break;
        }
        else if (command.empty()) {

            continue;

        }
        else {

            std::lock_guard<std::mutex> lock(consoleMutex);

            std::cout
                << "Unknown command. Type 'help'.\n";
        }
    }
}

// =========================================
// MAIN
// =========================================

int main() {

    // Create TCP socket
    int server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd < 0) {

        std::cerr
            << "Failed to create socket.\n";

        return 1;
    }

    // Allow port reuse
    int opt = 1;

    setsockopt(
        server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    // Server address
    sockaddr_in address{};

    address.sin_family = AF_INET;

    address.sin_addr.s_addr =
        INADDR_ANY;

    address.sin_port =
        htons(8080);

    // Bind socket
    if (bind(
            server_fd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) < 0) {

        std::cerr
            << "Bind failed.\n";

        close(server_fd);

        return 1;
    }

    // Listen for clients
    if (listen(server_fd, 10) < 0) {

        std::cerr
            << "Listen failed.\n";

        close(server_fd);

        return 1;
    }

    // Server startup message
    {
        std::lock_guard<std::mutex> lock(consoleMutex);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "          NETPULSE SERVER v2.0\n";
        std::cout << "========================================\n";
        std::cout << "Listening on port 8080\n";
        std::cout << "Type 'help' for commands.\n";
        std::cout << "========================================\n";
    }

    logEvent(
        "NetPulse Server v2.0 STARTED on port 8080"
    );

    // Start command console
    std::thread consoleThread(
        commandConsole
    );

    consoleThread.detach();

    // =====================================
    // ACCEPT CLIENTS
    // =====================================

    while (serverRunning) {

        sockaddr_in clientAddress{};

        socklen_t clientLength =
            sizeof(clientAddress);

        int client_fd = accept(
            server_fd,
            reinterpret_cast<sockaddr*>(
                &clientAddress
            ),
            &clientLength
        );

        if (client_fd < 0) {

            if (!serverRunning) {
                break;
            }

            continue;
        }

        // Get client IP
        char clientIPBuffer[
            INET_ADDRSTRLEN
        ];

        inet_ntop(
            AF_INET,
            &clientAddress.sin_addr,
            clientIPBuffer,
            sizeof(clientIPBuffer)
        );

        std::string clientIP =
            clientIPBuffer;

        // Get client port
        int clientPort =
            ntohs(
                clientAddress.sin_port
            );

        // Assign ID
        int clientId =
            nextClientId++;

        // Handle client independently
        std::thread(
            handleClient,
            client_fd,
            clientId,
            clientIP,
            clientPort
        ).detach();
    }

    close(server_fd);

    return 0;
}
