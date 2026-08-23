# NetPulse Documentation

## Overview

NetPulse is a multithreaded TCP client-server application developed in C++ to demonstrate practical networking and systems programming concepts.

The server accepts multiple clients simultaneously, monitors communication, tracks network statistics, maintains an active client registry, and records events in a log file.

---

## Architecture

```text
                    +------------------+
                    | NetPulse Server  |
                    +------------------+
                             |
                          accept()
                             |
          +------------------+------------------+
          |                  |                  |
      Thread 1           Thread 2           Thread N
          |                  |                  |
       Client 1           Client 2           Client N


                    Command Console
                           |
            +--------------+--------------+
            |              |              |
          help           stats          clients

