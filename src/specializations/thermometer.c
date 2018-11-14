#include <stdio.h>
#include "thermometer.h"
#include "src/util/bytelib.h"
#include "src/dim/nomenclature.h"
#include "src/util/dateutil.h"
#include "communication/parser/encoder_ASN1.h"
#include "src/dim/mds.h"
#include "src/util/log.h"


static ConfigObjectList *thermometer_get_config_ID0320()
{
	ConfigObjectList *std_object_list = malloc(sizeof(ConfigObjectList));
	std_object_list->count = 1;
	std_object_list->length = 44;
	std_object_list->value = malloc(sizeof(ConfigObject) * std_object_list->count);
	std_object_list->value[0].obj_class = MDC_MOC_VMO_METRIC_NU;
	std_object_list->value[0].obj_handle = 1;

	AttributeList *attr_list1 = malloc(sizeof(AttributeList));
	attr_list1->count = 4;
	attr_list1->length = 36;
	attr_list1->value = malloc(attr_list1->count * sizeof(AVA_Type));

	attr_list1->value[0].attribute_id = MDC_ATTR_ID_TYPE;
	attr_list1->value[0].attribute_value.length = 4;
	ByteStreamWriter *bsw = byte_stream_writer_instance(4);
	write_intu16(bsw, MDC_PART_SCADA);
	write_intu16(bsw, MDC_TEMP_BODY);
	attr_list1->value[0].attribute_value.value = bsw->buffer;

	attr_list1->value[1].attribute_id = MDC_ATTR_METRIC_SPEC_SMALL;
	attr_list1->value[1].attribute_value.length = 2;
	free(bsw);
	bsw = byte_stream_writer_instance(2);
	write_intu16(bsw, 0xF040); // 0xF0 0x40
	attr_list1->value[1].attribute_value.value = bsw->buffer;

	attr_list1->value[2].attribute_id = MDC_ATTR_UNIT_CODE;
	attr_list1->value[2].attribute_value.length = 2;
	free(bsw);
	bsw = byte_stream_writer_instance(2);
	write_intu16(bsw, MDC_DIM_DEGC);
	attr_list1->value[2].attribute_value.value = bsw->buffer;

	attr_list1->value[3].attribute_id = MDC_ATTR_ATTRIBUTE_VAL_MAP;
	attr_list1->value[3].attribute_value.length = 12;
	free(bsw);
	bsw = byte_stream_writer_instance(12);
	write_intu16(bsw, 0x0002); 
	write_intu16(bsw, 0x0008); 
	write_intu16(bsw, MDC_ATTR_NU_VAL_OBS_SIMP);
	write_intu16(bsw, 0x0002); 
	write_intu16(bsw, MDC_ATTR_TIME_STAMP_ABS);
	write_intu16(bsw, 0x0008); 
	attr_list1->value[3].attribute_value.value = bsw->buffer;

	std_object_list->value[0].attributes = *attr_list1;

	free(attr_list1);
	free(bsw);

	return std_object_list;
}

static DATA_apdu *thermometer_populate_event_report(void *edata)
{
	DATA_apdu *data;
	EventReportArgumentSimple evt;
	ScanReportInfoFixed scan;
	ObservationScanFixedList scan_fixed;
	ObservationScanFixed measure[1];
	AbsoluteTime nu_time;
	FLOAT_Type nu_temperature;
	//FLOAT_Type nu_bmi;
	struct thermometer_event_report_data *evtdata;

	data = calloc(sizeof(DATA_apdu), 1);
	evtdata = (struct thermometer_event_report_data*) edata;

	nu_time = date_util_create_absolute_time(evtdata->century * 100 + evtdata->year,
						evtdata->month,
						evtdata->day,
						evtdata->hour,
						evtdata->minute,
						evtdata->second,
						evtdata->sec_fractions);

	nu_temperature = evtdata->temperature;
	//nu_bmi = evtdata->bmi;

	// will be filled afterwards by service_* function
	data->invoke_id = 0xffff;

	data->message.choice = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	data->message.length = 34;

	evt.obj_handle = 0;
	//evt.event_time = 0xFFFFFFFF;
	evt.event_time = 0x0;
	evt.event_type = MDC_NOTI_SCAN_REPORT_FIXED;
	evt.event_info.length = 24;

	scan.data_req_id = 0xF000;
	scan.scan_report_no = 0;

	scan_fixed.count = 1;
	scan_fixed.length = 16;
	scan_fixed.value = measure;

	measure[0].obj_handle = 1;
	measure[0].obs_val_data.length = 12;
	ByteStreamWriter *writer0 = byte_stream_writer_instance(measure[0].obs_val_data.length);

	write_float(writer0, nu_temperature);
	encode_absolutetime(writer0, &nu_time);

	//measure[1].obj_handle = 3;
	//measure[1].obs_val_data.length = 12;
	//ByteStreamWriter *writer1 = byte_stream_writer_instance(measure[1].obs_val_data.length);

	//write_float(writer1, nu_bmi);
	//encode_absolutetime(writer1, &nu_time);

	//measure[2].obj_handle = 1;
	//measure[2].obs_val_data.length = 12;
	//ByteStreamWriter *writer2 = byte_stream_writer_instance(measure[2].obs_val_data.length);

	//write_float(writer2, nu_weight);
	//encode_absolutetime(writer2, &nu_time);

	//measure[3].obj_handle = 3;
	//measure[3].obs_val_data.length = 12;
	//ByteStreamWriter *writer3 = byte_stream_writer_instance(measure[3].obs_val_data.length);

	//write_float(writer3, nu_bmi);
	//encode_absolutetime(writer3, &nu_time);

	measure[0].obs_val_data.value = writer0->buffer;
	//measure[1].obs_val_data.value = writer1->buffer;
	//measure[2].obs_val_data.value = writer2->buffer;
	//measure[3].obs_val_data.value = writer3->buffer;

	scan.obs_scan_fixed = scan_fixed;

	ByteStreamWriter *scan_writer = byte_stream_writer_instance(evt.event_info.length);

	encode_scanreportinfofixed(scan_writer, &scan);

	del_byte_stream_writer(writer0, 1);
	//del_byte_stream_writer(writer1, 1);
	//del_byte_stream_writer(writer2, 1);
	//del_byte_stream_writer(writer3, 1);

	evt.event_info.value = scan_writer->buffer;
	data->message.u.roiv_cmipEventReport = evt;

	del_byte_stream_writer(scan_writer, 0);

	return data;
}

struct StdConfiguration *thermometer_create_std_config_ID0320()
{
	struct StdConfiguration *result = malloc(
			sizeof(struct StdConfiguration));
	result->dev_config_id = 0x0320;
	result->configure_action = &thermometer_get_config_ID0320;
	result->event_report = &thermometer_populate_event_report;
	return result;
}
