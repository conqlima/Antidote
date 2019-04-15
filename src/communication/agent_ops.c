/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file agent_ops.c
 * \brief Finite State Machine agent operations
 * Copyright (C) 2011 Signove Tecnologia Corporation.
 * All rights reserved.
 * Contact: Signove Tecnologia Corporation (contact@signove.com)
 *
 * $LICENSE_TEXT:BEGIN$
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and appearing
 * in the file LICENSE included in the packaging of this file; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * $LICENSE_TEXT:END$
 *
 * IEEE 11073 Communication Model - Finite State Machine implementation
 *
 * \author Elvis Pfutzenreuter
 * \date May 31, 2011
 */

#include "src/asn1/phd_types.h"
#include "fsm.h"
#include "communication.h"
#include "communication/service.h"
#include "communication/stdconfigurations.h"
#include "communication/parser/encoder_ASN1.h"
#include "communication/parser/decoder_ASN1.h"
#include "src/communication/parser/struct_cleaner.h"
#include "src/dim/mds.h"
#include "src/util/log.h"
#include "src/agent_p.h"
#include <string.h>
#include <stdbool.h>

static DataReqMode req_mode = 0;
static intu32 singleMeasurement = 0;
static intu32 timeLimit = 0;
static intu32 noTimeLimit = 0;
static int start[6] = {0};



/**
 * Answer to unexpected AARQ (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_aare_rejected_permanent_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Answer to unexpected ROIV (e.g. in config phase) (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roer_no_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Respond to confirmed ROIV Action request (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_confirmed_action_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	int nodeNumber = (ctx->id.plugin+1)/2;
	/* TODO HERE ROBSON */
	//ROIV_CMIP_EVENT_REPORT_CHOSEN = 0x0100,
	//ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN = 0x0101,
	//ROIV_CMIP_GET_CHOSEN = 0x0103,
	//ROIV_CMIP_SET_CHOSEN = 0x0104,
	//ROIV_CMIP_CONFIRMED_SET_CHOSEN = 0x0105,
	//ROIV_CMIP_ACTION_CHOSEN = 0x0106,
	//ROIV_CMIP_CONFIRMED_ACTION_CHOSEN = 0x0107,
	//DATA_apdu *rx = encode_get_data_apdu(&data->received_apdu->u.prst);
	//switch(rx->message.choice){
		//case ROIV_CMIP_CONFIRMED_ACTION_CHOSEN:
		//communication_agent_roiv_confirmed_action_respond_tx(ctx, evt, data);
		//break;
		//default: return;
		//break;
	//}
	/* TODO HERE ROBSON */
	//#define MDC_ACT_SEG_CLR 3084 /* */
	//#define MDC_ACT_SEG_GET_INFO 3085 /* */
	//#define MDC_ACT_SET_TIME 3095 /* */
	//#define MDC_ACT_DATA_REQUEST 3099 /* */
	//#define MDC_ACT_SEG_TRIG_XFER 3100 /* */
	DATA_apdu *rx = encode_get_data_apdu(&data->received_apdu->u.prst);
	switch(rx->message.u.roiv_cmipConfirmedAction.action_type){
		case MDC_ACT_DATA_REQUEST:{
			intu8 *buffer = rx->message.u.roiv_cmipConfirmedAction.action_info_args.value;
			intu16 length = rx->message.u.roiv_cmipConfirmedAction.action_info_args.length;
			ByteStreamReader *stream = byte_stream_reader_instance(buffer, length);
			DataRequest *request = (DataRequest *) calloc(1,sizeof(DataRequest));
			int error = 0;
			decode_datarequest(stream, request, &error);
			if (request->data_req_mode & DATA_REQ_START_STOP)
			{

				

				start[nodeNumber] = 1;
				req_mode = request->data_req_mode;
				if (request->data_req_mode & DATA_REQ_SUPP_MODE_SINGLE_RSP)
				{
					singleMeasurement = 1;
				}
				else if (request->data_req_mode & DATA_REQ_SUPP_MODE_TIME_PERIOD)
				{
					timeLimit = 1;
				}
				else if (request->data_req_mode & DATA_REQ_SUPP_MODE_TIME_NO_LIMIT)
				{
					noTimeLimit = 1;
				}
				APDU *apdu = (APDU *)calloc(1, sizeof(APDU));
				DATA_apdu *data_apdu = (DATA_apdu *) calloc(1, sizeof(DATA_apdu));

				if (apdu != NULL && data_apdu != NULL) 
				{
					apdu->choice = PRST_CHOSEN;
				}
				
				//observationsscan
				PRST_apdu prst;
				ConfigId spec = agent_configuration(nodeNumber)->config;
				struct StdConfiguration *cfg =
					std_configurations_get_supported_standard(spec);
				// TODO support extended configurations too for agent
			
				if (!cfg) {
					DEBUG("No std configuration for %d, bailing out", spec);
					return;
				}
			
				void *evtreport = agent_configuration(nodeNumber)->event_report_cb();
				data_apdu = cfg->event_report(evtreport);
				free(evtreport);
				
				data_apdu->message.choice = RORS_CMIP_CONFIRMED_ACTION_CHOSEN;
				data_apdu->message.u.roiv_cmipConfirmedAction.obj_handle = 0;
				data_apdu->message.u.roiv_cmipConfirmedAction.action_type
				= MDC_ACT_DATA_REQUEST;
				
				DataResponse *response = (DataResponse *) calloc(1,
				       sizeof(DataResponse));
				response->rel_time_stamp = 0;
				response->data_req_result = DATA_REQ_RESULT_NO_ERROR;
				response->event_type = MDC_NOTI_SCAN_REPORT_FIXED;
				response->event_info = data_apdu->message.u.roiv_cmipEventReport.event_info;
				
				int length = sizeof(RelativeTime) 
					+ sizeof(DataReqResult)
					+ sizeof(OID_Type) + sizeof(intu16)
					+ response->event_info.length;
				ByteStreamWriter *writer1 = byte_stream_writer_instance(length);
				encode_dataresponse(writer1, response);

				free(response);
				response = NULL;
				
				data_apdu->message.u.rors_cmipConfirmedAction.action_info_args.value
				= writer1->buffer;
				data_apdu->message.u.rors_cmipConfirmedAction.action_info_args.length
				= writer1->size;
				
				// prst = length + DATA_apdu
				// take into account data's invoke id and choice
				data_apdu->message.length
				= data_apdu->message.u.rors_cmipConfirmedAction.action_info_args.length
				+ sizeof(intu16)
				+ sizeof(OID_Type)
				+ sizeof(ASN1_HANDLE);
				
				//response for remote invoke has the same id
				data_apdu->invoke_id = rx->invoke_id;
				
				prst.length = data_apdu->message.length 
					+ sizeof(intu16) 
					+ sizeof(DATA_apdu_choice)
					+ sizeof(InvokeIDType); // 46 + 6 = 52 for oximeter
				encode_set_data_apdu(&prst, data_apdu);
				
				apdu->length = prst.length + sizeof(intu16); // 52 + 2 = 54 for oximeter
				apdu->u.prst = prst;
				
				
				communication_send_apdu(ctx, apdu);
				// passes ownership
				//if (data_apdu->message.choice == ROIV_CMIP_EVENT_REPORT_CHOSEN) {
					//service_send_unconfirmed_operation_request(ctx, apdu);
				//} else {
					// ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN, presumably
					//timeout_callback tm = {.func = &communication_timeout, .timeout = 3};
					//service_send_remote_operation_request(ctx, apdu, tm, NULL);
				//}
				free(writer1);
			}
			else
			{
				start[nodeNumber] = 0;
			}
			break;
		}
	
	}

}

