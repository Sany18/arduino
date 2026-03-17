#!/bin/bash

# Build and upload to ESP32-S3
echo "Building and uploading to ESP32-S3..."
pio run -t upload

# Check if upload was successful
if [ $? -eq 0 ]; then
    echo "Upload successful!"
    
    # Ask if user wants to open serial monitor
    read -p "Open serial monitor? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        pio device monitor
    fi
else
    echo "Upload failed!"
    exit 1
fi
