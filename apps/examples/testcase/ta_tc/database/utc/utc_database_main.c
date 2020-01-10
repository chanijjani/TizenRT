/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <arastorage/arastorage.h>
#include <tinyara/fs/fs_utils.h>
#include "tc_common.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/
#ifndef CONFIG_MOUNT_POINT
#define CONFIG_MOUNT_POINT "/mnt"
#endif

#define RELATION_NAME1  "rel1"
#define RELATION_NAME2  "rel2"
#define INDEX_BPLUS     "bplustree"
#define INDEX_INLINE    "inline"
#define QUERY_LENGTH    128

#define DATA_SET_NUM    10
#define DATA_SET_MULTIPLIER 80

/****************************************************************************
 *  Global Variables
 ****************************************************************************/
static db_cursor_t *g_cursor;

const static char *g_attribute_set[] = {"id", "date", "fruit", "value", "weight"};

struct arastorage_data_type_s {
	long long_value;
	char *string_value;
	double double_value;
};

const static struct arastorage_data_type_s g_arastorage_data_set[DATA_SET_NUM] = {
	{20160101,       "apple"     , 1.0 },
	{20160102,       "banana"    , 2.0 },
	{20160103,       "mango"     , 3.0 },
	{20160104,       "orange"    , 4.0 },
	{20160105,       "melon"     , 5.0 },
	{20160106,       "kiwi"      , 6.0 },
	{20160107,       "peach"     , 7.0 },
	{20160108,       "cherry"    , 8.0 },
	{20160109,       "strawberry", 9.0 },
	{20160110,       "watermelon", 10.0}
};

static void check_query_result(char * query)
{
	db_result_t res;
	g_cursor = db_query(query);
	TC_ASSERT_NEQ("db_query", g_cursor, NULL);
	if (DB_SUCCESS(cursor_move_first(g_cursor))) {
		do {
			res = db_print_tuple(g_cursor);
			TC_ASSERT_EQ("db_print_tuple", DB_SUCCESS(res), true);
		} while (DB_SUCCESS(cursor_move_next(g_cursor)));
	}

	res = db_cursor_free(g_cursor);
	TC_ASSERT_EQ("db_cursor_free", DB_SUCCESS(res), true);
	g_cursor = NULL;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/* Clean up all components used in these TCs */
static void cleanup(void)
{
	char query[QUERY_LENGTH];

	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "REMOVE RELATION %s;", RELATION_NAME1);
	db_exec(query);

	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "REMOVE RELATION %s;", RELATION_NAME2);
	db_exec(query);
}

/**
* @testcase         utc_arastorage_db_init_p
* @brief            Initialize database resources
* @scenario         Initialize resources used in database
* @apicovered       db_init
* @precondition     none
* @postcondition    none
*/
static void utc_arastorage_db_init_p(void)
{
	db_result_t res;

	res = db_init();
	TC_ASSERT_EQ("db_init", DB_SUCCESS(res), true);

	TC_SUCCESS_RESULT();
}

static void utc_database_dbcreate_1(void)
{
	db_result_t res;

	db_query_mm_t mm;
	char segment[2000];
	init_query_mm(&mm, segment, 2000);

	char command[] = "CREATE TABLE mytable (attr0 INT);";
	char tablename[] = "mytable";
	db_lexer_t lexer;
	res = lexer_init(&lexer, command);

//	CuAssertTrue(tc, 1==lexer_next(&lexer));
//	CuAssertTrue(tc, DB_LEXER_TT_RESERVED == lexer.token.type);
//	CuAssertTrue(tc, DB_LEXER_TOKENINFO_COMMANDCLAUSE == lexer.token.info);
//	CuAssertTrue(tc, DB_LEXER_TOKENBCODE_CLAUSE_CREATE == lexer.token.bcode);
//	CuAssertTrue(tc, 1==processCreate(&lexer, strlen(command), &mm));
	TC_ASSERT_EQ("lexer_next", DB_SUCCESS(res), true);


	db_fileref_t newtable = db_openreadfile(tablename);
	db_uint8 temp8;
	char tempstring[100];

	/* Number of attributes. */
	res = db_fileread(newtable, &temp8, 1);
//	CuAssertTrue(tc, 1==db_fileread(newtable, &temp8, 1));
//	CuAssertTrue(tc, 1==temp8);
	TC_ASSERT_EQ("db_fileread", DB_SUCCESS(res), 1);
	TC_ASSERT_EQ("temp8", DB_SUCCESS(temp8), 1);

//	/* Header information for attr0. */
//	/* Size of name. */
//	CuAssertTrue(tc, 1==db_fileread(newtable, &temp8, 1));
//	CuAssertTrue(tc, 6==temp8);
//	/* Contents of name. */
//	CuAssertTrue(tc, 6==db_fileread(newtable, (unsigned char*)tempstring, 6));
//	CuAssertTrue(tc, 0==strcmp("attr0", tempstring));
//	/* Attribute type. */
//	CuAssertTrue(tc, 1==db_fileread(newtable, &temp8, 1));
//	CuAssertTrue(tc, DB_INT==temp8);
//	/* Attribute offset. */
//	CuAssertTrue(tc, 1==db_fileread(newtable, &temp8, 1));
//	CuAssertTrue(tc, 0==temp8);
//	/* Attribute size. */
//	CuAssertTrue(tc, 1==db_fileread(newtable, &temp8, 1));
//	CuAssertTrue(tc, ((db_uint8)(sizeof(db_int)))==temp8);
//
//	CuAssertTrue(tc, 1==db_fileremove(tablename));
//
//	TC_ASSERT_EQ("db_exec", DB_SUCCESS(res), true);

	TC_SUCCESS_RESULT();
}

