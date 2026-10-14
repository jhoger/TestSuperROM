#!/bin/bash
# Test script to launch VirtualT with socket interface for testing

VIRTUALT_PATH="/home/jhoger/projects/virtualt/virtualt"
SUT_DIR="/home/jhoger/projects/modelt/TestSuperROM/SUT"
SUPERROM="$SUT_DIR/SUPNEC.bin"

# Check if VirtualT exists
if [ ! -x "$VIRTUALT_PATH" ]; then
    echo "ERROR: VirtualT not found at $VIRTUALT_PATH"
    exit 1
fi

# Check if SuperROM exists
if [ ! -f "$SUPERROM" ]; then
    echo "ERROR: SuperROM not found at $SUPERROM"
    exit 1
fi

echo "Launching VirtualT with socket interface..."
echo "VirtualT: $VIRTUALT_PATH"
echo "SuperROM: $SUPERROM"
echo ""

# Launch VirtualT in background with socket interface
# Using -n for headless mode and -p for socket port
"$VIRTUALT_PATH" -n -p 6166 &
VT_PID=$!

echo "VirtualT PID: $VT_PID"

# Wait for VirtualT to start
echo "Waiting for VirtualT to initialize..."
sleep 2

# Check if process is still running
if ps -p $VT_PID > /dev/null 2>&1; then
    echo "VirtualT is running"
    
    # Try to connect with vt_client
    VTC_CLIENT="/home/jhoger/projects/virtualt/vt_client"
    if [ -x "$VTC_CLIENT" ]; then
        echo ""
        echo "Testing socket connection with vt_client..."
        timeout 3 "$VTC_CLIENT" 6166 status 2>&1 || true
    else
        echo "vt_client not found at $VTC_CLIENT (optional)"
    fi
    
    echo ""
    echo "VirtualT is running. Press Ctrl+C to stop."
    echo "To test manually, run: $VTC_CLIENT 6166"
    
    # Keep running until interrupted
    wait $VT_PID
else
    echo "ERROR: VirtualT failed to start"
    exit 1
fi
