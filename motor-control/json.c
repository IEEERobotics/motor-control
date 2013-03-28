/*
 * json.c
 *
 *  Created on: Mar 28, 2013
 *      Author: eal
 */

#include <stdio.h>
#include <stdbool.h>
#include "pid.h"
#include "json.h"


const char *json_true = "true";
const char *json_false = "false";
const char *json_null = "null";

/* in serial_interactive.c */
extern bool interactive_mode;
extern const char *lf;
extern const char *crlf;


void json_start_response(bool result, const char *msg, int id)
{
	const char *result_str = result ? json_true : json_false;

	/* Why? to keep a JSON response from being printed in the PID controller interrupt while
	 * another one is being printed in serial_interactive.c
	 */
	bool pid_enabled = pid_is_enabled();

	if(pid_enabled) pid_disable();
	printf("{\"result\":%s,\"msg\":\"%s\",\"id\":%d", result_str, msg, id);
	if(pid_enabled) pid_enable();
}


void json_add_int(const char *key, int val)
{
	bool pid_enabled = pid_is_enabled();

	if(pid_enabled) pid_disable();
	printf(",\"%s\":%d", key, val);
	if(pid_enabled) pid_enable();
}


void json_add_object(const char *key, json_kv_t *kv_pairs, uint8_t len)
{
	uint8_t i;
	bool pid_enabled = pid_is_enabled();

	if(pid_enabled) pid_disable();
	printf(",\"%s\":{", key);
	for(i=0; i<len; i++)
	{
		if(i > 0)
			printf(",");
		printf("\"%s\":%d", kv_pairs[i].key, kv_pairs[i].value);
	}
	printf("}");
	if(pid_enabled) pid_enable();
}


void json_end_response(void)
{
	const char *newline = interactive_mode ? crlf : lf;
	bool pid_enabled = pid_is_enabled();

	if(pid_enabled) pid_disable();
	printf("}%s", newline);
	if(pid_enabled) pid_enable();
}


void json_respond_ok(const char *msg, int id)
{
	json_start_response(true, msg, id);
	json_end_response();
}


void json_respond_error(const char *msg, int id)
{
	json_start_response(false, msg, id);
	json_end_response();
}
