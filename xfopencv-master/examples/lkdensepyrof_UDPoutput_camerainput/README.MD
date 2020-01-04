Before lauching the example, remember to set:

    export LD_PRELOAD=/usr/lib/libv4l/v4l1compat.so

in the Zedboard linux

In this example, multi-threading is used to keep loading image from the webcam while the FPGA is processing a specific frame.
The FPGA will consume the latest frame instead of some other frames in the webcam buffer.

For this example, the difficulty is to set the webcam driver correctly. You can check the driver in your computer system using dmesg.
