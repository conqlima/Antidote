#ifndef THERMOMETER_MONITOR_H_
#define THERMOMETER_MONITOR_H_

#include <communication/stdconfigurations.h>

/* Thermometer nomenclature */
/*********************************************************************************
* From Medical supervisory control and data acquisition (MDC_PART_SCADA)
**********************************************************************************/

#define MDC_TEMP_TYMP		19320
#define MDC_TEMP_RECT		57348
#define MDC_TEMP_ORAL		57352
#define MDC_TEMP_EAR		57356
#define MDC_TEMP_FINGER		57360
#define MDC_TEMP_TOE		57376

/* New nomenclature codes introduced in the present document (IEEE Std 11073-10408).*/
#define MDC_TEMP_AXILLA		57380
#define MDC_TEMP_GIT		57384

/*********************************************************************************
* From Dimensions (MDC_PART_DIM)
**********************************************************************************/

#define MDC_DIM_FAHR	4416// °F

struct StdConfiguration *thermometer_create_std_config_ID0320();

/**
 * Event report data, used in Agent role
 */
struct thermometer_event_report_data {
	FLOAT_Type temperature;
	int century;
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int sec_fractions;
};

extern uint16_t event_conf_or_unconf_thermometer;

#endif /* THERMOMETER_MONITOR_H_ */
