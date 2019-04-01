#ifndef __SECURITY_API_H__
#define  __SECURITY_API_H__

typedef struct _security_data {
	void *data;
	unsigned int length;
} security_data;

typedef enum {
	SECURITY_OK,

	SECURITY_ERROR,
	SECURITY_ALLOC_ERROR,
	SECURITY_INVALID_INPUT_PARAMS,
	SECURITY_INVALID_CERT_INDEX,
	SECURITY_INVALID_KEY_INDEX,
	SECURITY_INVALID_BUFFER_SIZE,
	SECURITY_MUTEX_INIT_ERROR,
	SECURITY_MUTEX_LOCK_ERROR,
	SECURITY_MUTEX_UNLOCK_ERROR,
	SECURITY_MUTEX_FREE_ERROR,

	SECURITY_WRITE_CERT_ERROR,
	SECURITY_READ_CERT_ERROR,
	SECURITY_GET_HASH_ERROR,
	SECURITY_GET_RANDOM_ERROR,
	SECURITY_ECDSA_SIGN_ERROR,
	SECURITY_ECDSA_VERIFY_ERROR,
	SECURITY_ECDH_COMPUTE_ERROR,

	SECURITY_NOT_SUPPORT,
} security_error;

typedef struct _security_csr {
	unsigned char 	issuer_country [128];
	unsigned char 	issuer_organization [128];
	unsigned char 	issuer_cn [128];
	unsigned char 	issuer_keyname [20];
	unsigned int 	issuer_algorithm;
	unsigned char 	subject_country [128];
	unsigned char 	subject_organization [128];
	unsigned char 	subject_cn [128];
	unsigned char 	subject_keyname [20];
	unsigned int 	subject_algorithm;
	unsigned int 	serial;
	unsigned int 	cert_years;
} security_csr;

typedef enum {
	RSASSA_PKCS1_V1_5,
	RSASSA_PKCS1_PSS_MGF1,
	RSASSA_UNKNOWN,
} security_rsa_algo;

typedef enum {
	ECDSA_BRAINPOOL_P256R1, 
	ECDSA_BRAINPOOL_P384R1,
	ECDSA_BRAINPOOL_P512R1,
	ECDSA_SEC_P192R1,
	ECDSA_SEC_P224R1,
	ECDSA_SEC_P256R1,
	ECDSA_SEC_P384R1,
	ECDSA_SEC_P521R1,
	ECDSA_UNKNOWN,
} security_ecdsa_curve;

typedef enum {
	DH_1024,
	DH_2048,
	DH_4096,
	DH_UNKNOWN,
} security_dh_key_type;

typedef enum {
	AES_128,
	AES_192,
	AES_256,
	RSA_1024,
	RSA_2048,
	RSA_3072,
	RSA_4096,
	ECC_BRAINPOOL_P256R1,
	ECC_BRAINPOOL_P384R1,
	ECC_BRAINPOOL_P512R1,
	ECC_SEC_P192R1,
	ECC_SEC_P224R1,
	ECC_SEC_P256R1,
	ECC_SEC_P384R1,
	ECC_SEC_P521R1,
	HMAC_MD5,
	HMAC_SHA1,
	HMAC_SHA224,
	HMAC_SHA256,
	HMAC_SHA384,
	HMAC_SHA512,
	ALGO_UNKNOWN,	
} security_algorithm;

typedef enum {
	AES_ECB_NOPAD,
	AES_ECB_ISO9797_M1,
	AES_ECB_ISO9797_M2,
	AES_ECB_PKCS5,
	AES_ECB_PKCS7,
	AES_CBC_NOPAD,
	AES_CBC_ISO9797_M1,
	AES_CBC_ISO9797_M2,
	AES_CBC_PKCS5,
	AES_CBC_PKCS7,
	AES_CTR,
	AES_UNKNWON,
} security_aes_algo;

typedef enum {
	HASH_MD5,
	HASH_SHA1,
	HASH_SHA224,
	HASH_SHA256,
	HASH_SHA384,
	HASH_SHA512,
	HASH_UNKNOWN,
} security_hash_type;

