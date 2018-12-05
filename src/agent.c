/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file agent.c
 * \brief Protocol agent implementation.
 * \author Adrian Guedes
 * \author Fabricio Silva
 * \author Elvis Pfutzenreuter
 * \date May 31, 2011
 *
 * \internal
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
 */

/**
 * \defgroup Agent Agent
 * \brief The Agent provides a facade to use IEEE standard implementation. All health/wellness
 * application should access IEEE standard library through this class.
 *
 * The following code shows how to implement a simple application that reads data from device. Once
 * the health device is properly paired with agent (desktop), the application receives measures
 * and print them. It is also possible to execute commands, such as <em>connect</em>, <em>disconnect</em>
 * and <em>exit</em>.
 *
 * @{
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "src/agent_p.h"
#include "src/api/data_encoder.h"
#include "src/communication/plugin/plugin.h"
#include "src/communication/communication.h"
#include "src/communication/configuring.h"
#include "src/communication/stdconfigurations.h"
#include "src/specializations/blood_pressure_monitor.h"
#include "src/specializations/pulse_oximeter.h"
#include "src/specializations/weighing_scale.h"
#include "src/specializations/glucometer.h"
#include "src/specializations/thermometer.h"
#include "src/specializations/basic_ECG.h"
#include "src/dim/mds.h"
#include "src/util/log.h"


/**
 * Agent listener list
 */
static AgentListener *agent_listener_list = NULL;

/**
 * Agent listener count
 */
static int agent_listener_count = 0;

/**
 * Agent configuration
 */
static AgentConfiguration *configuration = NULL;
static unsigned int configuration_size = 0;

AgentConfiguration *agent_configuration(int nodeNumber)
{
	return &configuration[nodeNumber];
}

static void agent_handle_transition_evt(Context *ctx, fsm_states previous, fsm_states next);

/*!
 *
 * This API implements the IEEE 11073-20601 Standard and some device
 * specializations (IEEE 11073-104XX).
 *
 * The IEEE 11073-20601 defines a common framework for making an abstract
 * model of personal health data available in transport-independent
 * transfer syntax required to establish logical connections between
 * systems and to provide presentation capabilities and services needed
 * to perform communication tasks. The protocol is optimized to personal
 * health usage requirements and leverages commonly used methods and
 * tools wherever possible.
 *
 * The ISO/IEEE 11073 family of standards is based on an object-oriented
 * systems management paradigm. Data (measurement, state, and so on) are
 * modeled in the form of information objects that are accessed and
 * manipulated using an object access service protocol.
 *
 * The overall ISO/IEEE 11073 system model is divided into three principal
 * components: the DIM, the service model, and the communications model.
 * These three models work together to represent data, define data access and
 * command methodologies, and communicate the data from an agent to a agent.
 *
 * For more information about how to use IEEE standard implementation, see \ref Agent
 * implementation (including examples).
 *
 * For more information about how to add new device specializations at IEEE library
 * , see documentation on \ref SpecializationDescription "IEEE device specializations"
 */

int agent_notify_evt_device_connected(Context *ctx, const char *addr);
int agent_notify_evt_device_disconnected(Context *ctx, const char *addr);

/**
 * Initializes the agent on application load. This function also
 * registers existing device specializations.
 *
 * This method should be invoked in a thread safe execution.
 *
 * @param plugins the configured plugins to define communication behavior
 * @param config Configuration ID of the agent
 * @param event_report_cb The event report callback
 * @param mds_data_cb Data callback
 */
void agent_init(ContextId id, CommunicationPlugin **plugins, int config,
		void *(*event_report_cb)(),
		struct mds_system_data *(*mds_data_cb)())
{
	
	DEBUG("Agent Initialization");
	unsigned int nodeNumber = (id.plugin+1)/2;
	
	if (!configuration){
	configuration_size = nodeNumber;
	configuration = (AgentConfiguration*) malloc(sizeof(AgentConfiguration)*(nodeNumber+1));
	//Is the firts time here? so register!
	// Register standard configurations for each specialization.
	std_configurations_register_conf(
		blood_pressure_monitor_create_std_config_ID02BC());
	std_configurations_register_conf(
		pulse_oximeter_create_std_config_ID0190());
	std_configurations_register_conf(
		pulse_oximeter_create_std_config_ID0191());
	std_configurations_register_conf(
		weighting_scale_create_std_config_ID05DC());
	std_configurations_register_conf(
		glucometer_create_std_config_ID06A4());
	std_configurations_register_conf(
		thermometer_create_std_config_ID0320());
	std_configurations_register_conf(
		basic_ECG_create_std_config_ID0258());
	}else if (nodeNumber > configuration_size){
	configuration_size = nodeNumber;
	configuration = (AgentConfiguration*) realloc(configuration, sizeof(AgentConfiguration)*(nodeNumber+1));
	}
	configuration[nodeNumber].config = config;
	configuration[nodeNumber].event_report_cb = event_report_cb;
	configuration[nodeNumber].mds_data_cb = mds_data_cb;
	
