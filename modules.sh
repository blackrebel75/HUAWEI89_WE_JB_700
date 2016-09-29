#/bin/bash

cp ./kernel/drivers/combo/drv_wlan/mt6628/wlan/wlan_mt6628.ko ./modules
cp ./kernel/drivers/combo/common/mtk_stp_gps.ko ./modules
cp ./kernel/drivers/combo/common/mtk_hif_sdio.ko ./modules
cp ./kernel/drivers/combo/common/mtk_stp_bt.ko ./modules
cp ./kernel/drivers/combo/common/mtk_stp_wmt.ko ./modules
cp ./kernel/drivers/combo/common/mtk_wmt_wifi.ko ./modules
cp ./kernel/drivers/combo/common/mtk_stp_uart.ko ./modules
cp ./kernel/drivers/dual_ccci/ccci.ko ./modules
cp ./kernel/drivers/fmradio/mtk_fm_drv.ko ./modules
cp ./platform/mt6589/kernel/drivers/masp/sec.ko ./modules
cp ./platform/mt6589/kernel/drivers/videocodec/vcodec_kernel_driver.ko ./modules
cp ./platform/mt6589/kernel/drivers/dual_ccci/ccci_plat.ko ./modules
cp ./platform/mt6589/kernel/drivers/devapc/devapc.ko ./modules
cp ./platform/mt6589/kernel/drivers/devinfo/devinfo.ko ./modules
cp ./platform/mt6589/kernel/drivers/gpu/pvr/pvrsrvkm.ko ./modules
cp ./platform/mt6589/kernel/drivers/gpu/pvr/mtklfb.ko ./modules
cp ../crypto/md4.ko ./modules
cp ../drivers/hid/hid-logitech-dj.ko ./modules
cp ../drivers/scsi/scsi_tgt.ko ./modules
cp ../drivers/scsi/scsi_wait_scan.ko ./modules
cp ../fs/cifs/cifs.ko ./modules
mv ./modules ../../../out