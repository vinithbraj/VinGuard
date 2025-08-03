# VinGuard

**VinGuard** is a homebrew Windows anti-malware project designed to explore the internals of malware detection, behavioral monitoring, and system protection mechanisms.

> ⚠️ This is a research/learning project. It is **not intended for production use** or as a replacement for commercial antivirus solutions.

---

## 🚀 Project Goals

- Build a lightweight, modular anti-malware engine for Windows.
- Explore kernel-mode and user-mode communication.
- Detect suspicious behaviors like:
  - Unauthorized process injection
  - Suspicious file modifications
  - Unusual registry changes
  - Unauthorized DLL loads
- Implement a minimal agent-to-kernel communication pipeline.
- Learn about and experiment with:
  - Minifilter drivers (File I/O interception)
  - Process & image load notification callbacks
  - Behavioral heuristics

---

## 🧩 Components

| Component             | Description                                                  |
|-----------------------|--------------------------------------------------------------|
| `VinGuardFS.sys`      | Minifilter driver to intercept file operations               |
| `VinGuardAgent.exe`   | User-mode service that communicates with the driver          |
| `VinGuardUI.exe`      | (Planned) Minimal GUI for visualizing detections             |
| `Common/`             | Shared data structures and communication definitions         |

---

## 🛠️ Tech Stack

- Language: C++, C
- Platform: Windows 10/11 x64
- Kernel: Windows Driver Kit (WDK)
- Build System: Visual Studio 2022 (admin mode for signing)
- Communication: `FltSendMessage`, `FilterGetMessage` (for minifilter); IOCTL (planned)

---

## 🔐 Signing & Driver Deployment

1. Ensure Visual Studio is run as **Administrator**
2. Use a test certificate for signing the driver:
   - Create with `MakeCert.exe` or use a `.pfx` file
3. Enable test signing mode:
   ```cmd
   bcdedit /set testsigning on
