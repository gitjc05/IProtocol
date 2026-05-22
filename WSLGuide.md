Below is the revised slim workflow we ended up with.

# Visual Studio 2022 + CMake + WSL Linux Build Guide

Goal: take a C++ CMake project developed on Windows/MSVC and verify that it builds, runs, and debugs under Linux using WSL.

---

## 1. Install Linux-side build tools in WSL

Open Ubuntu/WSL:

```bash
sudo apt update
sudo apt install build-essential cmake gdb make ninja-build rsync zip
```

For standalone Asio:

```bash
sudo apt install libasio-dev
```

Roadblock we hit: Windows had access to `asio.hpp`, but WSL did not. The Linux build uses Linux headers, so installing it in WSL was required.

---

## 2. Add a Linux CMake configure preset

In `CMakePresets.json`, keep your existing Windows presets and add a Linux one.

Example:

```json
{
  "name": "linux-debug",
  "displayName": "Linux Debug",
  "generator": "Ninja",
  "binaryDir": "${sourceDir}/build/linux-debug",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_CXX_STANDARD": "20"
  },
  "vendor": {
    "microsoft.com/VisualStudioSettings/CMake/1.0": {
      "hostOS": "Linux",
      "intelliSenseMode": "linux-gcc-x64"
    }
  }
}
```

Important notes:

```json
"generator": "Ninja"
```

not:

```json
"generator": "Visual Studio 17 2022"
```

Linux does not use the Visual Studio generator.

Also, do not reuse the exact same build folder for Windows and Linux. This is bad:

```json
"binaryDir": "${sourceDir}/build"
```

This is fine:

```json
"binaryDir": "${sourceDir}/build/linux-debug"
```

---

## 3. Add a Linux build preset

In `buildPresets`, add:

```json
{
  "name": "build-linux-debug",
  "configurePreset": "linux-debug"
}
```

Your Windows presets can stay as-is:

```json
{
  "name": "build-debug",
  "configurePreset": "windows-debug",
  "configuration": "Debug"
}
```

The Linux/Ninja preset does not need `"configuration": "Debug"` because `CMAKE_BUILD_TYPE` already handles that.

---

## 4. Switch Visual Studio to WSL

In Visual Studio’s top CMake toolbar:

```text
Target System: WSL / Ubuntu
Configure Preset: Linux Debug
Build Preset: build-linux-debug
Startup Item: your executable target
```

The preset alone does not move the build to WSL. Visual Studio also needs the **Target System** set to WSL.

---

## 5. Generate and build

Run CMake configure/generate.

Expected signs that you are really building Linux:

```text
Generator: Ninja
```

and compiler paths like:

```text
/usr/bin/c++
/usr/bin/g++
/usr/bin/clang++
```

not:

```text
cl.exe
```

Then build the project.

---

## 6. Find the Linux binary

Linux binaries usually have **no `.exe` extension**.

So this is normal:

```text
main
```

instead of:

```text
main.exe
```

Roadblock we hit: Visual Studio’s WSL build may place the real binary inside its WSL-side `.vs` directory, not necessarily where you expected in the Windows project folder.

From WSL, you can locate it with:

```bash
find ~/.vs -type f -executable -name main
```

or search your project/build area:

```bash
find . -type f -executable
```

Then run it:

```bash
./main
```

---

## 7. Use macros for Windows-only code

For Windows APIs, COM ports, Win32 timeouts, etc., guard them:

```cpp
#ifdef _WIN32
#include <Windows.h>
#endif
```

For platform-specific logic:

```cpp
#ifdef _WIN32
    // Windows COM / Win32 behavior
#else
    // Linux / WSL behavior
#endif
```

Example from our case: Windows used `ComInterface`, but Linux skipped serial communication and only used UDP.

---

## 8. UDP notes for WSL/Linux

For a UDP client/server test running fully inside WSL:

```cpp
std::string ipAddr = "127.0.0.1";
```

is fine.

`127.0.0.1` inside WSL means WSL’s localhost.

If the WSL program needs to talk to a Windows-hosted UDP/TCP listener, then you may need the Windows host IP from inside WSL:

```bash
ip route show | grep -i default | awk '{ print $3 }'
```

Roadblock we hit: the UDP server was hanging because the server-side socket was also calling `connect()`. For the simple UDP server case, the better model is:

```text
server: bind()
client: connect() / send()
```

The server should not call `connect()` unless you intentionally want to filter incoming packets by source endpoint.

---

## 9. Linux socket timeout difference

Windows receive timeout used:

```cpp
DWORD timeout;
setsockopt(... SO_RCVTIMEO ...)
```

Linux uses `timeval`:

```cpp
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/time.h>
#include <cerrno>
#include <cstring>
#endif
```

Linux version:

```cpp
timeval timeout {};
timeout.tv_sec = static_cast<time_t>(timeoutMs / 1000);
timeout.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000) * 1000);

int result = setsockopt(
    UdpSock.native_handle(),
    SOL_SOCKET,
    SO_RCVTIMEO,
    &timeout,
    sizeof(timeout)
);
```

Roadblock we hit: the macro was returning `false` on Linux, so the timeout function was effectively useless until the Linux `timeval` version was added.

---

## 10. IntelliSense red underline issue

Visual Studio may still underline Linux-only includes like:

```cpp
#include <asio.hpp>
```

even when the WSL build succeeds.

That usually means IntelliSense is out of sync or looking from the wrong environment. The compiler result is the source of truth.

Things to check:

```text
Target System: WSL / Ubuntu
Configure Preset: Linux Debug
```

Then regenerate CMake.

---

## 11. Debug the Linux binary from Visual Studio

This worked cleanly.

Set:

```text
Target System: WSL / Ubuntu
Configure Preset: Linux Debug
Build Preset: build-linux-debug
Startup Item: your executable
```

Set a breakpoint in `main.cpp`.

Press:

```text
F5
```

not:

```text
Ctrl + F5
```

`F5` launches the WSL/Linux binary under the debugger, and you can step through it from Visual Studio.

---

# Final minimal workflow

1. Install Linux tools in WSL:

```bash
sudo apt install build-essential cmake gdb make ninja-build rsync zip
```

2. Install Linux dependencies, like:

```bash
sudo apt install libasio-dev
```

3. Add `linux-debug` configure preset using Ninja.

4. Add `build-linux-debug` build preset.

5. In Visual Studio, select:

```text
Target System: WSL / Ubuntu
Configure Preset: Linux Debug
Build Preset: build-linux-debug
```

6. Configure/generate CMake.

7. Build.

8. Run the no-extension Linux binary from WSL or press `F5` in Visual Studio to debug.

9. Guard Windows-only code with:

```cpp
#ifdef _WIN32
#endif
```

10. Treat Linux build errors as real portability issues, not Visual Studio/MSVC issues.
