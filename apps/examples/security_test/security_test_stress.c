/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License\n");
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

#include "mbedtls/base64.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/timing.h"

#include <stress_tool/st_perf.h>
#include <stdio.h>
#include <string.h>

#include "security_hal.h"
#include "mbedtls/see_api.h"
#include "mbedtls/see_misc.h"

/*
 * Macro
 */
#define SECURITY_HAL_TEST_TRIAL	   CONFIG_SECURITYHAL_TEST_TRIAL

/*
 * Global
 */
struct sAES_PARAM *g_aes_param;
hal_aes_param *g_aes_hal_param;
hal_data *g_dec_data;
hal_data *g_enc_data;

/*
 * hal_aes_encrypt
 */
unsigned char g_aes_input[150] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
								   1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
								   1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
								   1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
								   1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0
								 };

unsigned char g_aes_output[150];
unsigned char g_aes_encrypted_data[150];
unsigned int g_aes_size[2] = { 16, 32 };

int hal_aes_encrypt(_IN_ hal_data *dec_data, _IN_ hal_aes_param *aes_hal_param, _IN_ uint32_t key_idx, _OUT_ hal_data *enc_data)
{
	int ret = 0;

	g_aes_param = (struct sAES_PARAM *)malloc(sizeof(struct sAES_PARAM));
	g_aes_param->u32Mode = aes_hal_param->mode;
	g_aes_param->pu8Plaintext = dec_data->data;
	g_aes_param->u32Plaintext_byte_len = dec_data->data_len;
	g_aes_param->pu8Ciphertext = enc_data->data;
	g_aes_param->u32Ciphertext_byte_len = enc_data->data_len;

	ret = isp_aes_encrypt_securekey(g_aes_param, key_idx);

	free(g_aes_param);

	return ret;
}

int hal_aes_decrypt(_IN_ hal_data *enc_data, _IN_ hal_aes_param *aes_hal_param, _IN_ uint32_t key_idx, _OUT_ hal_data *dec_data)
{
	int ret = 0;

	g_aes_param = (struct sAES_PARAM *)malloc(sizeof(struct sAES_PARAM));
	g_aes_param->u32Mode = aes_hal_param->mode;
	g_aes_param->pu8Ciphertext = enc_data->data;
	g_aes_param->u32Ciphertext_byte_len = enc_data->data_len;
	g_aes_param->pu8Plaintext = dec_data->data;
	g_aes_param->u32Plaintext_byte_len = dec_data->data_len;

	ret = isp_aes_decrypt_securekey(g_aes_param, key_idx);

	free(g_aes_param);

	return ret;
}

TEST_SETUP(hal_aes_encrypt)
{
	ST_START_TEST;

	int ret = 0;

	ret = isp_set_securekey(g_aes_input, g_aes_size[0], SECURE_STORAGE_TYPE_KEY_AES, 0);
	if (ret) {
		return STRESS_TC_FAIL;
	}

	g_aes_hal_param = (hal_aes_param *)malloc(sizeof(hal_aes_param));
	g_dec_data = (hal_data *)malloc(sizeof(hal_data));
	g_enc_data = (hal_data *)malloc(sizeof(hal_data));
	g_aes_hal_param->mode = AES_ECB_MODE;
	g_dec_data->data = g_aes_input;
	g_dec_data->data_len = g_aes_size[0];	// Need to test i = 1 also
	g_enc_data->data = g_aes_encrypted_data;
	g_enc_data->data_len = g_aes_size[0];

	ST_END_TEST;
}

TEST_TEARDOWN(hal_aes_encrypt)
{
	ST_START_TEST;

	free(g_aes_hal_param);
	free(g_dec_data);
	free(g_enc_data);

	ST_END_TEST;
}

TEST_F(hal_aes_encrypt)
{
	ST_START_TEST;

	ST_EXPECT(0, hal_aes_encrypt(g_dec_data, g_aes_hal_param, 0, g_enc_data));

	ST_END_TEST;
}

/*
 * hal_aes_decrypt
 */
unsigned char aes_check[150];

