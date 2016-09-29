#/bin/bash
rm -R ./out/modules/*
cp ./kernel/out/mediatek/kernel/drivers/combo/drv_wlan/mt6628/wlan/wlan_mt6628.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_stp_gps.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_hif_sdio.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_stp_bt.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_stp_wmt.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_wmt_wifi.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/combo/common/mtk_stp_uart.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/dual_ccci/ccci.ko ./out/modules
cp ./kernel/out/mediatek/kernel/drivers/fmradio/mtk_fm_drv.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/masp/sec.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/videocodec/vcodec_kernel_driver.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/dual_ccci/ccci_plat.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/devapc/devapc.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/devinfo/devinfo.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/gpu/pvr/pvrsrvkm.ko ./out/modules
cp ./kernel/out/mediatek/platform/mt6589/kernel/drivers/gpu/pvr/mtklfb.ko ./out/modules
cp ./kernel/out/crypto/md4.ko ./out/modules
cp ./kernel/out/drivers/hid/hid-logitech-dj.ko ./out/modules
cp ./kernel/out/drivers/scsi/scsi_tgt.ko ./out/modules
cp ./kernel/out/drivers/scsi/scsi_wait_scan.ko ./out/modules
cp ./kernel/out/fs/cifs/cifs.ko ./out/modules
cp ./kernel/out/kernel_huawei89_cmcc_jb2.bin ./out/zImage
./mtk-tools/repack-MTK.pl -boot --debug ./out/zImage ./mtk-tools/cm12-ramdisk/ ./out/boot.img
cp ./out/modules/* ./bootimg/system/lib/modules
cp ./out/boot.img ./bootimg
cd ./bootimg
zip -r -0 "kernel-update.zip" ./*
mv kernel-update.zip ../