/**
* @testcase         utc_arastorage_db_exec_n
* @brief            Excute database operations with invalid argument
* @scenario         Create exiting relation, attribute and excute db with NULL value
* @apicovered       db_exec
* @precondition     none
* @postcondition    none
*/
static void utc_arastorage_db_exec_n(void)
{
	db_result_t res;
	char query[QUERY_LENGTH];
	char *name = "BAD_RELATION";
	char *attr_name = "attr";
	/* Try to create existing relation */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "CREATE RELATION %s;", RELATION_NAME1);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* Try to create existing attribute */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "CREATE ATTRIBUTE %s DOMAIN int IN %s;", g_attribute_set[0], RELATION_NAME1);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* Try to excute db with NULL value */
	res = db_exec(NULL);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* Try to parse mis-spelt query */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "CRAETE ATTRIBUTE %s DOMAIN int IN %s;", g_attribute_set[0], RELATION_NAME1);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* Create duplicate attribute */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "CREATE ATTRIBUTE %s DOMAIN int IN %s;", g_attribute_set[0], name);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* Index on non-existent attribute */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "CREATE INDEX %s.%s TYPE %s;", RELATION_NAME1, attr_name, INDEX_BPLUS);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	/* execute un-implemented operation */
	memset(query, 0, QUERY_LENGTH);
	snprintf(query, QUERY_LENGTH, "REMOVE ATTRIBUTE %s.%s;", RELATION_NAME1, g_attribute_set[0]);
	res = db_exec(query);
	TC_ASSERT_EQ("db_exec", DB_ERROR(res), true);

	TC_SUCCESS_RESULT();
}

/**
* @testcase         utc_arastorage_db_query_p
* @brief            Query a database
* @scenario         Select data and get returned cursor
* @apicovered       db_query
* @precondition     utc_arastorage_db_exec_p should be passed
* @postcondition    none
*/
static void utc_arastorage_db_query_p(void)
{
	db_result_t res;
	char query[QUERY_LENGTH];

	/* Select over bplus-tree index */
#ifdef CONFIG_ARCH_FLOAT_H
	snprintf(query, QUERY_LENGTH, "SELECT id, date, fruit, value, weight FROM %s WHERE value < 990;", RELATION_NAME1);

#else
	snprintf(query, QUERY_LENGTH, "SELECT id, date, fruit, value FROM %s WHERE value < 990;", RELATION_NAME1);
#endif
	g_cursor = db_query(query);
	TC_ASSERT_NEQ("db_query", g_cursor, NULL);

	res = db_cursor_free(g_cursor);
	TC_ASSERT_EQ("db_cursor_free", DB_SUCCESS(res), true);
	g_cursor = NULL;

	/* Select with AND condition */
	snprintf(query, QUERY_LENGTH, "SELECT id, date FROM %s WHERE id < 25 AND id > 10;", RELATION_NAME2);
	check_query_result(query);

	/* Count aggregation operation */
	snprintf(query, QUERY_LENGTH, "SELECT COUNT(id) FROM %s WHERE id = 5 OR id = 85;", RELATION_NAME2);
	check_query_result(query);

	/* Mean aggregation operation */
	snprintf(query, QUERY_LENGTH, "SELECT MEAN(id) FROM %s;", RELATION_NAME2);
	check_query_result(query);

	/* Sum aggregation operation */
	snprintf(query, QUERY_LENGTH, "SELECT SUM(id) FROM %s;", RELATION_NAME2);
	check_query_result(query);

	/* Max aggregation operation */
	snprintf(query, QUERY_LENGTH, "SELECT MAX(id) FROM %s;", RELATION_NAME2);
	check_query_result(query);

	/* Min aggregation operation */
	snprintf(query, QUERY_LENGTH, "SELECT MIN(id) FROM %s;", RELATION_NAME2);
	check_query_result(query);

	/* Remove operation */
	snprintf(query, QUERY_LENGTH, "REMOVE FROM %s WHERE date > 2000 AND date < 8000;", RELATION_NAME2);
	check_query_result(query);

	/* Select over inline index */

#ifdef CONFIG_ARCH_FLOAT_H
	snprintf(query, QUERY_LENGTH, "SELECT id, date, fruit, weight FROM %s WHERE id > 0;", RELATION_NAME1);

#else
	snprintf(query, QUERY_LENGTH, "SELECT id, date, fruit FROM %s WHERE id > 0;", RELATION_NAME1);
#endif
	g_cursor = db_query(query);
	TC_ASSERT_NEQ("db_query", g_cursor, NULL);

	TC_SUCCESS_RESULT();
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int utc_arastorage_main(int argc, char *argv[])
#endif
{
	if (testcase_state_handler(TC_START, "Arastorage UTC") == ERROR) {
		return ERROR;
	}

	/* Positive TCs */
	utc_arastorage_db_init_p();
	utc_arastorage_db_exec_p();
	utc_arastorage_db_query_p();

	db_init();

	/* Negative TCs */
	utc_arastorage_db_exec_n();
	utc_arastorage_db_query_n();

	cleanup();
	db_deinit();

	utc_database_dbcreate_1();

	(void)testcase_state_handler(TC_END, "Arastorage UTC");

	return 0;
}
