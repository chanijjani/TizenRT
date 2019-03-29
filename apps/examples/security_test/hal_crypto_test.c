#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <security/hal/security_hal.h>
#include <stress_tool/st_perf.h>
#include "hal_test_utils.h"

/*  Configuration */
#define HAL_CRYPTO_TEST_TRIAL 10
#define HAL_CRYPTO_TEST_LIMIT_TIME 100000000
#define HAL_CRYPTO_TEST_MEM_SIZE 4096

#define HAL_TEST_KEY_LEN 32
#define HAL_TEST_KEY_SLOT 1
#define HAL_TEST_AES_DATA_LEN 16
#define HAL_TEST_RSA_DATA_LEN 128

/*
 * Desc: Encrypt data using AES
 * Refered https://developer.artik.io/documentation/security-api/see-encdec-test_8c-example.html
 */
static hal_aes_param g_aes_param;
static hal_data g_aes_input;
static hal_data g_aes_output;
TEST_SETUP(aes_encrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_generate_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	g_aes_param.mode = HAL_AES_ECB_NOPAD;
	g_aes_param.iv = NULL;
	g_aes_param.iv_len = 0;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_input, HAL_CRYPTO_TEST_MEM_SIZE));
	memset(g_aes_input.data, 1, HAL_TEST_AES_DATA_LEN);
	g_aes_input.data_len = HAL_TEST_AES_DATA_LEN;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_output, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_END_TEST;
}

TEST_TEARDOWN(aes_encrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	hal_test_free_buffer(&g_aes_input);
	hal_test_free_buffer(&g_aes_output);
	if (g_aes_param.iv) {
		free(g_aes_param.iv);
	}

	ST_END_TEST;
}

TEST_F(aes_encrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_aes_encrypt(&g_aes_input, &g_aes_param, HAL_TEST_KEY_SLOT, &g_aes_output));

	ST_END_TEST;
}

/*
 * Desc: Decrypt data using AES
 * Refered https://developer.artik.io/documentation/security-api/see-encdec-test_8c-example.html
 */
static hal_data g_aes_final;
TEST_SETUP(aes_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_generate_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	g_aes_param.mode = HAL_AES_ECB_NOPAD;
	g_aes_param.iv = NULL;
	g_aes_param.iv_len = 0;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_input, HAL_CRYPTO_TEST_MEM_SIZE));
	memset(g_aes_input.data, 1, HAL_TEST_AES_DATA_LEN);
	g_aes_input.data_len = HAL_TEST_AES_DATA_LEN;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_output, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_EXPECT(0, hal_test_malloc_buffer(&g_aes_final, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_EXPECT(0, hal_aes_encrypt(&g_aes_input, &g_aes_param, HAL_TEST_KEY_SLOT, &g_aes_output));

	ST_END_TEST;
}

TEST_TEARDOWN(aes_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_AES_256, HAL_TEST_KEY_SLOT));

	hal_test_free_buffer(&g_aes_input);
	hal_test_free_buffer(&g_aes_output);
	hal_test_free_buffer(&g_aes_final);
	if (g_aes_param.iv) {
		free(g_aes_param.iv);
	}

	ST_END_TEST;
}

TEST_F(aes_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_aes_decrypt(&g_aes_output, &g_aes_param, HAL_TEST_KEY_SLOT, &g_aes_final));
	ST_EXPECT(0, memcmp(g_aes_input.data, g_aes_final.data, g_aes_final.data_len));

	ST_END_TEST;
}

/*
 * Desc: Encrypt data using RSA
 * Refered https://developer.artik.io/documentation/security-api/see-encdec-test_8c-example.html
 */
static hal_data g_rsa_input;
static hal_data g_rsa_output;
static hal_rsa_mode g_rsa_mode;
TEST_SETUP(rsa_encrypt)
{
	ST_START_TEST;
	
	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_generate_key(HAL_KEY_RSA_2048, HAL_TEST_KEY_SLOT));

	ST_EXPECT(0, hal_test_malloc_buffer(&g_rsa_input, HAL_CRYPTO_TEST_MEM_SIZE));
	memset(g_rsa_input.data, 1, HAL_TEST_RSA_DATA_LEN);
	g_rsa_input.data_len = HAL_TEST_RSA_DATA_LEN;

	g_rsa_mode.rsa_a = HAL_RSASSA_PKCS1_PSS_MGF1;
	g_rsa_mode.hash_t = HAL_HASH_SHA256;
	g_rsa_mode.salt_byte_len = 64;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_rsa_output, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_END_TEST;
}