TEST_SETUP(hal_aes_decrypt)
{
	ST_START_TEST;

	int ret = 0;
	ret = isp_set_securekey(g_aes_input, g_aes_size[0], SECURE_STORAGE_TYPE_KEY_AES, 0);
	if (ret) {
		return STRESS_TC_FAIL;
	}

	g_aes_hal_param = (hal_aes_param *)malloc(sizeof(hal_aes_param));
	g_enc_data = (hal_data *)malloc(sizeof(hal_data));
	g_dec_data = (hal_data *)malloc(sizeof(hal_data));
	g_aes_hal_param->mode = AES_ECB_MODE;
	g_enc_data->data = g_aes_encrypted_data;
	g_enc_data->data_len = g_aes_size[0];	// Need to test i = 1 also
	g_dec_data->data = g_aes_output;
	g_dec_data->data_len = g_aes_size[0];

	ST_END_TEST;
}

TEST_TEARDOWN(hal_aes_decrypt)
{
	ST_START_TEST;

	free(g_aes_hal_param);
	free(g_dec_data);
	free(g_enc_data);

	ST_END_TEST;
}

TEST_F(hal_aes_decrypt)
{
	ST_START_TEST;

	hal_aes_decrypt(g_enc_data, g_aes_hal_param, 0, g_dec_data);
	ST_EXPECT(0, memcmp(g_aes_input, g_dec_data->data, g_aes_size[0]));

	ST_END_TEST;
}

/*
 * hal_rsa_encrypt
 */
TEST_SETUP(hal_rsa_encrypt)
{
	ST_START_TEST;

//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_init(&g_wifi_callbacks));
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_connect_ap(&apconfig));

	ST_END_TEST;
}

TEST_TEARDOWN(hal_rsa_encrypt)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_deinit());
	ST_END_TEST;
}

TEST_F(hal_rsa_encrypt)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_disconnect_ap());

	ST_END_TEST;
}

/*
 * hal_rsa_decrypt
 */
TEST_SETUP(hal_rsa_decrypt)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_init(&g_wifi_callbacks));
	ST_END_TEST;
}

TEST_TEARDOWN(hal_rsa_decrypt)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_deinit());
	ST_END_TEST;
}

TEST_F(hal_rsa_decrypt)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_scan_ap());
	ST_END_TEST;
}

/*
 * hal_read_storage
 */
TEST_SETUP(hal_read_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_init(&g_wifi_callbacks));
	ST_END_TEST;
}

TEST_TEARDOWN(hal_read_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_deinit());
	ST_END_TEST;
}

TEST_F(hal_read_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_set_mode(SOFTAP_MODE, &ap_config));
	ST_END_TEST;
}

/*
 * hal_write_storage
 */
TEST_SETUP(hal_write_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_init(&g_wifi_callbacks));
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_set_mode(SOFTAP_MODE, &ap_config));
	ST_END_TEST;
}

TEST_TEARDOWN(hal_write_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_deinit());
	ST_END_TEST;
}

TEST_F(hal_write_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_scan_ap());
	ST_END_TEST;
}

/*
 * hal_delete_storage
 */
TEST_SETUP(hal_delete_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_init(&g_wifi_callbacks));
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_set_mode(SOFTAP_MODE, &ap_config));
	ST_END_TEST;
}

TEST_TEARDOWN(hal_delete_storage)
{
	ST_START_TEST;
	ST_END_TEST;
}

TEST_F(hal_delete_storage)
{
	ST_START_TEST;
//  ST_EXPECT(WIFI_MANAGER_SUCCESS, wifi_manager_deinit());
	ST_END_TEST;
}

// Crypto.
ST_SET_SMOKE_TAIL(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_aes_encrypt", hal_aes_encrypt);
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_aes_decrypt", hal_aes_decrypt, hal_aes_encrypt);
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_rsa_encrypt", hal_rsa_encrypt, hal_aes_decrypt);
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_rsa_decrypt", hal_rsa_decrypt, hal_rsa_encrypt);
//Secure Storage
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_read_storage", hal_read_storage, hal_rsa_decrypt);
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_write_storage", hal_write_storage, hal_read_storage);
ST_SET_SMOKE(SECURITY_HAL_TEST_TRIAL, 5000000, "hal_delete_storage", hal_delete_storage, hal_write_storage);
ST_SET_PACK(security_hal, hal_delete_storage);

void security_hal_run_stress_test(void *arg)
{
	ST_RUN_TEST(security_hal);
	ST_RESULT_TEST(security_hal);
}
