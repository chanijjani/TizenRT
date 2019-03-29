/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
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

#include <tinyara/config.h>

#include <stdio.h>
#include <stdlib.h>

#include <tinyara/seclink.h>


/**
 * Debugging
 */
#define SECTEST_OK 														                    \
	do {																                    \
		printf("[OK] %s\t%s:%d (%d)\n", __FUNCTION__, __FILE__, __LINE__, sectest_res);	    \
	} while (0)
#define SECTEST_ERR														                    \
	do {																                    \
		printf("[ERR] %s\t%s:%d (%d)\n", __FUNCTION__, __FILE__, __LINE__, sectest_res);	\
	} while (0)

#define SECTEST_CALL(func)                      \
    do {                                        \
		printf("\n");                           \
        int sectest_res = func;                 \
        if (sectest_res != SECLINK_OK) {        \
            SECTEST_ERR;                        \
        } else {                                \
			SECTEST_OK;                         \
		}                                       \
    } while (0)

#define SECTEST_KEY_IDX 1
#define SECTEST_FACTORY_KEY_IDX 2
#define SECTEST_FACTORY_CERT_IDX 3
#define SECTEST_FACTORY_DATA_IDX 4
#define SECTEST_MEM_SIZE 4096

int seclink_test(void)
{
	sl_ctx hnd;
	hal_data output;

	char *pubkey_data = {"public_sssssssssssstttttttttt"};
	char *hash_data = {"hash_sssssssssssstttttttttt"};
	char *hmac_data = {"hash_sssssssssssstttttttttt"};
	char *rsahash_data = {"rsahash_sssssssssssstttttttttt"};
	char *sign_data = {"sign_sssssssssssstttttttttt"};
	char *dhg_data = {"dh_G_sssssssssssstttttttttt"};
	char *dhp_data = {"dh_P_sssssssssssstttttttttt"};
	char *cert_data = {"certificate_sssssssssssstttttttttt"};
	char *dec_data = {"decrypted_sssssssssssstttttttttt"};
	char *enc_data = {"encrypted_sssssssssssstttttttttt"};
	char *iv_data = {"iv_sssssssssssstttttttttt"};
	char *ss_data = {"securestorage_sssssssssssstttttttttt"};

	hal_data pubkey;
	pubkey.data = pubkey_data;
	pubkey.data_len = sizeof(pubkey_data);

	hal_data hash;
	hash.data = hash_data;
	hash.data_len = sizeof(hash_data);

	hal_data hmac;
	hmac.data = hmac_data;
	hmac.data_len = sizeof(hmac_data);

	hal_data rsahash;
	rsahash.data = rsahash_data;
	rsahash.data_len = sizeof(rsahash_data);

	hal_data sign;
	sign.data = sign_data;
	sign.data_len = sizeof(sign_data);

	hal_data dhg;
	dhg.data = dhg_data;
	dhg.data_len = sizeof(dhg_data);

	hal_data dhp;
	dhp.data = dhp_data;
	dhp.data_len = sizeof(dhp_data);

	hal_data cert;
	cert.data = cert_data;
	cert.data_len = sizeof(cert_data);

	hal_data dec;
	dec.data = dec_data;
	dec.data_len = sizeof(dec_data);

	hal_data enc;
	enc.data = enc_data;
	enc.data_len = sizeof(enc_data);

	hal_data ss;
	ss.data = ss_data;
	ss.data_len = sizeof(ss_data);


	/*  Initialize handle */
	SECTEST_CALL(sl_init(&hnd));

    /*
	 * Key manager
	 */
	SECTEST_CALL(sl_set_key(hnd, HAL_KEY_AES_128, SECTEST_KEY_IDX, &pubkey, NULL));

	SECTEST_CALL(sl_get_key(hnd, HAL_KEY_AES_128, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_remove_key(hnd, HAL_KEY_AES_128, SECTEST_KEY_IDX));

	SECTEST_CALL(sl_generate_key(hnd, HAL_KEY_ECC_SEC_P256R1, SECTEST_KEY_IDX));

	SECTEST_CALL(sl_remove_key(hnd, HAL_KEY_ECC_SEC_P256R1, SECTEST_KEY_IDX));
	/*
	 * Authenticate
	 */
	#define SECTEST_RANDOM_LEN 32
	SECTEST_CALL(sl_generate_random(hnd, SECTEST_RANDOM_LEN, &output));

	SECTEST_CALL(sl_get_hash(hnd, HAL_HASH_SHA256, &hash, &output));

	SECTEST_CALL(sl_get_hmac(hnd, HAL_HMAC_SHA256, &hmac, SECTEST_KEY_IDX, &output));

	hal_rsa_mode rsa_mode = {HAL_RSASSA_PKCS1_PSS_MGF1, HAL_HASH_SHA256, HAL_HASH_SHA256, 32};
	SECTEST_CALL(sl_rsa_sign_md(hnd, rsa_mode, &rsahash, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_rsa_verify_md(hnd, rsa_mode, &rsahash, &sign, SECTEST_KEY_IDX));

	hal_ecdsa_mode ecdsa_mode = {HAL_ECDSA_BRAINPOOL_P512R1, HAL_HASH_SHA256};
	SECTEST_CALL(sl_ecdsa_sign_md(hnd, ecdsa_mode, &hash, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_ecdsa_verify_md(hnd, ecdsa_mode, &hash, &sign, SECTEST_KEY_IDX));

	hal_data pubkey_tmp = {0,};
	hal_dh_data dh_mode = {HAL_DH_1024, &dhg, &dhp, &pubkey_tmp};
	SECTEST_CALL(sl_dh_generate_param(hnd, SECTEST_KEY_IDX, &dh_mode));

	SECTEST_CALL(sl_dh_compute_shared_secret(hnd, &dh_mode, SECTEST_KEY_IDX, &output));

	hal_ecdh_data ecdh_mode = {HAL_ECDSA_BRAINPOOL_P512R1, NULL, NULL};
	SECTEST_CALL(sl_ecdh_compute_shared_secret(hnd, &ecdh_mode, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_set_certificate(hnd, SECTEST_KEY_IDX, &cert));

	SECTEST_CALL(sl_get_certificate(hnd, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_remove_certificate(hnd, SECTEST_KEY_IDX));

	SECTEST_CALL(sl_get_factory_key(hnd, SECTEST_FACTORY_KEY_IDX, &output));
	SECTEST_CALL(sl_get_factory_cert(hnd, SECTEST_FACTORY_CERT_IDX, &output));
	SECTEST_CALL(sl_get_factory_data(hnd, SECTEST_FACTORY_DATA_IDX, &output));

	/*
	 * Crypto
	 */
	hal_aes_param aes_param = {HAL_AES_ECB_PKCS5, (unsigned char *)&iv_data, sizeof(iv_data)};
	SECTEST_CALL(sl_aes_encrypt(hnd, &dec, &aes_param, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_aes_decrypt(hnd, &enc, &aes_param, SECTEST_KEY_IDX, &output));

	hal_rsa_mode crypto_rsa_mode = {HAL_RSASSA_PKCS1_PSS_MGF1, HAL_HASH_SHA256};
	SECTEST_CALL(sl_rsa_encrypt(hnd, &dec, &crypto_rsa_mode, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_rsa_decrypt(hnd, &enc, &crypto_rsa_mode, SECTEST_KEY_IDX, &output));

	/*
	 * Secure Storage
	 */
	SECTEST_CALL(sl_write_storage(hnd, SECTEST_KEY_IDX, &ss));

	SECTEST_CALL(sl_read_storage(hnd, SECTEST_KEY_IDX, &output));

	SECTEST_CALL(sl_delete_storage(hnd, SECTEST_KEY_IDX));

	/*  Terminate handle */
	SECTEST_CALL(sl_deinit(hnd));

	return 0;
}
