# ZenithCache

A high-performance, ultra-fast, multi-threaded in-memory key-value store engineered in **Modern C++20**. Architected for low-latency systems, ZenithCache utilizes an asynchronous event-driven network engine to handle massive concurrency without sacrificing predictable performance.

---

## 🛑 The Problem

Modern high-throughput applications are heavily bottlenecked by traditional database architectures. The core pain points include:
1. **Disk I/O Bottlenecks:** Disk-bound databases introduce severe latency spikes, making them unsuitable for real-time caching, session management, or rapid state evaluation.
2. **Thread-Per-Connection Inefficiency:** Classic networking models allocate an operating system thread to each incoming connection. Under thousands of concurrent clients, this causes massive memory overhead and severe CPU degradation due to constant context switching.
3. **Single-Threaded CPU Bottlenecks:** While single-threaded in-memory databases avoid synchronization overhead, they fail to leverage modern multi-core processors for long-running compute operations, leading to head-of-line blocking under heavy loads.
4. **Memory Fragmentation & Leakage:** Poor resource tracking and frequent, uncontrolled heap allocations along the critical network path lead to memory leaks, high fragmentation, and unexpected garbage collection pauses.

---

## ⚡ The Solution: ZenithCache Architecture

ZenithCache resolves these challenges by decoupling the networking layer from the storage architecture, enforcing strict resource safety and maximizing cache efficiency through low-level Linux optimizations.

### 1. High-Performance Event Loop (Reactor Pattern)
Instead of spawning a thread per client, ZenithCache implements an asynchronous, non-blocking **Reactor Pattern** using Linux **`epoll`** in **Edge-Triggered (EPOLLET)** mode. 
* This allows a single network thread to multiplex thousands of active connections efficiently.
* The system is notified only when state transitions occur, avoiding the overhead of polling.

### 2. Zero-Allocation Critical Path
Memory allocation (`malloc`/`new`) during a server’s runtime is a silent performance killer. 
* ZenithCache pre-allocates contiguous event buffers (`std::vector<epoll_event>`) upon initialization.
* Request parsing is executed using **`std::string_view`**, shifting pointers over stack-allocated buffers rather than copying strings to the Heap. This ensures $O(1)$ sanitization with a **Zero-Allocation Network Path**.

### 3. Strict Resource Management (RAII)
To eliminate memory leaks and dangling file descriptors, the codebase adheres to strict **Resource Acquisition Is Initialization (RAII)** principles. Custom `NonCopyable` safety wrappers guarantee at compile-time that system resources (such as sockets and epoll instances) cannot be duplicated or corrupted.

---

## 🗺️ Project Roadmap & Milestones

- [x] **Milestone 1: Asynchronous Core Network Engine**
  - Edge-Triggered `epoll` architecture with strict event boundaries.
  - Non-blocking socket listener and clean client disconnection handlers.
  - Graceful shutdown mechanics using C++20 `std::stop_token` and Unix signal catching.
  - Zero-heap leak verification via Valgrind.
- [ ] **Milestone 2: Segmented Concurrent In-Memory Storage**
  - High-concurrency hash map utilizing fine-grained locking (`std::shared_mutex`).
  - Read/Write lock separation to ensure lock-free reads while isolating writes.
- [ ] **Milestone 3: LRU Eviction Policy & Expiration**
  - Cache memory capping through an integrated Least Recently Used (LRU) eviction algorithm.
- [ ] **Milestone 4: Durability via Write-Ahead Logging (WAL)**
  - Append-only disk logging to recover system state safely after a crash.

---

## 📂 Directory Structure

```text
zenithcache/
├── CMakeLists.txt         # Target-based Modern CMake configuration
├── README.md              # System documentation
├── include/               # Public API headers
│   └── zenith/
│       ├── EventLoop.hpp  # Epoll multiplexer
│       ├── Server.hpp     # Non-blocking listening socket
│       ├── Connection.hpp # Client connection and buffer tracking
│       └── NonCopyable.hpp# Move-only RAII utility
└── src/                   # Implementation files
    ├── main.cpp           # Entry point and signal handling
    ├── EventLoop.cpp
    ├── Server.cpp
    └── Connection.cpp

```

🛠️ Building and Running
Prerequisites
Linux OS or Windows Subsystem for Linux (WSL)

Modern C++ Compiler supporting C++20 (GCC 11+ or Clang 13+)

CMake 3.22+

Compilation Steps
Compile the project with full production optimization flags (-O3) and strict error checking (-Werror):

Bash
# 1. Create and navigate to the build directory
mkdir build && cd build

# 2. Configure the project with CMake
cmake ..

# 3. Compile the executable
make
Running the Server
Launch the compiled binary:

Bash
./zenithcache
🧪 Verification & Testing
1. Concurrency and Input Sanitization
You can stress-test the connection engine using Netcat. In a separate terminal window, execute:

Bash
nc localhost 6379
Type any payload or hit Enter. The server’s zero-allocation parsing layer automatically discards empty packets (\r\n or spaces) and securely processes valid commands.

2. Memory Leak Analysis (Valgrind)
ZenithCache is rigorously profile-tested to ensure zero runtime leaks. To verify memory cleanliness:

Bash
valgrind --leak-check=full --show-leak-kinds=all ./zenithcache
Expected Result: All heap blocks were freed -- no leaks are possible.
