# NetPulse

## Multithreaded TCP Network Monitoring System

NetPulse is a C++ based multithreaded TCP server and client application designed to monitor network connections and communication statistics.

The system allows multiple TCP clients to connect simultaneously while the server tracks client information, messages, data transfer statistics, connection duration, and server events.

## Features

- TCP socket programming
- Multi-client communication
- Multithreading using `std::thread`
- Unique client identification
- Client IP and port detection
- Active client registry
- Thread-safe shared data using mutexes
- Message monitoring
- Bytes sent and received tracking
- Connection duration monitoring
- Thread-safe file logging
- Interactive server dashboard

## Architecture

```text
                     NetPulse Server
                            |
                       Main Thread
                            |
                         accept()
                            |
          +-----------------+-----------------+
          |                 |                 |
       Thread 1          Thread 2          Thread N
          |                 |                 |
       Client 1          Client 2          Client N


                    Command Console
                           |
                +----------+----------+
                |          |          |
              stats      clients     help
```

## Project Structure

```text
NetPulse/
|
|-- src/
|   |-- server.cpp
|   `-- client.cpp
|
|-- logs/
|   `-- netpulse.log
|
|-- README.md
|-- DOCUMENTATION.md
|-- Makefile
`-- .gitignore
```

## Technologies Used

- C++
- POSIX Socket API
- TCP/IP Networking
- Linux / Ubuntu
- WSL
- Multithreading
- Mutex Synchronization
- Git and GitHub

## Requirements

- Linux or WSL Ubuntu
- g++
- make

Install required tools:

```bash
sudo apt update
sudo apt install build-essential
```

## Build

Clone the repository:

```bash
git clone <your-repository-url>
cd NetPulse
```

Compile:

```bash
make
```

## Run

Start the server:

```bash
./src/server
```

Start a client in another terminal:

```bash
./src/client
```

Multiple clients can connect simultaneously.

## Server Commands

| Command | Description |
|---|---|
| `help` | Display available commands |
| `stats` | Display server statistics |
| `clients` | Display currently connected clients |
| `clearlog` | Clear the log file |
| `exit` | Stop the server |

## Example Statistics

```text
========================================
        NETPULSE SERVER STATISTICS
========================================

Total Clients Connected : 5
Active Clients          : 2
Total Messages Received : 24
Total Bytes Received    : 1250
Total Bytes Sent        : 1680
========================================
```

## Learning Outcomes

Through this project, I learned:

- TCP/IP communication
- Socket lifecycle
- Client-server architecture
- `socket()`
- `bind()`
- `listen()`
- `accept()`
- `connect()`
- `send()`
- `recv()`
- Multithreading
- Mutex synchronization
- Shared resource management
- Network connection monitoring
- File logging

## Future Improvements

- UDP support
- Packet capture using libpcap
- Wireshark integration
- Real-time bandwidth calculation
- Web dashboard
- Authentication
- TLS encryption
- Epoll-based high-performance server
- Docker deployment

## Author

Raja Kumar Bisi
M.Tech Computer Science and Engineering
