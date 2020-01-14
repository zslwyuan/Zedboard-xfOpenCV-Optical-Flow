# Zedboard-xfOpenCV-Optical-Flow
**xfOpenCV Optical Flow implemented on Zedboard with built aarch32 OpenCV libraries**

***

This simple project extends [xfOpenCV](https://github.com/Xilinx/xfopencv) Optical Flow in different ways for various scenarios on Zedboard. 

Xilinx reVision provides many supports for high-end FPGA SoCs like ZCU102, while Zedboard is not supported. Compared to those high-end products, Zedboard is equiped with 32-bit Arm Core and less resource on FPGA. Besides, in the examples provided in xfOpenCV, no video application is presented.

Therefore, we compile the [OpenCV 2.4.9](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/arm_opencv_249) and [other required libraries](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/arm_other_libs), e.g. ffmpeg, with aarch32 compiler, to support wider usage of Zedboard. Furthermore, we optimize the hardware design of dense optical flow providing in xfOpenCV and the software process to reduce the cost of resource and improve performance. Moreover, we adjust MakeFile for Zedboard.

This project is implemented based on SDSoC 2019.1. For these open source projects, we provide 0.5FPS~5FPS version. We also implemented a commercial version which can run at 23~24fps with streaming output via TCP and will be open to community later.


**INPUT:** <img src="https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/input.gif" data-canonical-src="https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/input.gif" width="200" /> **OUTPUT:** <img src="https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/output.gif" data-canonical-src="https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/demo_photos/input.gif" width="200" />


***

**Usage:**

Please note that three versions of optical flow implementation are provided: [for the processing of two images](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_photoinput), [for video file](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_videoinput), [for webcam streaming input](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_camerainput) and [for UDP Ethernet output with webcam streaming input](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/tree/master/xfopencv-master/examples/lkdensepyrof_UDPoutput_camerainput) .

1. enter the corresponding directory:

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_videoinput 

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_photoinput

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_camerainput

    cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof_UDPoutput_camerainput


2. source /tools/Xilinx/SDx/2019.1/settings64.sh
3. make all -j4
4. If you want to try the examples with video/webcam, you have to build the petalinux for video/webcam libraries and device drivers according the note below. Then you can use the built SDSoC design for your purpose. For example, copy all the SD card files, the libraries and data (video/photos) into SD card. And boot the board.
5. the system can boot via a command "boot" in serial terminal (braudrate=115200). To login, the system account is "root" and the password is "root".
***

**Note**

(1) Please remember to **copy the libraries including opencv and other libs** and **libsds_lib.so** (this file is in /Xilinx/SDx/2019.1/target/aarch32-linux/lib) to /usr/lib/ or /lib in you SD card for dynamic linking.

(2) If you want to set **a WebCam for Zedboard** for this application, you can follow the instruction: [Interfacing a USB WebCam and Enable USB Tethering on ZYNQ-7000 AP SoC Running Linux](https://medium.com/developments-and-implementations-on-zynq-7000-ap/interfacing-a-usb-webcam-and-enable-usb-tethering-on-zynq-7000-ap-soc-running-linux-1ba6d836749d). **Important:** The instruction missed some important parts:

(2a) when you do **"petalinux-config -c kernel"**, find the driver for your WebCam in "Device Drivers-->Multimedia support\[y\]-->Media USB Adapters\[y\]-->YOUR WEBCAM\[y\] (e.g. for our system, SONIX JPEG USB Camera Driver)"

(2b) when you do **"petalinux-config -c kernel"**, set up xlnk for your FPGA accelerator:

    i   ) Device Drivers -> Staging drivers (ON) 
    ii  ) Device Drivers -> Staging drivers -> Xilinx APF Accelerator driver (ON)
    iii ) Device Drivers -> Staging drivers -> Xilinx APF Accelerator driver -> Xilinx APF DMA engines support (ON)

(2c) when you do **"petalinux-config -c rootfs"**, add glibc and libstdc++ in "File Packages System / misc"

(2d) **a modified ZED BSP and hdf file are provided** ([here](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/petalinux)), for which you just need to set the webcam driver according to (2a). With this BSP, you don't need to follow the instruction mentioned in (2). [A more detailed guide](https://github.com/zslwyuan/Zedboard-xfOpenCV-Optical-Flow/blob/master/petalinux/README.md) is provided.
