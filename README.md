# Zedboard-xfOpenCV-Optical-Flow
**xfOpenCV Optical Flow implemented on Zedboard with built aarch32 OpenCV libraries**

***

This project, provided by **Direct Drive Joint Inc.**, implements xfOpenCV Optical Flow on Zedboard. 

Xilinx reVision provides many supports for high-end FPGA SoCs like ZCU102, while Zedboard is not supported.
Compared to those high-end products, Zedboard is equiped with 32-bit Arm Core and less resource on FPGA.
Therefore, engineers in **Direct Drive Joint Inc.** compile the OpenCV 2.4.9 and other required libraries, e.g. ffmpeg, with aarch32 compiler, modify the original design of dense optical flow providing in xfOpenCV to reduce the cost of resource and adjust MakeFile for Zedboard.

This project is implemented based on SDSoC 2019.1.

***

**Contributors:**

Yikang ZHANG

Tingyuan LIANG

***

**Usage:**

1. cd Zedboard-xfOpenCV-Optical-Flow/xfopencv-master/examples/lkdensepyrof
2. source /tools/Xilinx/SDx/2019.1/settings64.sh
3. make all -j4
4. Then you can use the built SDSoC design for your purpose.
