# Tools - Unity Test Framework Setup

This directory contains test utilities and the Unity test framework.

## Downloading Unity Framework

The Unity test framework is required to build and run tests. If the `Tools/unity/` directory is missing or empty, download it using one of these methods:

### Option 1: Download Latest Release (Recommended)

Download the latest stable release from:
```
https://github.com/ThrowTheSwitch/Unity/releases/latest
```

Download `unity-master.zip`, extract it, and place the contents in `Tools/unity/`.

### Option 2: Using git (Development)

```bash
cd Tools
git clone https://github.com/ThrowTheSwitch/Unity.git unity
```

### Option 3: Using wget/curl (Linux/macOS)

```bash
cd Tools
wget https://github.com/ThrowTheSwitch/Unity/archive/refs/heads/master.zip -O unity.zip
unzip unity.zip
mv Unity-* unity
rm unity.zip
```

### Option 4: Using PowerShell (Windows)

```powershell
cd Tools
Invoke-WebRequest -Uri "https://github.com/ThrowTheSwitch/Unity/archive/refs/heads/master.zip" -OutFile "unity.zip"
Expand-Archive -Path unity.zip -DestinationPath .
Move-Item -Path "Unity-master" -Destination "unity"
Remove-Item "unity.zip"
```

## Verifying the Setup

After downloading, the structure should look like:

```
Tools/
└── unity/
    ├── src/
    │   ├── unity.c
    │   └── unity.h
    ├── auto/
    └── ...
```

## Building Tests

Once Unity is in place:

```bash
mkdir build
cd build
cmake ..
cmake --build .
./vt_process_tests
```

## Troubleshooting

- **CMake error: "Unity not found"** - Ensure `Tools/unity/src/unity.c` exists
- **Linker errors** - Re-run cmake after downloading Unity
- **Missing unity.h** - Make sure you downloaded the complete repository, not just a single file
