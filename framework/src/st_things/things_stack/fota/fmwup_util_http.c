/* ****************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "fmwup_util_http.h"
#include <apps/system/fota_hal.h>
#include <json/cJSON.h>
#include <protocols/webclient.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "memory/things_malloc.h"
#include "logging/things_logger.h"

#define TAG "[things_fota]"

#define FOTA_WEBCLIENT_BUF_SIZE     4600
#define FOTA_REC_JSON_SIZE		1000

#define KEY_URL "url"

const char c_ca_crt_rsa[] =
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDhzCCAm+gAwIBAgIBADANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"
	"MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"
	"MTEwMjEyMTQ0NDAwWhcNMjEwMjEyMTQ0NDAwWjA7MQswCQYDVQQGEwJOTDERMA8G\r\n"
	"A1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwggEiMA0G\r\n"
	"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA3zf8F7vglp0/ht6WMn1EpRagzSHx\r\n"
	"mdTs6st8GFgIlKXsm8WL3xoemTiZhx57wI053zhdcHgH057Zk+i5clHFzqMwUqny\r\n"
	"50BwFMtEonILwuVA+T7lpg6z+exKY8C4KQB0nFc7qKUEkHHxvYPZP9al4jwqj+8n\r\n"
	"YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\r\n"
	"R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\r\n"
	"KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\r\n"
	"gZUwgZIwDAYDVR0TBAUwAwEB/zAdBgNVHQ4EFgQUtFrkpbPe0lL2udWmlQ/rPrzH\r\n"
	"/f8wYwYDVR0jBFwwWoAUtFrkpbPe0lL2udWmlQ/rPrzH/f+hP6Q9MDsxCzAJBgNV\r\n"
	"BAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEZMBcGA1UEAxMQUG9sYXJTU0wgVGVz\r\n"
	"dCBDQYIBADANBgkqhkiG9w0BAQUFAAOCAQEAuP1U2ABUkIslsCfdlc2i94QHHYeJ\r\n"
	"SsR4EdgHtdciUI5I62J6Mom+Y0dT/7a+8S6MVMCZP6C5NyNyXw1GWY/YR82XTJ8H\r\n"
	"DBJiCTok5DbZ6SzaONBzdWHXwWwmi5vg1dxn7YxrM9d0IjxM27WNKs4sDQhZBQkF\r\n"
	"pjmfs2cb4oPl4Y9T9meTx/lvdkRYEug61Jfn6cA+qHpyPYdTH+UshITnmp5/Ztkf\r\n"
	"m/UTSLBNFNHesiTZeH31NcxYGdHSme9Nc/gfidRa0FLOCfWxRlFqAI47zG9jAQCZ\r\n"
	"7Z2mCGDNMhjQc+BYcdnl0lPXjdDK6V0qCg1dVewhUBcW5gZKzV7e9+DpVA==\r\n"
	"-----END CERTIFICATE-----\r\n";

const char c_cli_crt_rsa[] =
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDPzCCAiegAwIBAgIBBDANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"
	"MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"
	"MTEwMjEyMTQ0NDA3WhcNMjEwMjEyMTQ0NDA3WjA8MQswCQYDVQQGEwJOTDERMA8G\r\n"
	"A1UEChMIUG9sYXJTU0wxGjAYBgNVBAMTEVBvbGFyU1NMIENsaWVudCAyMIIBIjAN\r\n"
	"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyHTEzLn5tXnpRdkUYLB9u5Pyax6f\r\n"
	"M60Nj4o8VmXl3ETZzGaFB9X4J7BKNdBjngpuG7fa8H6r7gwQk4ZJGDTzqCrSV/Uu\r\n"
	"1C93KYRhTYJQj6eVSHD1bk2y1RPD0hrt5kPqQhTrdOrA7R/UV06p86jt0uDBMHEw\r\n"
	"MjDV0/YI0FZPRo7yX/k9Z5GIMC5Cst99++UMd//sMcB4j7/Cf8qtbCHWjdmLao5v\r\n"
	"4Jv4EFbMs44TFeY0BGbH7vk2DmqV9gmaBmf0ZXH4yqSxJeD+PIs1BGe64E92hfx/\r\n"
	"/DZrtenNLQNiTrM9AM+vdqBpVoNq0qjU51Bx5rU2BXcFbXvI5MT9TNUhXwIDAQAB\r\n"
	"o00wSzAJBgNVHRMEAjAAMB0GA1UdDgQWBBRxoQBzckAvVHZeM/xSj7zx3WtGITAf\r\n"
	"BgNVHSMEGDAWgBS0WuSls97SUva51aaVD+s+vMf9/zANBgkqhkiG9w0BAQUFAAOC\r\n"
	"AQEAAn86isAM8X+mVwJqeItt6E9slhEQbAofyk+diH1Lh8Y9iLlWQSKbw/UXYjx5\r\n"
	"LLPZcniovxIcARC/BjyZR9g3UwTHNGNm+rwrqa15viuNOFBchykX/Orsk02EH7NR\r\n"
	"Alw5WLPorYjED6cdVQgBl9ot93HdJogRiXCxErM7NC8/eP511mjq+uLDjLKH8ZPQ\r\n"
	"8I4ekHJnroLsDkIwXKGIsvIBHQy2ac/NwHLCQOK6mfum1pRx52V4Utu5dLLjD5bM\r\n"
	"xOBC7KU4xZKuMXXZM6/93Yb51K/J4ahf1TxJlTWXtnzDr9saEYdNy2SKY/6ZiDNH\r\n"
	"D+stpAKiQLAWaAusIWKYEyw9MQ==\r\n"
	"-----END CERTIFICATE-----\r\n";

const char c_cli_key_rsa[] =
	"-----BEGIN RSA PRIVATE KEY-----\r\n"
	"MIIEpAIBAAKCAQEAyHTEzLn5tXnpRdkUYLB9u5Pyax6fM60Nj4o8VmXl3ETZzGaF\r\n"
	"B9X4J7BKNdBjngpuG7fa8H6r7gwQk4ZJGDTzqCrSV/Uu1C93KYRhTYJQj6eVSHD1\r\n"
	"bk2y1RPD0hrt5kPqQhTrdOrA7R/UV06p86jt0uDBMHEwMjDV0/YI0FZPRo7yX/k9\r\n"
	"Z5GIMC5Cst99++UMd//sMcB4j7/Cf8qtbCHWjdmLao5v4Jv4EFbMs44TFeY0BGbH\r\n"
	"7vk2DmqV9gmaBmf0ZXH4yqSxJeD+PIs1BGe64E92hfx//DZrtenNLQNiTrM9AM+v\r\n"
	"dqBpVoNq0qjU51Bx5rU2BXcFbXvI5MT9TNUhXwIDAQABAoIBAGdNtfYDiap6bzst\r\n"
	"yhCiI8m9TtrhZw4MisaEaN/ll3XSjaOG2dvV6xMZCMV+5TeXDHOAZnY18Yi18vzz\r\n"
	"4Ut2TnNFzizCECYNaA2fST3WgInnxUkV3YXAyP6CNxJaCmv2aA0yFr2kFVSeaKGt\r\n"
	"ymvljNp2NVkvm7Th8fBQBO7I7AXhz43k0mR7XmPgewe8ApZOG3hstkOaMvbWAvWA\r\n"
	"zCZupdDjZYjOJqlA4eEA4H8/w7F83r5CugeBE8LgEREjLPiyejrU5H1fubEY+h0d\r\n"
	"l5HZBJ68ybTXfQ5U9o/QKA3dd0toBEhhdRUDGzWtjvwkEQfqF1reGWj/tod/gCpf\r\n"
	"DFi6X0ECgYEA4wOv/pjSC3ty6TuOvKX2rOUiBrLXXv2JSxZnMoMiWI5ipLQt+RYT\r\n"
	"VPafL/m7Dn6MbwjayOkcZhBwk5CNz5A6Q4lJ64Mq/lqHznRCQQ2Mc1G8eyDF/fYL\r\n"
	"Ze2pLvwP9VD5jTc2miDfw+MnvJhywRRLcemDFP8k4hQVtm8PMp3ZmNECgYEA4gz7\r\n"
	"wzObR4gn8ibe617uQPZjWzUj9dUHYd+in1gwBCIrtNnaRn9I9U/Q6tegRYpii4ys\r\n"
	"c176NmU+umy6XmuSKV5qD9bSpZWG2nLFnslrN15Lm3fhZxoeMNhBaEDTnLT26yoi\r\n"
	"33gp0mSSWy94ZEqipms+ULF6sY1ZtFW6tpGFoy8CgYAQHhnnvJflIs2ky4q10B60\r\n"
	"ZcxFp3rtDpkp0JxhFLhiizFrujMtZSjYNm5U7KkgPVHhLELEUvCmOnKTt4ap/vZ0\r\n"
	"BxJNe1GZH3pW6SAvGDQpl9sG7uu/vTFP+lCxukmzxB0DrrDcvorEkKMom7ZCCRvW\r\n"
	"KZsZ6YeH2Z81BauRj218kQKBgQCUV/DgKP2985xDTT79N08jUo3hTP5MVYCCuj/+\r\n"
	"UeEw1TvZcx3LJby7P6Xad6a1/BqveaGyFKIfEFIaBUBItk801sDDpDaYc4gL00Xc\r\n"
	"7lFuBHOZkxJYlss5QrGpuOEl9ZwUt5IrFLBdYaKqNHzNVC1pCPfb/JyH6Dr2HUxq\r\n"
	"gxUwAQKBgQCcU6G2L8AG9d9c0UpOyL1tMvFe5Ttw0KjlQVdsh1MP6yigYo9DYuwu\r\n"
	"bHFVW2r0dBTqegP2/KTOxKzaHfC1qf0RGDsUoJCNJrd1cwoCLG8P2EF4w3OBrKqv\r\n"
	"8u4ytY0F+Vlanj5lm3TaoHSVF1+NWPyOTiwevIECGKwSxvlki4fDAA==\r\n"
	"-----END RSA PRIVATE KEY-----\r\n";

struct http_client_ssl_config_t g_config = {
	(char *)c_ca_crt_rsa, 
	(char *)c_cli_crt_rsa, 
	(char *)c_cli_key_rsa,
	sizeof(c_ca_crt_rsa), 
	sizeof(c_cli_crt_rsa), 
	sizeof(c_cli_key_rsa), 
	2
};

static int g_https;
static bool g_finish = false;

typedef enum {
	FOTA_DOWNLOAD_STATE_NONE = 0,
	FOTA_DOWNLOAD_STATE_JSON,
	FOTA_DOWNLOAD_STATE_BINARY,
	FOTA_DOWNLOAD_STATE_DONE
} fota_download_state_e;

fota_download_state_e download_state;

static const char headerfield_connect[] = "Connect";
static const char headerfield_close[] = "close";
static const char headerfield_useragent[] = "User-Agent";
static const char headerfield_tinyara[] = "TinyARA";

static const char str_wget[] = "WGET";
static const char str_get[] = "GET";

char *json_str;
fotahal_handle_t fotahal_handle;
unsigned int recv_size = 0;
unsigned int total_size = 0;
static bool	is_link_fail;

static void callback(struct http_client_response_t *response)
{
	if (response->status != 200) {
		is_link_fail = true;
		THINGS_LOG_D(THINGS_ERROR, TAG, "recv callback status %d", response->status);
		return;
	}

	if (download_state == FOTA_DOWNLOAD_STATE_JSON) {
		strncpy(json_str + recv_size, response->entity, response->entity_len);
	} else if (download_state == FOTA_DOWNLOAD_STATE_BINARY) {
		fotahal_write(fotahal_handle, response->entity, response->entity_len);
	}
	recv_size += response->entity_len;
	total_size = response->total_len;
}

/****************************************************************************
 * Name: wget_main
 ****************************************************************************/

