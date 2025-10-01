This is for me to work on the esp32s3.
I created a docker image to install necessary files for ESP-IDF
The build and run scripts are used to make things faster
I primarily used Zed Editor and its Remote Development feature
Feel free to use othe Text Editor that allows for Dev Containers

# KWF BioAcoustics - Teensy MicroPython Development Environment

This project provides a consistent, Docker-based development environment for the Shaman I project, specifically for working with the Teensy 4.1 and MicroPython.

## Prerequisites

1. **Docker Desktop:** You must have Docker installed and running on your system.
2. **Git:** Required for cloning the repository.
3. **Text Editor with SSH:** A modern text editor like Zed or VS Code (with the Remote - SSH extension) is recommended.

## Quick Start

1. **Build the Docker Image:**

  From the root of this project, run the build script. This only needs to be done once.

    ```bash
    ./build.sh
    ```

2. **Find Your Teensy Device Path:**

  Plug in your Teensy board and find its device name. This is different for each operating system.

    * Linux: Open a terminal and run ``dmesg | grep tty``. Look for a new device like ``/dev/ttyACM0`` or ``/dev/ttyUSB0`` that appears after you plug in the board.

    * macOS: Open a terminal and run ``ls /dev/tty.*``. Look for a device that looks like ``/dev/tty.usbmodem1234567A1``.

    * Windows (using WSL2): You need to "attach" the USB device to your WSL environment.

      + Open PowerShell as an Administrator and run ``winget install --interactive --exact dorssel.usbipd-win``.

      + Run ``usbipd wsl list``. Find the BUSID for the Teensy.

      + Run ``usbipd wsl attach --busid <BUSID>``.

      + Now, in your WSL terminal, run ``dmesg | grep`` tty to find the device name, which will be something like ``/dev/ttyS3``.

3. **Run the Container:**

  Now, run the run.sh script from the project root, passing the device path you just found as an argument.

    * Linux Example:

      ```bash
      ./run.sh /dev/ttyACM0
      ```

    * macOS Example:

      ```bash
      ./run.sh /dev/tty.usbmodem12345
      ```

4. **Connect Your Editor:**

  You can now SSH into the running container.

    * Host: localhost

    * User: root

    * Password: TEENSYdevelopment (from the Dockerfile)

    * Port: 2224

    * The project's workspace directory will be available inside the container at /workspace.