	//while (*plugins) {
	if (*plugins) {
		(*plugins)->type |= AGENT_CONTEXT;
		communication_add_plugin(id, *plugins);
		++plugins;
	}

	// Listen to all communication state transitions
	communication_add_state_transition_listener(id, fsm_state_size, &agent_handle_transition_evt);
	communication_set_connection_listeners(&agent_notify_evt_device_connected,
						&agent_notify_evt_device_disconnected);

	
}

/**
 * Finalizes the agent
 *
 * This method should be invoked in a thread safe execution.
 *
 */
void agent_finalize(ContextId id)
{
	static int finalize = 0;
	DEBUG("Agent Finalization");
	++finalize;
	if (finalize == 5){
	finalize = 0;
	configuration_size = 0;
	free(configuration);
	configuration = NULL;
	}

	//agent_remove_all_listeners();
	//std_configurations_destroy();
	//communication_finalize(id);
}


/**
 * Adds a agent listener.
 *
 * This method should be invoked in a thread safe execution.
 *
 * @param listener the listen to be added.
 * @return 1 if operation succeeds, 0 if not.
 */
int agent_add_listener(ContextId id, AgentListener listener)
{

	// test if there is not elements in the list
	unsigned int size = agent_listener_count;
if (size == 0) {
		agent_listener_list = malloc(sizeof(struct AgentListener));

	} 
	if (size < id.plugin) { // change the list size
		agent_listener_list = realloc(agent_listener_list,
						sizeof(struct AgentListener)
						* (id.plugin + 1));
	agent_listener_count = id.plugin;
	}

	// add element to list

	if (agent_listener_list == NULL) {
		return 0;
	}

	agent_listener_list[id.plugin] = listener;

	//agent_listener_count++;

	return 1;

}


/**
 * Removes all agent's listeners
 *
 * This method should be invoked in a thread safe execution.
 */
void agent_remove_all_listeners()
{
	if (agent_listener_list != NULL) {
		agent_listener_count = 0;
		free(agent_listener_list);
		agent_listener_list = NULL;
	}
	if (configuration != NULL){
	free(configuration);
	configuration = NULL;
	}
}

/**
 * Notifies 'device associated'  event.
 * This function should be visible to source layer of events.
 * This function must be called in a thread safe communication context.
 *
 * @param ctx
 * @return 1 if any listener catches the notification, 0 if not
 */
int agent_notify_evt_device_associated(Context *ctx)
{
	int ret_val = 0;
	//int i;

	//for (i = 0; i < agent_listener_count; i++) {
		AgentListener *l = &agent_listener_list[ctx->id.plugin];

		if (l != NULL && l->device_associated != NULL) {
			(l->device_associated)(ctx);
			ret_val = 1;
		}
	//}

	return ret_val;
}

/**
 * Notifies 'device connected'  event.
 * This function should be visible to source layer of events.
 * This function must be called in a thread safe communication context.
 *
 * @param ctx
 * @param addr
 * @return 1 if any listener catches the notification, 0 if not
 */
int agent_notify_evt_device_connected(Context *ctx, const char *addr)
{
	int ret_val = 0;
	//int i;

	//for (i = 0; i < agent_listener_count; i++) {
		AgentListener *l = &agent_listener_list[ctx->id.plugin];

		if (l != NULL && l->device_connected != NULL) {
			(l->device_connected)(ctx, addr);
			ret_val = 1;
		}
	//}

	return ret_val;
}

/**
 * Notifies 'device disconnected'  event.
 * This function should be visible to source layer of events.
 * This function must be called in a thread safe communication context.
 *
 * @param ctx
 * @param addr
 * @return 1 if any listener catches the notification, 0 if not
 */
int agent_notify_evt_device_disconnected(Context *ctx, const char *addr)
{
	int ret_val = 0;
	//int i;

	//for (i = 0; i < agent_listener_count; i++) {
		AgentListener *l = &agent_listener_list[ctx->id.plugin];

		if (l != NULL && l->device_disconnected != NULL) {
			(l->device_disconnected)(ctx, addr);
			ret_val = 1;
		}
	//}

	return ret_val;
}

/**
 * Notifies 'device unavailable'  event.
 * This function should be visible to source layer of events.
 * This function must be called in a thread safe communication context.
 *
 * @param ctx
 * @return 1 if any listener catches the notification, 0 if not
 */