typedef struct _security_rsa_mode {
	security_rsa_algo rsa_a;
	security_hash_type hash_t;
	security_hash_type mgf;
	uint32_t salt_byte_len;
} security_rsa_mode;

typedef struct _security_aes_param {
	security_aes_algo mode;
	unsigned char *iv;
	unsigned int iv_len;
} security_aes_param;

typedef struct _security_ecdsa_mode {
	security_ecdsa_curve curve;
	security_hash_type hash_t;
} security_ecdsa_mode;

typedef struct _security_dh_data {
	security_dh_key_type mode;
	security_data *G;
	security_data *P;
	security_data *pubkey;
} security_dh_data;

typedef struct _security_ecdh_data {
	security_ecdsa_curve curve;
	security_data *pubkey_x;
	security_data *pubkey_y;
} security_ecdh_data;

typedef struct security_storage_file {
	char 	name [20];
	unsigned int 	attr;
} security_storage_file;

typedef security_storage_file* security_storage_list;

/**
 * Common
 */
int security_init(void);
int security_deinit(void);
int security_free_data(security_data *data);
int security_get_status(int *status);

/**
 * Authenticate
 */
int auth_generate_random(unsigned int size, security_data *random);
int auth_generate_certificate(const char *cert_name, security_csr *csr, security_data *cert);
int auth_set_certificate(const char *cert_name, security_data *cert);
int auth_get_certificate(const char *cert_name, security_data *cert);
int auth_remove_certificate(const char *cert_name);
int auth_get_rsa_signature(security_rsa_mode mode, const char *key_name, security_data *hash, security_data *sign);
int auth_verify_rsa_signature(security_rsa_mode mode, const char *key_name, security_data *hash, security_data *sign);
int auth_get_ecdsa_signature(security_ecdsa_mode mode, const char *key_name, security_data *hash, security_data *sign);
int auth_verify_ecdsa_signature(security_ecdsa_mode mode, const char *key_name, security_data *hash, security_data *sign);
int auth_get_hash(security_hash_type algo, security_data *data, security_data *hash);
int auth_get_hmac(security_algorithm algo, const char *key_name, security_data *data, security_data *hmac);
int auth_generate_dhparams(security_dh_data *params, security_data *pub);
int auth_set_dhparams(security_dh_data *params, security_data *pub);
int auth_compute_dhparams(security_dh_data *params, security_data *secret);
int auth_generate_ecdhkey(security_ecdh_data *params, security_data *pub);
int auth_compute_ecdhkey(security_ecdh_data *params, security_data *secret);

/**
 * Crypto
 */
int crypto_aes_encryption(security_aes_param mode, const char *key_name, security_data *input, security_data *output);
int crypto_aes_decryption(security_aes_param mode, const char *key_name, security_data *input, security_data *output);
int crypto_rsa_encryption(security_rsa_mode mode, const char *key_name, security_data *input, security_data *output);
int crypto_rsa_decryption(security_rsa_mode mode, const char *key_name, security_data *input, security_data *output);

/**
 * Secure Storage
 */
int ss_read_secure_storage(const char *name, unsigned int offset, security_data *data);
int ss_write_secure_storage(const char *name, unsigned int offsest, security_data *data);
int ss_delete_secure_storage(const char *name);
int ss_get_size_secure_storage(const char *name, unsigned int *size);
int ss_get_list_secure_storage(unsigned int *count, security_storage_list *list);

/**
 * Key Manager
 */
int keymgr_generate_key(security_algorithm algo, const char *key_name);
int keymgr_set_key(security_algorithm algo, const char *key_name, security_data *pubkey, security_data *prikey);
int keymgr_get_key(security_algorithm algo, const char *key_name, security_data *pubkey_x, security_data *pubkey_y);
int keymgr_remove_key(security_algorithm algo, const char *key_name);

#endif // __SECURITY_API_H__
