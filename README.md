# Zedboard-xfOpenCV-Optical-Flow
**xfOpenCV Optical Flow implemented on Zedboard with built aarch32 OpenCV libraries**

***

This simple project implements xfOpenCV Optical Flow on Zedboard. 

Xilinx reVision provides many supports for high-end FPGA SoCs like ZCU102, while Zedboard is not supported. Compared to those high-end products, Zedboard is equiped with 32-bit Arm Core and less resource on FPGA. Besides, in the examples provided in xfOpenCV, no video application is presented.

Therefore, we compile the OpenCV 2.4.9 and other required libraries, e.g. ffmpeg, with aarch32 compiler, to support wider usage of Zedboard. Furthermore, we optimize the hardware design of dense optical flow providing in xfOpenCV and the software process to reduce the cost of resource and improve performance. Moreover, we adjust MakeFile for Zedboard.

This project is implemented based on SDSoC 2019.1.

***

**Usage:**

Please note that two versions of optical flow implementation are provided: one is for the processing of two images and the other one is for video.

1. cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof\_videoinput **or** cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_photoinput
2. source /tools/Xilinx/SDx/2019.1/settings64.sh
3. make all -j4
4. Then you can use the built SDSoC design for your purpose. For example, copy all the SD card files, the libraries and data (video/photos) into SD card. And boot the board.

***

**Note**

(1) Please remember to copy the libraries to you SD card for dynamic linking.
(2) If you want to set a WebCam for Zedboard for this application, you can follow the instruction: [Interfacing a USB WebCam and Enable USB Tethering on ZYNQ-7000 AP SoC Running Linux](https://medium.com/developments-and-implementations-on-zynq-7000-ap/interfacing-a-usb-webcam-and-enable-usb-tethering-on-zynq-7000-ap-soc-running-linux-1ba6d836749d). **Important:** The instruction missed some important parts:

(2a) when do "petalinux-config -c kernel", find the driver for your WebCam in "Device Drivers-->Multimedia support\[y\]-->Media USB Adapters\[y\]-->YOUR WEBCAM\[y\] (e.g. our SONIX JPEG USB Camera Driver)"

(2b) when do "petalinux-config -c kernel", set up xlnk for your FPGA accelerator:

    • Device Drivers → Staging drivers (ON) 
    • Device Drivers → Staging drivers → Xilinx APF Accelerator driver (ON)
    • Device Drivers → Staging drivers → Xilinx APF Accelerator driver → Xilinx APF DMA engines support (ON)

(2c) when do "petalinux-config -c rootfs", add glibc and libstdc++

(2d) a modified ZED BSP is provided (webcam_zed.bsp), for which you just need to set the webcam driver according to (2a).

***

**DEMO**

***

**INPUT**

![image](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/input.gif)

**OUTPUT**

![image](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/output.gif)