/**
 * Respond to a ROIV that is not ROIV GET
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Respond to a ROIV event report
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_event_report_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Respond to a confirmed ROIV event report
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_confirmed_event_report_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Respond to ROIV SET request (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_set_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * Respond to confirmed ROIV SET request (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_confirmed_set_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}


/**
 * Respond to ROIV Action request (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param data state machine event data
 */
void communication_agent_roiv_action_respond_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	/* TODO */
}

/**
 * React to "Send Event" request to state machine (Agent)
 *
 * @param ctx state machine context
 * @param evt state machine event
 * @param evtdata state machine event data
 */
void communication_agent_send_event_tx(FSMContext *ctx, fsm_events evt, FSMEventData *evtdata)
{
	int nodeNumber = (ctx->id.plugin+1)/2;
	APDU *apdu = calloc(sizeof(APDU), 1);
	PRST_apdu prst;
	DATA_apdu *data;

	ConfigId spec = agent_configuration(nodeNumber)->config;
	struct StdConfiguration *cfg =
		std_configurations_get_supported_standard(spec);
	// TODO support extended configurations too for agent

	if (!cfg) {
		DEBUG("No std configuration for %d, bailing out", spec);
		return;
	}

	void *evtreport = agent_configuration(nodeNumber)->event_report_cb();
	data = cfg->event_report(evtreport);
	free(evtreport);

	// prst = length + DATA_apdu
	// take into account data's invoke id and choice
	prst.length = data->message.length + 6; // 46 + 6 = 52 for oximeter
	encode_set_data_apdu(&prst, data);
	
	apdu->choice = PRST_CHOSEN;
	apdu->length = prst.length + 2; // 52 + 2 = 54 for oximeter
	apdu->u.prst = prst;

	// passes ownership
	if (data->message.choice == ROIV_CMIP_EVENT_REPORT_CHOSEN) {
		service_send_unconfirmed_operation_request(ctx, apdu);
	} else {
		// ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN, presumably
		timeout_callback tm = {.func = &communication_timeout, .timeout = 3};
		service_send_remote_operation_request(ctx, apdu, tm, NULL);
	}
}

