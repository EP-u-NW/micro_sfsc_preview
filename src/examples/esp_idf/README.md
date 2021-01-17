We are using the [platformIO](https://platformio.org/) build system with the official [esp-idf](https://github.com/espressif/esp-idf) as framework.
To use this build system, move every non-code file (excluding this README) from this folder to the repository's root.
To configure the platformIO build system, take a look a the `platformio.ini`.
A recommended IDE for platformIO is VSCode, if you use it, just open the `platformio.ini` with it and you should be ready to go.

There are two configuration changes to the default `sdkconfig` file of the esp-idf, which are already set up in this example `sdkconfig` file:
+ **CONFIG_LWIP_SO_RCVBUF**: We need this option to use non-blocking WIFI I/O.
+ **CONFIG_LWIP_TCPIP_TASK_AFFINITY_CPU1**: We want to use both cores of the ESP32, so we use the LWIP stack on core1 while our application will run on core0. This was chosen arbitrarily and there is no need for both tasks running on different cores, the examples should just demonstrate that this is possible, and use the resources of the ESP32 optimally.

You can edit the `sdkconfig` using the `idf.py menuconfig` command from the esp-idf. If you get an CMake version error, edit the generated CMakeLists.txt to require a lower version.

When compiling using platformIO on Windows, it can happen that linking will fail due to a command line overflow: The Windows cmd.exe has a limit on how many characters there can be in a single command, but linking command is too long, because of the many files that need to be linked. A workaround is using Ubuntu with the [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/de-de/windows/wsl/install-win10), install platformIO there, and build it there (since the Ubuntus bash doesn't have a this strict length limit). There is a bat file for building, see `build_pio_esp_idf_wsl.bat`. Then, you can flash the compiled firmware without platformIO, but with the plain esp-idf on Windows using `flash_pio_esp_idf.bat`. This requires a separate copy of the esp-idf (separate to the one included in platformIO) set up according to the esp-idf install guide.