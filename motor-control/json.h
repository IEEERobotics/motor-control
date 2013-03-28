/*
 * json.h
 *
 *  Created on: Mar 28, 2013
 *      Author: eal
 */

#ifndef JSON_H_
#define JSON_H_

#include <stdio.h>
#include <stdbool.h>


typedef struct json_kv {
	const char *key;
	int value;
} json_kv_t;

void json_start_response(bool result, const char *msg, int id);
void json_add_int(const char *key, int val);
void json_add_object(const char *key, json_kv_t *kv_pairs, uint8_t len);
void json_end_response(void);
void json_respond_ok(const char *msg, int id);
void json_respond_error(const char *msg, int id);


#endif /* JSON_H_ */
