/*
 * json.c
 *
 *  Created on: Mar 28, 2013
 *      Author: eal
 */

#include <stdio.h>
#include <stdbool.h>
#include "json.h"


const char *json_true = "true";
const char *json_false = "false";
const char *json_null = "null";

/* in serial_interactive.c */
extern bool interactive_mode;
extern const char *lf;
extern const char *crlf;


void json_start_response(bool result, const char *msg)
{
	const char *result_str = result ? json_true : json_false;
	printf("{\"result\":%s,\"msg\":\"%s\"", result_str, msg);
}


void json_add_int(const char *key, int val)
{
	printf(",\"%s\":%d", key, val);
}


void json_add_object(const char *key, json_kv_t *kv_pairs, uint8_t len)
{
	uint8_t i;


	printf(",\"%s\":{", key);
	for(i=0; i<len; i++)
	{
		if(i > 0)
			printf(",");
		printf("\"%s\":%d", kv_pairs[i].key, kv_pairs[i].value);
	}
	printf("}");
}


void json_end_response(void)
{
	const char *newline = interactive_mode ? crlf : lf;

	printf("}%s", newline);
}


void json_respond_ok(const char *msg)
{
	json_start_response(true, msg);
	json_end_response();
}


void json_respond_error(const char *msg)
{
	json_start_response(false, msg);
	json_end_response();
}
