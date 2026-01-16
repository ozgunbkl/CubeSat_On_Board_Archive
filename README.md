# 📦 On-Board Data Archiving System (OBDH / Archive Service)

A modular, deterministic **On-Board Data Handling (OBDH)** subsystem designed for a simulated 1U/3U CubeSat Flight Software (FSW) stack.  
This service provides reliable, persistent storage for mission telemetry and events using non-volatile memory simulation with **CRC-16 integrity verification**.

---

## 🎯 Project Objective

The primary objective of this project is to implement a **"Black Box" data recorder** for Flight Software.

The Archive Service ensures that critical mission data is:
- Stored sequentially during nominal operations
- Preserved across simulated resets or power cycles
- Available for later downlink to the Ground Segment

The subsystem is designed **without assuming the presence of a global Time Service or FDIR**, enabling clean standalone verification and future system integration.

---

## 🧠 Architectural Design

The Archive Service follows a layered, hardware-independent architecture:

### 1. Archive Service Core
- Manages logical record structures
- Maintains sequential write/read pointers
- Validates inputs and memory bounds

### 2. Storage Backend
- Simulates non-volatile memory (Flash / EEPROM)
- Implemented using a static byte array
- Enforces strict capacity limits to prevent overflow

### 3. Utility Layer
- Abstracts integrity checking (CRC-16)
- Designed for future replacement with ECSS / CCSDS-compliant implementations

---

## 🧩 Key Features

- **Deterministic Memory Management**  
  Uses fixed-size records and **O(1)** indexing to guarantee predictable execution time, critical for real-time FSW.

- **Encapsulated Record Format**  
  Each entry is wrapped in a structured `ArchiveRecord_t` envelope containing metadata and integrity checks.

- **Radiation Resilience (Simulated)**  
  CRC verification is performed on every read operation to detect:
  - Single Event Upsets (SEUs)
  - Bit corruption during storage

- **Software-in-the-Loop (SIL) Ready**  
  Developed without hardware dependencies and fully verifiable on a host PC using unit tests.

---

## 🛠️ Technical Implementation

### Record Structure

```c
typedef struct {
    uint16_t record_id;       // Source subsystem identifier
    uint16_t length;          // Actual payload size
    uint8_t  payload[128];    // Fixed-size telemetry buffer
    uint16_t crc;             // Integrity seal (CRC-16)
} ArchiveRecord_t;
```
Each record is self-contained and validated independently.

Fault Detection & Status Codes
The Archive Service reports explicit status codes to calling subsystems:

ARCHIVE_OK – Operation successful

ARCHIVE_ERR_FULL – Non-volatile memory capacity reached

ARCHIVE_ERR_CRC – Data corruption detected during readback

ARCHIVE_ERR_PARAM – Invalid input pointer or payload size

This allows higher-level software (FDIR, CDHS) to react appropriately.

🧪 Verification & Validation
The subsystem is verified using the Unity Test Framework.
Covered mission-critical scenarios include:

Normal Write/Read Cycle  
Confirms data remains bit-perfect through storage and retrieval.

Buffer Overflow Protection  
Ensures writes are rejected once the 4096-byte memory limit is reached.

Corruption Detection  
Simulated “radiation strike” test where memory is manually altered to verify CRC error detection.

📁 Project Structure
```
archive/
 ├── include/
 │    ├── archive_service.h   # Public API and record definitions
 │    └── utils.h             # Integrity check abstractions
 ├── src/
 │    ├── archive_service.c   # Core storage logic & memory simulation
 │    └── utils.c             # CRC-16 implementation
 ├── test/
 │    ├── test_archive.c      # Unity test suite
 │    └── unity/              # Unity Test Framework
 └── README.md
```
🚀 Planned System Integration (Future Work)
This module is designed to integrate into a full CubeSat FSW stack.
Planned upgrades include:

Time Service Integration  
Adding timestamps (e.g., PUS-compatible) to each record.

ECSS / CCSDS Alignment  
Mapping Record IDs to standardized packet and service definitions.

FDIR Integration  
Enabling autonomous fault responses when storage corruption or capacity faults are detected.