/**
 * Execute association data protocol 20601 - agent
 *
 * @param ctx Context
 * @param evt State events
 * @param data Event data
 */
void association_agent_mds(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	DEBUG("association_agent_mds");
	int nodeNumber = (ctx->id.plugin+1)/2;
	if (ctx->mds != NULL) {
		mds_destroy(ctx->mds);
	}

	ConfigId spec = agent_configuration(nodeNumber)->config;
	ConfigObjectList *cfg = std_configurations_get_configuration_attributes(spec);

	MDS *mds = mds_create();
	ctx->mds = mds;

	struct mds_system_data *mds_data = agent_configuration(nodeNumber)->mds_data_cb();

	mds->dev_configuration_id = agent_configuration(nodeNumber)->config;
	mds->data_req_mode_capab.data_req_mode_flags = DATA_REQ_SUPP_INIT_AGENT;
	// max number of simultaneous sessions
	mds->data_req_mode_capab.data_req_init_agent_count = 1;
	mds->data_req_mode_capab.data_req_init_manager_count = 0;
	mds->system_id.length = 8;
	mds->system_id.value = (intu8*) malloc(mds->system_id.length);
	memcpy(mds->system_id.value, &mds_data->system_id, mds->system_id.length);

	mds_configure_operating(ctx, cfg, 0);

	free(mds_data);
	free(cfg);
}

/**
 * Answer ROIV GET MDS - agent
 *
 * @param ctx Context
 * @param evt State events
 * @param data Event data
 */
void communication_agent_roiv_get_mds_tx(FSMContext *ctx, fsm_events evt, FSMEventData *data)
{
	DATA_apdu *rx = encode_get_data_apdu(&data->received_apdu->u.prst);
	InvokeIDType id = rx->invoke_id;

	if (rx->message.choice != ROIV_CMIP_GET_CHOSEN) {
		return;
	}

	if (rx->message.u.roiv_cmipGet.obj_handle != MDS_HANDLE) {
		communication_fire_evt(ctx, fsm_evt_rx_roiv, NULL);
		return;
	}

	DEBUG("send RORS with MDS");

	APDU apdu;
	apdu.choice = PRST_CHOSEN;

	DATA_apdu *data_apdu = calloc(sizeof(DATA_apdu), 1);
	data_apdu->invoke_id = id;
	data_apdu->message.choice = RORS_CMIP_GET_CHOSEN;

	AttributeList attrs;

	attrs.count = 0;
	attrs.length = 0;
	attrs.value = mds_get_attributes(ctx->mds, &attrs.count, &attrs.length);
	
	DEBUG("send RORS with MDS: %d attributes, length %d", attrs.count, attrs.length);

	data_apdu->message.u.rors_cmipGet.obj_handle = MDS_HANDLE;
	data_apdu->message.u.rors_cmipGet.attribute_list = attrs;

	data_apdu->message.length = sizeof(data_apdu->message.u.rors_cmipGet.obj_handle) +
					2 + 2 + attrs.length;

	apdu.u.prst.length = sizeof(id)
			     + sizeof(data_apdu->message.choice)
			     + sizeof(data_apdu->message.length)
			     + data_apdu->message.length;

	apdu.length = sizeof(apdu.u.prst.length) + apdu.u.prst.length;
	encode_set_data_apdu(&apdu.u.prst, data_apdu);
	communication_send_apdu(ctx, &apdu);

	// deletes information that came from mds_get_attributes()
	// and data_apdu
	del_apdu(&apdu);
}

/**
 * Return the manager-initiated mode
 *
 * @return mode
 */
DataReqMode communication_manager_initiated_mode(void)
{
	return req_mode;
}

int communication_manager_initiated_mode_start(int nodeNumber)
{
	return start[nodeNumber];
}



/** @} */
