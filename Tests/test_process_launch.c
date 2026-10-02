/* Simple test to experiment with launching VirtualT process */

#include <windows.h>
#include <stdio.h>
#include <string.h>

/* Function prototypes for debugging */
void printLastError(const char* msg);
BOOL createProcessTest(const char* exePath, const char* testCase);

void printLastError(const char* msg) {
    DWORD error = GetLastError();
    printf("[ERROR] %s: GetLastError() = %lu (0x%lx)\n", msg, error, error);
}

BOOL createProcessTest(const char* exePath, const char* testCase, const char* workingDirOverride) {
    char commandLine[512];
    char workingDir[512] = "";
    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    BOOL result;
    
    printf("\n=== Test Case: %s ===\n", testCase);
    printf("Executable path: %s\n", exePath);
    
    /* Get working directory */
    if (workingDirOverride && workingDirOverride[0] != '\0') {
        strncpy_s(workingDir, sizeof(workingDir), workingDirOverride, sizeof(workingDir) - 1);
    } else if (workingDirOverride == NULL) {
        /* NULL working directory - let CreateProcess use current directory */
        workingDir[0] = '\0';
    } else {
        /* Extract directory from exePath */
        char exeDir[512];
        strncpy_s(exeDir, sizeof(exeDir), exePath, sizeof(exeDir) - 1);
        char* lastSlash = strrchr(exeDir, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
            strncpy_s(workingDir, sizeof(workingDir), exeDir, sizeof(workingDir) - 1);
        } else {
            GetCurrentDirectoryA(sizeof(workingDir), workingDir);
        }
    }
    
    if (workingDir[0] == '\0') {
        printf("Working directory: NULL (using current directory)\n");
    } else {
        printf("Working directory: %s\n", workingDir);
    }
    
    /* Build command line */
    snprintf(commandLine, sizeof(commandLine), "\"%s\" -p 6166", exePath);
    printf("Command line: %s\n", commandLine);
    
    /* Initialize STARTUPINFO */
    memset(&startupInfo, 0, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    
    /* Attempt to create process */
    result = CreateProcessA(
        NULL,           /* lpApplicationName (NULL = use command line) */
        commandLine,    /* lpCommandLine */
        NULL,           /* lpProcessAttributes */
        NULL,           /* lpThreadAttributes */
        FALSE,          /* bInheritHandles */
        0,              /* dwCreationFlags */
        NULL,           /* lpEnvironment */
        workingDir[0] ? workingDir : NULL,     /* lpCurrentDirectory */
        &startupInfo,   /* lpStartupInfo */
        &processInfo    /* lpProcessInformation */
    );
    
    if (!result) {
        printLastError("CreateProcessA failed");
        return FALSE;
    }
    
    printf("Process created successfully!\n");
    printf("Process handle: %p\n", processInfo.hProcess);
    printf("Process ID: %lu\n", processInfo.dwProcessId);
    
    /* Wait a moment for process to fully start */
    printf("Waiting 2 seconds for process to initialize...\n");
    Sleep(2000);
    
    /* Check if process is still running */
    DWORD exitCode;
    BOOL gotExitCode = GetExitCodeProcess(processInfo.hProcess, &exitCode);
    
    if (!gotExitCode) {
        printLastError("GetExitCodeProcess failed");
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return FALSE;
    }
    
    if (exitCode == STILL_ACTIVE) {
        printf("Process is RUNNING (exit code = STILL_ACTIVE)\n");
    } else {
        printf("Process has TERMINATED (exit code = %lu)\n", exitCode);
    }
    
    /* Clean up */
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    
    return (exitCode == STILL_ACTIVE);
}

int main(int argc, char** argv) {
    BOOL result1, result2;
    
    printf("VirtualT Process Launch Test\n");
    printf("============================\n");
    
    /* Case 1: Try with correct VirtualT path (using environment variable) */
    const char* vtPath = getenv("VIRTUALT_PATH");
    if (vtPath == NULL) {
        printf("\n[VIRTUALT_PATH environment variable not set]\n");
        printf("Please set VIRTUALT_PATH to the path of VirtualT.exe\n");
        printf("Example: set VIRTUALT_PATH=C:\\Program Files\\VirtualT\\virtualt.exe\n");
        return 1;
    }
    
    printf("\nVIRTUALT_PATH = %s\n", vtPath);
    
    /* Check if the file exists */
    DWORD fileAttr = GetFileAttributesA(vtPath);
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {
        printf("\n[WARNING] VirtualT executable not found at specified path!\n");
        printLastError("GetFileAttributesA");
    } else {
        printf("VirtualT executable found at specified path\n");
    }
    
    /* Test Case 1: Correct path */
    result1 = createProcessTest(vtPath, "Correct Path (VirtualT.exe)", NULL);
    
    /* Test Case 1b: With NULL working directory (reproduces the bug) */
    char* nullWorkingDir = "";
    createProcessTest(vtPath, "With NULL working directory", nullWorkingDir);
    
    /* Test Case 2: Incorrect path (simulate by modifying the last part) */
    char badPath[512];
    strncpy_s(badPath, sizeof(badPath), vtPath, sizeof(badPath) - 1);
    
    /* Find the last backslash and replace the filename */
    char* lastSlash = strrchr(badPath, '\\');
    if (lastSlash) {
        /* Calculate remaining buffer space after the slash */
        size_t remaining = sizeof(badPath) - (lastSlash - badPath) - 1;
        strcpy_s(lastSlash + 1, remaining, "NotVirtualT.exe");
    }
    
    result2 = createProcessTest(badPath, "Incorrect Path (NotVirtualT.exe)", NULL);
    
    /* Summary */
    printf("\n\n=== SUMMARY ===\n");
    printf("Test 1 (Correct path):  %s\n", result1 ? "PASSED - Process is running" : "FAILED - Process not running or failed to launch");
    printf("Test 2 (Incorrect path): %s\n", result2 ? "PASSED - Process is running" : "FAILED - Process not running or failed to launch");
    
    if (!result1 && !result2) {
        printf("\n[NOTE] Both tests failed - check error messages above\n");
        printf("[NOTE] Error info: %s\n", vtPath);
    }
    
    return (result1 ? 0 : 1);
}