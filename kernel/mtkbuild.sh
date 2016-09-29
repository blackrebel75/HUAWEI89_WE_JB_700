export CROSS_COMPILE=~/android/toolchains/arm-eabi-4.7/bin/arm-eabi-
TARGET_PRODUCT=huawei89_cmcc_jb2 TARGET_BUILD_VARIANT=user MTK_ROOT_CUSTOM=../mediatek/custom/ make -j4

../mediatek/build/tools/mkimage arch/arm/boot/zImage KERNEL > out/huawei89_cmcc_jb2/zImage_p

#Modules part
make TARGET_PRODUCT=huawei89_cmcc_jb2 TARGET_BUILD_VARIANT=user INSTALL_MOD_STRIP=1 INSTALL_MOD_PATH=./out/huawei89_cmcc_jb2/system INSTALL_MOD_DIR=./out/huawei89_cmcc_jb2/system android_modules_install

#Repack part - You need to set PATH var correctly poiting to a directory with https://github.com/bgcngm/mtk-tools.git and chmod +x mkbootimg
#You need ramdisk directory too