int webclient_init_request(char *url, struct http_client_request_t *request)
{
	int ret = -1;

	memset(request, 0, sizeof(struct http_client_request_t));

	request->method = WGET_MODE_GET;	
	request->url = (char *)things_malloc(strlen(url) + 1);
	if (!request->url) {
		return ret;
	}
	strncpy(request->url, url, strlen(url));
	request->url[strlen(url)] = '\0';

#ifdef CONFIG_NET_SECURITY_TLS
	if (!strncmp(request->url, "https", 5)) {
		g_https = 1;
	} else
#endif
	if (!strncmp(request->url, "http", 4)) {
		g_https = 0;
	} else {
		return ret;
	}

	request->buflen = FOTA_WEBCLIENT_BUF_SIZE;
	ret = 0;

	return ret;
}

int wget_from_url(char *download_url) 
{
	THINGS_LOG_D(THINGS_INFO, TAG, "download url : %s\n", download_url);

	struct http_client_request_t request;
	struct http_keyvalue_list_t headers;
	struct http_client_response_t response;
	struct http_client_ssl_config_t *ssl_config = NULL;

	int ret;
	ret = -1;

	if (webclient_init_request(download_url, &request) != 0) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "webclient_init_request error");
		return NULL;
	}

	ssl_config = g_https ? &g_config : NULL;

	/* before sending request,
	 * must initialize keyvalue list for request headers
	 */
	http_keyvalue_list_init(&headers);
	http_keyvalue_list_add(&headers, headerfield_connect, headerfield_close);
	http_keyvalue_list_add(&headers, headerfield_useragent, headerfield_tinyara);
	request.headers = &headers;

	/* before sending request by sync function,
	 * must initialize response structure
	 */
	if (http_client_send_request_async(&request, ssl_config, (wget_callback_t)callback)) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "fail to send request");
		goto release_out;
	}
	/* sleep for end request */
	while (request.async_flag > 0) {
		usleep(100000);
	}

	if (request.async_flag < 0) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "fail to send request");
		goto release_out;
	}

	ret = 1;