int agent_notify_evt_device_unavailable(Context *ctx)
{
	int ret_val = 0;
	//int i;

	//for (i = 0; i < agent_listener_count; i++) {
		AgentListener *l = &agent_listener_list[ctx->id.plugin];

		if (l != NULL && l->device_unavailable != NULL) {
			(l->device_unavailable)(ctx);
			ret_val = 1;
		}
	//}

	return ret_val;
}


/**
 * Notifies 'communication timeout'  event.
 * This function should be visible to source layer of events.
 * This function must be called in a thread safe communication context.
 *
 * @param ctx current context
 * @return 1 if any listener catches the notification, 0 if not
 */
int agent_notify_evt_timeout(Context *ctx)
{
	int ret_val = 0;
	//int i;

	//for (i = 0; i < agent_listener_count; i++) {
		AgentListener *l = &agent_listener_list[ctx->id.plugin];

		if (l != NULL && l->timeout != NULL) {
			(l->timeout)(ctx);
			ret_val = 1;
		}
	//}

	return ret_val;
}

/**
 * Start to listen agents, it means the network layer
 * is initialized and the communication will be ready to
 * read/send data.
 */
void agent_start(ContextId id)
{
	// kill any previous connection
	if (communication_is_network_started(id)) {
		DEBUG("Agent restarting...");
		agent_stop(id);
	} else {
		DEBUG("Agent starting...");
	}

	communication_network_start(id);
}


/**
 * Stop to listen agents, all open network connections will be closed
 */
void agent_stop(ContextId id)
{
	if (communication_is_network_started(id)) {
		DEBUG("Agent stopping...");
		communication_network_stop(id);
	}
}

/**
 * Runs connection loop over communication layer.
 * This function must run after 'agent_start()'operation if no mainloop
 * implementation is used e.g. GMainLoop.
 *
 * @param context_id Current context.
 */
void agent_connection_loop(ContextId context_id)
{
	Context *ctx;
	while ((ctx = context_get_and_lock(context_id))) {
		communication_connection_loop(ctx);
		context_unlock(ctx);
		sleep(1);
	}
}

/**
 * Requests "association release request" to agent
 * @param id the ID of current context.
 */
void agent_request_association_release(ContextId id)
{
	Context *ctx = context_get_and_lock(id);

	if (ctx) {
		req_association_release(ctx);
		context_unlock(ctx);
	}
}

/**
 * Requests "association abort" to agent
 * @param id the ID of current context.
 */
void agent_request_association_abort(ContextId id)
{
	Context *ctx = context_get_and_lock(id);

	if (ctx) {
		communication_fire_evt(ctx, fsm_evt_req_assoc_abort, NULL);
		context_unlock(ctx);
	}
}


/**
 * Handle low level communication state transition events and notifies
 * high level application
 *
 * @param ctx context
 * @param previous the previous FSM state.
 * @param next the next FSM state.
 */
void agent_handle_transition_evt(Context *ctx, fsm_states previous, fsm_states next)
{
	DEBUG("agent: handling transition event");

	if (previous == fsm_state_operating && next != previous) {
		DEBUG(" agent: Notify device unavailable.\n");
		// Exiting operating state
		agent_notify_evt_device_unavailable(ctx);
	}

	if (previous != next && next == fsm_state_operating) {
		agent_notify_evt_device_associated(ctx);
	}

	if (next == fsm_state_config_sending) {
		// provoke it to go to waiting_approval
		communication_fire_evt(ctx, fsm_evt_req_send_config, NULL);
	}
}

/**
 * Provoke agent to initiate association
 *
 * @param id context Id
 */
void agent_associate(ContextId id)
{
	DEBUG(" agent: Move state machine to init assoc");
	Context *ctx = context_get_and_lock(id);

	if (ctx) {
		communication_fire_evt(ctx, fsm_evt_req_assoc, NULL);
		context_unlock(ctx);
	}
}

/**
 * Provoke agent to terminate connection
 *
 * @param id context Id
 */
void agent_disconnect(ContextId id)
{
	DEBUG(" agent: terminate conn");
	Context *ctx = context_get_and_lock(id);

	if (ctx) {
		communication_force_disconnect(ctx);
		context_unlock(ctx);
	}
}

/**
 * Provoke agent to send event report with measure data
 *
 * @param id context Id
 */
void agent_send_data(ContextId id)
{
	Context *ctx = context_get_and_lock(id);

	if (ctx) {
		communication_fire_evt(ctx, fsm_evt_req_send_event, NULL);
		context_unlock(ctx);
	}
}

/** @} */
