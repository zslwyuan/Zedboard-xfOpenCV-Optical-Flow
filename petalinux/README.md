**An reference command flow:**

1. petalinux-create -t project -s webcam_zed.bsp
2. cd avnet-digilent-zedboard-2019.1/
3. petalinux-config --get-hw-description=`pwd`    (please note the .hdf file is extraxted from ./Xilinx/SDx/2019.1/platforms/zed/sw/prebuilt/)

**if you use webcam_zed.bsp, you don't need the step 4~8.**

4. petalinux-config -c kernel  (refer to the suggestions in the main README.MD file in the root of this repo)
5. petalinux-config -c rootfs  (refer to the suggestions in the main README.MD file in the root of this repo)
6. petalinux-build -c device-tree 

7. copy the following text to ./project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi:

---
    /{
    xlnk {
    compatible = "xlnx,xlnk-1.0";
    };
    };
---


8. copy the following text to ./components/plnx_workspace/device-tree/device-tree/system-top.dts:

---
    &gem0 {
     phy-handle = <&phy0>;
     ps7_ethernet_0_mdio: mdio {
      #address-cells = <1>;
      #size-cells = <0>;
      phy0: phy@7 {
       device_type = "ethernet-phy";
       reg = <7>;
      };
     };
    };/{
     usb_phy0: usb_phy@0 {
      compatible = "ulpi-phy";
      #phy-cells = <0>;
      reg = <0xe0002000 0x1000>;
      view-port = <0x0170>;
      drv-vbus;
     };
    };&usb0 {
     dr_mode = "host";
     usb-phy = <&usb_phy0>;
    };
---

9. fix a bug in petalinux-2019.1: replace "booti" with "bootm" in /avnet-digilent-zedboard-2019.1/project-spec/meta-plnx-generated/recipes-bsp/u-boot/configs/platform-auto.h

10. petalinux-build 

11. petalinux-package --boot --format BIN --fsbl ./images/linux/zynq\_fsbl.elf --fpga ../../xfopencv-master/examples/lkdensepyrof_videoinput/_sds/p0/vivado/vpl/output/system.bit --u-boot ./images/linux/u-boot.elf

12. copy the files in ./image/linux to the FAT32 partition on SD card

13. copy the ./image/linux/rootfs.cpio to the EXT4 partition on SD card

14. go to the EXT4 partition and execute:    sudo pax -r -c -f rootfs.cpio & sync