release_out:
	/* before finish of app,
	 * must release keyvalue list for request headers
	 */
	http_keyvalue_list_release(&headers);
	THINGS_LOG_D(THINGS_DEBUG, TAG, "end request");

	return ret;
}

int fmwup_http_download_file(const char *download_url)
{
	THINGS_LOG_D(THINGS_DEBUG, TAG, THINGS_FUNC_ENTRY);

	download_state = FOTA_DOWNLOAD_STATE_JSON;
	json_str = (char *)things_malloc(FOTA_REC_JSON_SIZE); 
	recv_size = 0;
	
	is_link_fail = false;

	// parsing json
	if (wget_from_url(download_url) < 0) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "wget_from_url error");
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	json_str[recv_size] = 0;

	if (is_link_fail) {
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	THINGS_LOG_D(THINGS_DEBUG, TAG, "[recv:JSON] state : %d / recv_size : %u / total size : %u / json = %s", download_state, recv_size, total_size, json_str);

	if (recv_size != total_size) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "[recv:JSON] file size error");
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	fotahal_handle = fotahal_open();
	if (fotahal_handle == NULL) {
		fotahal_close(fotahal_handle);
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}
	download_state = FOTA_DOWNLOAD_STATE_BINARY;


	cJSON *root = cJSON_Parse((const char *)json_str);
	cJSON *url = cJSON_GetObjectItem(root, KEY_URL);
	recv_size = 0;

	is_link_fail = false;

	if (wget_from_url(url->valuestring) < 0) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "wget_from_url error");

		if (root != NULL) {
			cJSON_Delete(root);
		}
		fotahal_erase(fotahal_handle);
		fotahal_close(fotahal_handle);
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	if (is_link_fail) {
		things_free(json_str);
		fotahal_erase(fotahal_handle);
		fotahal_close(fotahal_handle);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	THINGS_LOG_D(THINGS_DEBUG, TAG, "[recv:BINARY] state : %d / recv_size : %u / total size : %u", download_state, recv_size, total_size, total_size);

	if (recv_size != total_size) {
		THINGS_LOG_ERROR(THINGS_ERROR, TAG, "[recv:BINARY] file size error");
		fotahal_erase(fotahal_handle);
		fotahal_close(fotahal_handle);
		things_free(json_str);
		download_state = FOTA_DOWNLOAD_STATE_NONE;
		return -1;
	}

	if (root != NULL) {
		cJSON_Delete(root);
	}
	things_free(json_str);

	download_state = FOTA_DOWNLOAD_STATE_DONE;

	fotahal_close(fotahal_handle);

	return 0;
}