TEST_TEARDOWN(rsa_encrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_RSA_2048, HAL_TEST_KEY_SLOT));

	hal_test_free_buffer(&g_rsa_input);
	hal_test_free_buffer(&g_rsa_output);

	ST_END_TEST;
}

TEST_F(rsa_encrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_rsa_encrypt(&g_rsa_input, &g_rsa_mode, HAL_TEST_KEY_SLOT, &g_rsa_output));

	ST_END_TEST;
}

/*
 * Desc: Decrypt data using RSA
 * Refered https://developer.artik.io/documentation/security-api/see-encdec-test_8c-example.html
 */
static hal_data g_rsa_final;
TEST_SETUP(rsa_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_generate_key(HAL_KEY_RSA_2048, HAL_TEST_KEY_SLOT));

	ST_EXPECT(0, hal_test_malloc_buffer(&g_rsa_input, HAL_CRYPTO_TEST_MEM_SIZE));
	memset(g_rsa_input.data, 1, HAL_TEST_RSA_DATA_LEN);
	g_rsa_input.data_len = HAL_TEST_RSA_DATA_LEN;

	g_rsa_mode.rsa_a = HAL_RSASSA_PKCS1_PSS_MGF1;
	g_rsa_mode.hash_t = HAL_HASH_SHA256;
	g_rsa_mode.salt_byte_len = 64;

	ST_EXPECT(0, hal_test_malloc_buffer(&g_rsa_output, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_EXPECT(0, hal_test_malloc_buffer(&g_rsa_final, HAL_CRYPTO_TEST_MEM_SIZE));

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_rsa_encrypt(&g_rsa_input, &g_rsa_mode, HAL_TEST_KEY_SLOT, &g_rsa_output));

	ST_END_TEST;
}

TEST_TEARDOWN(rsa_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_remove_key(HAL_KEY_RSA_2048, HAL_TEST_KEY_SLOT));

	hal_test_free_buffer(&g_rsa_input);
	hal_test_free_buffer(&g_rsa_output);
	hal_test_free_buffer(&g_rsa_final);

	ST_END_TEST;
}

TEST_F(rsa_decrypt)
{
	ST_START_TEST;

	ST_EXPECT_2(HAL_SUCCESS, HAL_NOT_SUPPORTED, hal_rsa_decrypt(&g_rsa_output, &g_rsa_mode, HAL_TEST_KEY_SLOT, &g_rsa_final));
	ST_EXPECT(0, memcmp(g_rsa_input.data, g_rsa_final.data, g_rsa_final.data_len));

	ST_END_TEST;
}

ST_SET_SMOKE_TAIL(HAL_CRYPTO_TEST_TRIAL, HAL_CRYPTO_TEST_LIMIT_TIME, "Encrypt data using AES", aes_encrypt);
ST_SET_SMOKE(HAL_CRYPTO_TEST_TRIAL, HAL_CRYPTO_TEST_LIMIT_TIME, "Decrypt data using AES", aes_decrypt, aes_encrypt);
ST_SET_SMOKE(HAL_CRYPTO_TEST_TRIAL, HAL_CRYPTO_TEST_LIMIT_TIME, "Encrypt data using RSA", rsa_encrypt, aes_decrypt);
ST_SET_SMOKE(HAL_CRYPTO_TEST_TRIAL, HAL_CRYPTO_TEST_LIMIT_TIME, "Decrypt data using RSA", rsa_decrypt, rsa_encrypt);
ST_SET_PACK(hal_crypto, rsa_decrypt);

int hal_crypto_test(void)
{
	ST_RUN_TEST(hal_crypto);
	ST_RESULT_TEST(hal_crypto);

	return 0;
}
