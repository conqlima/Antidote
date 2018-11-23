#ifndef BASIC_ECG_H_
#define BASIC_ECG_H_

#include <communication/stdconfigurations.h>

/* basic ECG nomenclature */
/* The format used here follows that of ISO/IEEE 11073-10101:2004 [B6]. */
/**From Object Infrastructure (MDC_PART_OBJ) *********/
#define MDC_ATTR_TICK_RES 2693 /* */

/** From Medical Supervisory Control and Data Acquisition (MDC_PART_SCADA) *******/
#define MDC_ECG_TIME_PD_RR_GL 16168 /* */

/** From Dimensions (MDC_PART_DIM) ******/ 
#define MDC_DIM_TICK 6848 /* */
#define MDC_DIM_MILLI_VOLT 4274 /* mV */

/** From Communication Infrastructure (MDC_PART_INFRA) *******/ 
#define MDC_DEV_SPEC_PROFILE_ECG 4102 /* */

/* 4236 through 4243 used for IEEE Std 11073-10406 (Basic ECG)*/
#define MDC_DEV_SUB_SPEC_PROFILE_ECG 4236 /* */
#define MDC_DEV_SUB_SPEC_PROFILE_HR 4237 /* */

/** From Personal Health Device Disease Management (MDC_PART_PHD_DM) **************************************/ 
#define MDC_ECG_DEV_STAT 21976 /**/
#define MDC_ECG_EVT_CTXT_GEN 21977 /**/
#define MDC_ECG_EVT_CTXT_USER 21978 /**/
#define MDC_ECG_EVT_CTXT_PERIODIC 21979 /**/
#define MDC_ECG_EVT_CTXT_DETECTED 21980 /**/
#define MDC_ECG_EVT_CTXT_EXTERNAL 21981 /**/
#define MDC_ECG_HEART_RATE_INSTANT 21982 /**/

#define MDC_ECG_ELEC_POTL 256
#define MDC_ECG_ELEC_POTL_I 257 
#define MDC_ECG_ELEC_POTL_II 258
#define MDC_ECG_ELEC_POTL_III 317
#define MDC_ECG_ELEC_POTL_AVR 318 
#define MDC_ECG_ELEC_POTL_AVL 319 
#define MDC_ECG_ELEC_POTL_AVF 320 
#define MDC_ECG_ELEC_POTL_V1 259 
#define MDC_ECG_ELEC_POTL_V2 260 
#define MDC_ECG_ELEC_POTL_V3 261 
#define MDC_ECG_ELEC_POTL_V4 262 
#define MDC_ECG_ELEC_POTL_V5 263 
#define MDC_ECG_ELEC_POTL_V6 264

struct StdConfiguration *basic_ECG_create_std_config_ID0258();

/**
 * Event report data, used in Agent role
 */
struct basic_ECG_event_report_data {
	FLOAT_Type mV[20];
	int century;
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int sec_fractions;
};

extern uint16_t event_conf_or_unconf_basic_ecg;

#endif /* BASIC_ECG_H_ */
