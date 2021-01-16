/*
 * sfsc_adapter_data.c
 *
 *  Created on: 28.10.2020
 *      Author: Eric
 */

#include "sfsc_adapter_data.h"

sfsc_int8 process_io(_sfsc_adapter_data* adapter) {
	sfsc_int8 op_result = zmtp_task(&(adapter->control_sub));
	if (op_result == ZMTP_OK) {
		sfsc_int8 op_result = zmtp_task(&(adapter->control_pub));
		if (op_result == ZMTP_OK) {
			sfsc_int8 op_result = zmtp_task(&(adapter->data_sub));
			if (op_result == ZMTP_OK) {
				return zmtp_task(&(adapter->data_pub));
			} else {
				return op_result;
			}
		} else {
			return op_result;
		}
	} else {
		return op_result;
	}
}
