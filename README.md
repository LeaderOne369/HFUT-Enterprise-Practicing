# HospAI — Intelligent Hospital Customer Service System (Qt/C++ + AI)

Modern, cross‑platform hospital customer service system featuring AI triage, real‑time consultation, hospital navigation, user/role management, operational analytics, and compliance‑ready auditing. Built with Qt 6 and C++17, backed by SQLite, and integrated with ByteDance Doubao AI for clinical triage assistance.

Badges: `Qt 6.x` · `C++17` · `SQLite` · `Doubao AI` · `Cross‑platform (Windows/macOS/Linux)` · `qmake/CMake`

## Overview

HospAI is an industry‑grade desktop application designed to streamline hospital front‑desk operations and patient experience. The system supports three roles — Patient, Staff (Customer Service), and Administrator — delivering AI‑assisted symptom assessment, seamless handoff to human agents, and robust administration with analytics and audit trails.

This project was developed under an enterprise‑style capstone program jointly run by Hefei University of Technology (HFUT) and GuoChuang Software Co., Ltd., with a strong emphasis on production‑ready architecture, security, maintainability, and cross‑platform delivery.

## Key Capabilities

- AI Triage and Guidance

  - Symptom understanding via natural‑language input
  - Department recommendation and urgency estimation
  - Actionable care guidance with safety caveats

- Real‑time Consultation Workflow

  - Role‑aware chat between patients and staff
  - Session lifecycle management (Waiting, In‑Progress, Ended)
  - Automatic escalation from AI to human agents
  - Timer‑based polling for consistent delivery in desktop contexts

- Hospital Navigation

  - Programmatically rendered, color‑coded floor maps
  - Clickable destinations and path visualization with arrows/markers
  - Clear navigation notes (ETA, special instructions)

- Administration and Governance
  - User and role management with RBAC
  - System statistics (user engagement, chat/session KPIs)
  - Configuration management (AI parameters, FAQ, department meta)
  - Comprehensive audit logging (operations, sessions, system events)

## Architecture and Technology

- Framework: Qt 6.x (C++17), Qt Widgets, Qt Network
- Data Layer: SQLite 3 (local, transactional)
- AI Integration: ByteDance Doubao API (REST over Qt Network)
- Build: qmake and CMake (both supported)
- Platforms: Windows, macOS, Linux

Layered design with modular boundaries:

- Core: database access, AI client, message storage/types, user roles
- Views: patient/staff/admin UIs, shared widgets and windowing
- Utilities: style/theming, settings, dialogs, navigation components

Database model highlights: Users, Sessions, Messages, Quick Replies, System Config. Passwords are stored as salted hashes; access is guarded by role‑based authorization.

## Project Structure

```
HospAI/
├── main.cpp                     # Application entry
├── mainwindow.*                 # Primary window and UI
├── HospAI.pro                   # qmake project file
├── CMakeLists.txt               # CMake project file
├── src/
│   ├── core/                    # Database, AI client, storage, types
│   └── views/                   # UI modules: common, patient, staff, admin
└── html/                        # HTML templates (mock/demo assets)
```

## Build and Run

### Prerequisites

- Qt 6.0+ (or Qt 5.15+)
- C++17‑capable compiler
- SQLite3

### Build with qmake (recommended)

```bash
qmake HospAI.pro
make
```

### Build with CMake

```bash
mkdir build && cd build
cmake ..
make -j4
```

### Run

```bash
./HospAI        # Linux/macOS
# or HospAI.exe # Windows
```

## Demo Accounts

Use the following seeded credentials for local testing:

| Username | Password | Role    | Notes            |
| -------- | -------- | ------- | ---------------- |
| guanli1  | 123456   | Admin   | System admin     |
| kefu1    | 123456   | Staff   | Customer service |
| huanzhe1 | 123456   | Patient | Standard patient |

## Usage Guide

- Patient

  1. Log in → AI Triage for symptom assessment
  2. Optionally escalate to human staff
  3. Use Hospital Navigation for department routes

- Staff (Customer Service)

  1. Log in → view waiting patients
  2. Accept a session and chat in real time
  3. Record outcomes and manage session states

- Administrator
  1. Manage users and role assignments
  2. Review system and engagement analytics
  3. Configure AI parameters, FAQs, department meta
  4. Audit system and session logs

## AI and Reliability

- Doubao AI REST integration encapsulated in an `AIApiClient` with
  - asynchronous requests
  - robust error handling and retry policy
- Deterministic desktop delivery via timer‑based polling

## Security and Compliance

- Password hashing and secure credential handling
- Role‑based access control (RBAC)
- Parameterized DB operations and validation
- End‑to‑end audit trail for admin and operational actions

## Performance and UX

- Optimized chat polling cadence for multi‑concurrent sessions
- Smooth session state transitions to minimize operator friction
- Consistent, modern theming via centralized style manager

## Academic–Industry Collaboration Context

This system was developed within the Enterprise Practicing program co‑organized by HFUT and GuoChuang Software, following enterprise‑grade processes: architectural design reviews, role‑based tasking, cross‑platform QA, and formal acceptance. The resulting application emphasizes correctness, maintainability, portability, and real‑world operability.

## Contributing

1. Fork the repo
2. Create a feature branch: `git checkout -b feature/AmazingFeature`
3. Commit changes: `git commit -m "Add some AmazingFeature"`
4. Push the branch: `git push origin feature/AmazingFeature`
5. Open a Pull Request

## License

MIT License — see `LICENSE` for details.

—

HospAI — Bringing intelligent, reliable assistance to hospital front desks.
