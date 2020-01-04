Before lauching the example, remember to set:

    export LD_PRELOAD=/usr/lib/libv4l/v4l1compat.so

in the Zedboard linux

In this example, multi-threading is used to keep loading image from the webcam while the FPGA is processing a specific frame.
The FPGA will consume the latest frame instead of some other frames in the webcam buffer.

MOREOVER, in the image processing thread, the output image will be compressed with JPEG and sent to host server via UDP protocol. You need to input the IPV6 address of the server. The host server can receive, decode and display the image based on [host_server.cpp](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_UDPoutput_camerainput/host_server.cpp).
