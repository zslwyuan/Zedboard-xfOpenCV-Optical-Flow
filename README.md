# Zedboard-xfOpenCV-Optical-Flow
**xfOpenCV Optical Flow implemented on Zedboard with built aarch32 OpenCV libraries**

***

This simple project extends [xfOpenCV](https://github.com/Xilinx/xfopencv) Optical Flow in different ways for various scenarios on Zedboard. 

Xilinx reVision provides many supports for high-end FPGA SoCs like ZCU102, while Zedboard is not supported. Compared to those high-end products, Zedboard is equiped with 32-bit Arm Core and less resource on FPGA. Besides, in the examples provided in xfOpenCV, no video application is presented.

Therefore, we compile the OpenCV 2.4.9 and other required libraries, e.g. ffmpeg, with aarch32 compiler, to support wider usage of Zedboard. Furthermore, we optimize the hardware design of dense optical flow providing in xfOpenCV and the software process to reduce the cost of resource and improve performance. Moreover, we adjust MakeFile for Zedboard.

This project is implemented based on SDSoC 2019.1.

***

**Usage:**

Please note that three versions of optical flow implementation are provided: [for the processing of two images](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_photoinput), [for video file](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_videoinput) and [for webcam streaming input](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_camerainput).

1. enter the corresponding directory:

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_videoinput 

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_photoinput

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_camerainput


2. source /tools/Xilinx/SDx/2019.1/settings64.sh
3. make all -j4
4. Then you can use the built SDSoC design for your purpose. For example, copy all the SD card files, the libraries and data (video/photos) into SD card. And boot the board.

***

**Note**

(1) Please remember to **copy the libraries including opencv and other libs** to /usr/lib/ in you SD card for dynamic linking.

(2) If you want to set **a WebCam for Zedboard** for this application, you can follow the instruction: [Interfacing a USB WebCam and Enable USB Tethering on ZYNQ-7000 AP SoC Running Linux](https://medium.com/developments-and-implementations-on-zynq-7000-ap/interfacing-a-usb-webcam-and-enable-usb-tethering-on-zynq-7000-ap-soc-running-linux-1ba6d836749d). **Important:** The instruction missed some important parts:

(2a) when you do **"petalinux-config -c kernel"**, find the driver for your WebCam in "Device Drivers-->Multimedia support\[y\]-->Media USB Adapters\[y\]-->YOUR WEBCAM\[y\] (e.g. for our system, SONIX JPEG USB Camera Driver)"

(2b) when you do **"petalinux-config -c kernel"**, set up xlnk for your FPGA accelerator:

    i   ) Device Drivers -> Staging drivers (ON) 
    ii  ) Device Drivers -> Staging drivers -> Xilinx APF Accelerator driver (ON)
    iii ) Device Drivers -> Staging drivers -> Xilinx APF Accelerator driver -> Xilinx APF DMA engines support (ON)

(2c) when you do **"petalinux-config -c rootfs"**, add glibc and libstdc++ in "File Packages System / misc"

(2d) **a modified ZED BSP and hdf file are provided** ([here](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/petalinux)), for which you just need to set the webcam driver according to (2a). With this BSP, you don't need to follow the instruction mentioned in (2).

(2e) remember to copy the .so libraries in the directory "/tools/Xilinx/SDK/2019.1/gnu/aarch32/lin/gcc-arm-linux-gnueabi/arm-linux-gnueabihf/libc/lib" to "/lib/arm-linux-gnueabihf" in the root file system of your SD card because the default version of glibc might be 2.27 but SDx compiles our source code with glibc-2.28, which might lead to errors.


***

**DEMO**

***

**INPUT**

![image](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/input.gif)

**OUTPUT**

![image](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/output.gif)

