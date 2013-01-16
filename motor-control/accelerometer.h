/*
 * accelerometer.h
 *
 *  Created on: Jan 11, 2013
 *      Author: eal
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_


#define ACCEL_TWI_ADDRESS							0x1c


#define ACCEL_STATUS								0x00
#define ACCEL_OUT_X_MSB								0x01
#define ACCEL_OUT_X_LSB								0x02
#define ACCEL_OUT_Y_MSB								0x03
#define ACCEL_OUT_Y_LSB								0x04
#define ACCEL_OUT_Z_MSB								0x05
#define ACCEL_OUT_Z_LSB								0x06
#define ACCEL_SYSMOD								0x0b
#define ACCEL_INT_SOURCE							0x0c
#define ACCEL_WHO_AM_I								0x0d
#define ACCEL_XYZ_DATA_CFG							0x0e
#define ACCEL_HP_FILTER_CUTOFF						0x0f
#define ACCEL_PL_STATUS								0x10
#define ACCEL_PL_CFG								0x11
#define ACCEL_PL_COUNT								0x12
#define ACCEL_PL_BF_ZCOMP							0x13
#define ACCEL_P_L_THS_REG							0x14
#define ACCEL_FF_MT_CFG								0x15
#define ACCEL_FF_MT_SRC								0x16
#define ACCEL_FF_MT_THS								0x17
#define ACCEL_FF_MT_COUNT							0x18
#define ACCEL_TRANSIENT_CFG							0x1d
#define ACCEL_TRANSIENT_SRC							0x1e
#define ACCEL_TRANSIENT_THS							0x1f
#define ACCEL_TRANSIENT_COUNT						0x20
#define ACCEL_PULSE_CFG								0x21
#define ACCEL_PULSE_SRC								0x22
#define ACCEL_PULSE_THSX							0x23
#define ACCEL_PULSE_THSY							0x24
#define ACCEL_PULSE_THSZ							0x25
#define ACCEL_PULSE_TMLT							0x26
#define ACCEL_PULSE_LTCY							0x27
#define ACCEL_PULSE_WIND							0x28
#define ACCEL_ASLP_COUNT							0x29
#define ACCEL_CTRL_REG1								0x2a
#define ACCEL_CTRL_REG2								0x2b
#define ACCEL_CTRL_REG3								0x2c
#define ACCEL_CTRL_REG4								0x2d
#define ACCEL_CTRL_REG5								0x2e
#define ACCEL_OFF_X									0x2f
#define ACCEL_OFF_Y									0x30
#define ACCEL_OFF_Z									0x31

#define ACCEL_STATUS_ZYXOW_bm						(1<<7)
#define ACCEL_STATUS_ZOW_bm							(1<<6)
#define ACCEL_STATUS_YOW_bm							(1<<5)
#define ACCEL_STATUS_XOW_bm							(1<<4)
#define ACCEL_STATUS_ZYXDR_bm						(1<<3)
#define ACCEL_STATUS_ZDR_bm							(1<<2)
#define ACCEL_STATUS_YDR_bm							(1<<1)
#define ACCEL_STATUS_XDR_bm							(1<<0)

#define ACCEL_SYSMOD_STANDBY_bm						(0x00)
#define ACCEL_SYSMOD_WAKE_bm						(0x01)
#define ACCEL_SYSMOD_SLEEP_bm						(0x02)

#define ACCEL_INT_SOURCE_SRC_ASLP_bm				(1<<7)
#define ACCEL_INT_SOURCE_SRC_TRANS_bm				(1<<5)
#define ACCEL_INT_SOURCE_SRC_LNDPRT_bm				(1<<4)
#define ACCEL_INT_SOURCE_SRC_PULSE_bm				(1<<3)
#define ACCEL_INT_SOURCE_SRC_FF_MT_bm				(1<<2)
#define ACCEL_INT_SOURCE_SRC_DRDY_bm				(1<<0)

#define ACCEL_XYZ_DATA_CFG_HFP_OUT_bm				(1<<4)
#define ACCEL_XYZ_DATA_CFG_FS_2G_bm					(0x00)
#define ACCEL_XYZ_DATA_CFG_FS_4G_bm					(0x01)
#define ACCEL_XYZ_DATA_CFG_FS_8G_bm					(0x02)

#define ACCEL_HP_FILTER_CUTOFF_PULSE_HPF_BYP_bm		(1<<5)
#define ACCEL_HP_FILTER_CUTOFF_PULSE_LPF_EN_bm		(1<<4)
#define ACCEL_HP_FILTER_CUTOFF_SEL1_bm				(1<<1)
#define ACCEL_HP_FILTER_CUTOFF_SEL0_bm				(1<<0)

#define ACCEL_PL_STATUS_NEWLP_bm					(1<<7)
#define ACCEL_PL_STATUS_LO_bm						(1<<6)
#define ACCEL_PL_STATUS_LAPO_PORTRAIT_UP_bm			(0<<2)
#define ACCEL_PL_STATUS_LAPO_PORTRAIT_DOWN_bm		(1<<2)
#define ACCEL_PL_STATUS_LAPO_LANDSCAPE_RIGHT_bm		(2<<2)
#define ACCEL_PL_STATUS_LAPO_LANDSCAPE_LEFT_bm		(3<<2)
#define ACCEL_PL_STATUS_BAFRO_bm					(1<<0)

#define ACCEL_PL_CFG_DBCNTM_bm						(1<<7)
#define ACCEL_PL_CFG_PL_EN_bm						(1<<6)

#define ACCEL_FF_MT_CFG_ELE_bm						(1<<7)
#define ACCEL_FF_MT_CFG_OAE_bm						(1<<6)
#define ACCEL_FF_MT_CFG_ZEFE_bm						(1<<5)
#define ACCEL_FF_MT_CFG_YEFE_bm						(1<<4)
#define ACCEL_FF_MT_CFG_XEFE_bm						(1<<3)

#define ACCEL_FF_MT_SRC_EA_bm						(1<<7)
#define ACCEL_FF_MT_SRC_ZHE_bm						(1<<5)
#define ACCEL_FF_MT_SRC_ZHP_bm						(1<<4)
#define ACCEL_FF_MT_SRC_YHE_bm						(1<<3)
#define ACCEL_FF_MT_SRC_YHP_bm						(1<<2)
#define ACCEL_FF_MT_SRC_XHE_bm						(1<<1)
#define ACCEL_FF_MT_SRC_XHP_bm						(1<<0)

#define ACCEL_FF_MT_THS_DBCNTM_bm					(1<<7)

#define ACCEL_TRANSIENT_CFG_ELE_bm					(1<<4)
#define ACCEL_TRANSIENT_CFG_ZTEFE_bm				(1<<3)
#define ACCEL_TRANSIENT_CFG_YTEFE_bm				(1<<2)
#define ACCEL_TRANSIENT_CFG_XTEFE_bm				(1<<1)
#define ACCEL_TRANSIENT_CFG_HPF_BYP_bm				(1<<0)

#define ACCEL_TRANSIENT_SRC_EA_bm					(1<<6)
#define ACCEL_TRANSIENT_SRC_ZTRANSE_bm				(1<<5)
#define ACCEL_TRANSIENT_SRC_Z_TRANS_POL_bm			(1<<4)
#define ACCEL_TRANSIENT_SRC_YTRANSE_bm				(1<<3)
#define ACCEL_TRANSIENT_SRC_Y_TRANS_POL_bm			(1<<2)
#define ACCEL_TRANSIENT_SRC_XTRANSE_bm				(1<<1)
#define ACCEL_TRANSIENT_SRC_X_TRANS_POL_bm			(1<<0)

#define ACCEL_TRANSIENT_THS_DBCNTM_bm				(1<<7)

#define ACCEL_PULSE_CFG_DPA_bm						(1<<7)
#define ACCEL_PULSE_CFG_ELE_bm						(1<<6)
#define ACCEL_PULSE_CFG_ZDPEFE_bm					(1<<5)
#define ACCEL_PULSE_CFG_ZSPEFE_bm					(1<<4)
#define ACCEL_PULSE_CFG_YDPEFE_bm					(1<<3)
#define ACCEL_PULSE_CFG_YSPEFE_bm					(1<<2)
#define ACCEL_PULSE_CFG_XDPEFE_bm					(1<<1)
#define ACCEL_PULSE_CFG_XSPEFE_bm					(1<<0)

#define ACCEL_PULSE_SRC_EA_bm						(1<<7)
#define ACCEL_PULSE_SRC_AXZ_bm						(1<<6)
#define ACCEL_PULSE_SRC_AXY_bm						(1<<5)
#define ACCEL_PULSE_SRC_AXX_bm						(1<<4)
#define ACCEL_PULSE_SRC_DPE_bm						(1<<3)
#define ACCEL_PULSE_SRC_POIZ_bm						(1<<2)
#define ACCEL_PULSE_SRC_POIY_bm						(1<<1)
#define ACCEL_PULSE_SRC_POIX_bm						(1<<0)

#define ACCEL_CTRL_REG1_ASLP_RATE_50HZ_bm			(0<<6)
#define ACCEL_CTRL_REG1_ASLP_RATE_12_5HZ_bm			(1<<6)
#define ACCEL_CTRL_REG1_ASLP_RATE_6_25HZ_bm			(2<<6)
#define ACCEL_CTRL_REG1_ASLP_RATE_1_56HZ_bm			(3<<6)
#define ACCEL_CTRL_REG1_DR_800HZ_bm					(0<<3)
#define ACCEL_CTRL_REG1_DR_400HZ_bm					(1<<3)
#define ACCEL_CTRL_REG1_DR_200HZ_bm					(2<<3)
#define ACCEL_CTRL_REG1_DR_100HZ_bm					(3<<3)
#define ACCEL_CTRL_REG1_DR_50HZ_bm					(4<<3)
#define ACCEL_CTRL_REG1_DR_12_5HZ_bm				(5<<3)
#define ACCEL_CTRL_REG1_DR_6_25HZ_bm				(6<<3)
#define ACCEL_CTRL_REG1_DR_1_56HZ_bm				(7<<3)
#define ACCEL_CTRL_REG1_LNOISE_bm					(1<<2)
#define ACCEL_CTRL_REG1_F_READ_bm					(1<<1)
#define ACCEL_CTRL_REG1_ACTIVE_bm					(1<<0)

#define ACCEL_CTRL_REG2_ST_bm						(1<<7)
#define ACCEL_CTRL_REG2_RST_bm						(1<<6)
#define ACCEL_CTRL_REG2_SMODS_NORMAL_bm				(0<<3)
#define ACCEL_CTRL_REG2_SMODS_LOW_NOISE_POWER_bm	(1<<3)
#define ACCEL_CTRL_REG2_SMODS_HIGH_RESOLUTION_bm	(2<<3)
#define ACCEL_CTRL_REG2_SMODS_LOW_POWER_bm			(3<<3)
#define ACCEL_CTRL_REG2_SPLE_bm						(1<<2)
#define ACCEL_CTRL_REG2_MODS_NORMAL_bm				(0<<0)
#define ACCEL_CTRL_REG2_MODS_LOW_NOISE_POWER_bm		(1<<0)
#define ACCEL_CTRL_REG2_MODS_HIGH_RESOLUTION		(2<<0)
#define ACCEL_CTRL_REG2_MODS_LOW_POWER_bm			(3<<0)

#define ACCEL_CTRL_REG3_WAKE_TRANS_bm				(1<<6)
#define ACCEL_CTRL_REG3_WAKE_LNDPRT_bm				(1<<5)
#define ACCEL_CTRL_REG3_WAKE_PULSE_bm				(1<<4)
#define ACCEL_CTRL_REG3_WAKE_FF_MT_bm				(1<<3)
#define ACCEL_CTRL_REG3_IPOL_bm						(1<<1)
#define ACCEL_CTRL_REG3_PP_OD_bm					(1<<0)

#define ACCEL_CTRL_REG4_INT_EN_ASLP_bm				(1<<7)
#define ACCEL_CTRL_REG4_INT_EN_TRANS_bm				(1<<5)
#define ACCEL_CTRL_REG4_INT_EN_LNDPRT_bm			(1<<4)
#define ACCEL_CTRL_REG4_INT_EN_PULSE_bm				(1<<3)
#define ACCEL_CTRL_REG4_INT_EN_FF_MT_bm				(1<<2)
#define ACCEL_CTRL_REG4_INT_EN_DRDY_bm				(1<<0)

#define ACCEL_CTRL_REG5_INT_CFG_ASLP_bm				(1<<7)
#define ACCEL_CTRL_REG5_INT_CFG_TRANS_bm			(1<<5)
#define ACCEL_CTRL_REG5_INT_CFG_LNDPRT_bm			(1<<4)
#define ACCEL_CTRL_REG5_INT_CFG_PULSE_bm			(1<<3)
#define ACCEL_CTRL_REG5_INT_CFG_FF_MT_bm			(1<<2)
#define ACCEL_CTRL_REG5_INT_CFG_DRDY_bm				(1<<0)


#endif /* ACCELEROMETER_H_ */
