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
#include "utils/things_malloc.h"
#include "logging/things_logger.h"
#include "framework/things_data_manager.h"

#define TAG "[things_fota]"

#define FOTA_WEBCLIENT_BUF_SIZE     4600
#define FOTA_REC_JSON_SIZE		1000

#define  AUTH_MAX_LEN	30

#define KEY_URL "url"

const char c_ca_crt_rsa[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEIDCCAwigAwIBAgIQNE7VVyDV7exJ9C/ON9srbTANBgkqhkiG9w0BAQUFADCB\r\n"
"qTELMAkGA1UEBhMCVVMxFTATBgNVBAoTDHRoYXd0ZSwgSW5jLjEoMCYGA1UECxMf\r\n"
"Q2VydGlmaWNhdGlvbiBTZXJ2aWNlcyBEaXZpc2lvbjE4MDYGA1UECxMvKGMpIDIw\r\n"
"MDYgdGhhd3RlLCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxHzAdBgNV\r\n"
"BAMTFnRoYXd0ZSBQcmltYXJ5IFJvb3QgQ0EwHhcNMDYxMTE3MDAwMDAwWhcNMzYw\r\n"
"NzE2MjM1OTU5WjCBqTELMAkGA1UEBhMCVVMxFTATBgNVBAoTDHRoYXd0ZSwgSW5j\r\n"
"LjEoMCYGA1UECxMfQ2VydGlmaWNhdGlvbiBTZXJ2aWNlcyBEaXZpc2lvbjE4MDYG\r\n"
"A1UECxMvKGMpIDIwMDYgdGhhd3RlLCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNl\r\n"
"IG9ubHkxHzAdBgNVBAMTFnRoYXd0ZSBQcmltYXJ5IFJvb3QgQ0EwggEiMA0GCSqG\r\n"
"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQCsoPD7gFnUnMekz52hWXMJEEUMDSxuaPFs\r\n"
"W0hoSVk3/AszGcJ3f8wQLZU0HObrTQmnHNK4yZc2AreJ1CRfBsDMRJSUjQJib+ta\r\n"
"3RGNKJpchJAQeg29dGYvajig4tVUROsdB58Hum/u6f1OCyn1PoSgAfGcq/gcfomk\r\n"
"6KHYcWUNo1F77rzSImANuVud37r8UVsLr5iy6S7pBOhih94ryNdOwUxkHt3Ph1i6\r\n"
"Sk/KaAcdHJ1KxtUvkcx8cXIcxcBn6zL9yZJclNqFwJu/U30rCfSMnZEfl2pSy94J\r\n"
"NqR32HuHUETVPm4pafs5SSYeCaWAe0At6+gnhcn+Yf1+5nyXHdWdAgMBAAGjQjBA\r\n"
"MA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBR7W0XP\r\n"
"r87Lev0xkhpqtvNG61dIUDANBgkqhkiG9w0BAQUFAAOCAQEAeRHAS7ORtvzw6WfU\r\n"
"DW5FvlXok9LOAz/t2iWwHVfLHjp2oEzsUHboZHIMpKnxuIvW1oeEuzLlQRHAd9mz\r\n"
"YJ3rG9XRbkREqaYB7FViHXe4XI5ISXycO1cRrK1zN44veFyQaEfZYGDm/Ac9IiAX\r\n"
"xPcW6cTYcvnIc3zfFi8VqT79aie2oetaupgf1eNNZAqdE8hhuvU5HIe6uL17In/2\r\n"
"/qxAeeWsEG89jxt5dovEN7MhGITlNgDrYyCZuen+MwS7QcjBAvlEYyCegc5C09Y/\r\n"
"LHbTY5xZ3Y+m4Q6gLkH3LpVHz7z9M/P2C2F+fpErgUfCJzDupxBdN49cOSvkBPB7\r\n"
"jVaMaA==\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"
"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\n"
"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\n"
"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\n"
"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\n"
"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\n"
"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n"
"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\n"
"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\n"
"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIENjCCAx6gAwIBAgIBATANBgkqhkiG9w0BAQUFADBvMQswCQYDVQQGEwJTRTEU\r\n"
"MBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFkZFRydXN0IEV4dGVybmFs\r\n"
"IFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBFeHRlcm5hbCBDQSBSb290\r\n"
"MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFowbzELMAkGA1UEBhMCU0Ux\r\n"
"FDASBgNVBAoTC0FkZFRydXN0IEFCMSYwJAYDVQQLEx1BZGRUcnVzdCBFeHRlcm5h\r\n"
"bCBUVFAgTmV0d29yazEiMCAGA1UEAxMZQWRkVHJ1c3QgRXh0ZXJuYWwgQ0EgUm9v\r\n"
"dDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALf3GjPm8gAELTngTlvt\r\n"
"H7xsD821+iO2zt6bETOXpClMfZOfvUq8k+0DGuOPz+VtUFrWlymUWoCwSXrbLpX9\r\n"
"uMq/NzgtHj6RQa1wVsfwTz/oMp50ysiQVOnGXw94nZpAPA6sYapeFI+eh6FqUNzX\r\n"
"mk6vBbOmcZSccbNQYArHE504B4YCqOmoaSYYkKtMsE8jqzpPhNjfzp/haW+710LX\r\n"
"a0Tkx63ubUFfclpxCDezeWWkWaCUN/cALw3CknLa0Dhy2xSoRcRdKn23tNbE7qzN\r\n"
"E0S3ySvdQwAl+mG5aWpYIxG3pzOPVnVZ9c0p10a3CitlttNCbxWyuHv77+ldU9U0\r\n"
"WicCAwEAAaOB3DCB2TAdBgNVHQ4EFgQUrb2YejS0Jvf6xCZU7wO94CTLVBowCwYD\r\n"
"VR0PBAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wgZkGA1UdIwSBkTCBjoAUrb2YejS0\r\n"
"Jvf6xCZU7wO94CTLVBqhc6RxMG8xCzAJBgNVBAYTAlNFMRQwEgYDVQQKEwtBZGRU\r\n"
"cnVzdCBBQjEmMCQGA1UECxMdQWRkVHJ1c3QgRXh0ZXJuYWwgVFRQIE5ldHdvcmsx\r\n"
"IjAgBgNVBAMTGUFkZFRydXN0IEV4dGVybmFsIENBIFJvb3SCAQEwDQYJKoZIhvcN\r\n"
"AQEFBQADggEBALCb4IUlwtYj4g+WBpKdQZic2YR5gdkeWxQHIzZlj7DYd7usQWxH\r\n"
"YINRsPkyPef89iYTx4AWpb9a/IfPeHmJIZriTAcKhjW88t5RxNKWt9x+Tu5w/Rw5\r\n"
"6wwCURQtjr0W4MHfRnXnJK3s9EK0hZNwEGe6nQY1ShjTK3rMUUKhemPR5ruhxSvC\r\n"
"Nr4TDea9Y355e6cJDUCrat2PisP29owaQgVR1EX1n6diIWgVIEM8med8vSTYqZEX\r\n"
"c4g/VhsxOBi0cQ+azcgOno4uG+GMmIPLHzHxREzGBHNJdmAPx/i9F4BrLunMTA5a\r\n"
"mnkPIAou1Z5jJh5VkpTYghdae9C8x49OhgQ=\r\n"
"-----END CERTIFICATE-----\r\n";

#if 0
"-----BEGIN CERTIFICATE-----\r\n"
"MIICiDCCAg2gAwIBAgIQNfwmXNmET8k9Jj1Xm67XVjAKBggqhkjOPQQDAzCBhDEL\r\n"
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDHRoYXd0ZSwgSW5jLjE4MDYGA1UECxMvKGMp\r\n"
"IDIwMDcgdGhhd3RlLCBJbmMuIC0gRm9yIGF1dGhvcml6ZWQgdXNlIG9ubHkxJDAi\r\n"
"BgNVBAMTG3RoYXd0ZSBQcmltYXJ5IFJvb3QgQ0EgLSBHMjAeFw0wNzExMDUwMDAw\r\n"
"MDBaFw0zODAxMTgyMzU5NTlaMIGEMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMdGhh\r\n"
"d3RlLCBJbmMuMTgwNgYDVQQLEy8oYykgMjAwNyB0aGF3dGUsIEluYy4gLSBGb3Ig\r\n"
"YXV0aG9yaXplZCB1c2Ugb25seTEkMCIGA1UEAxMbdGhhd3RlIFByaW1hcnkgUm9v\r\n"
"dCBDQSAtIEcyMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEotWcgnuVnfFSeIf+iha/\r\n"
"BebfowJPDQfGAFG6DAJSLSKkQjnE/o/qycG+1E3/n3qe4rF8mq2nhglzh9HnmuN6\r\n"
"papu+7qzcMBniKI11KOasf2twu8x+qi58/sIxpHR+ymVo0IwQDAPBgNVHRMBAf8E\r\n"
"BTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUmtgAMADna3+FGO6Lts6K\r\n"
"DPgR4bswCgYIKoZIzj0EAwMDaQAwZgIxAN344FdHW6fmCsO99YCKlzUNG4k8VIZ3\r\n"
"KMqh9HneteY4sPBlcIx/AlTCv//YoT7ZzwIxAMSNlPzcU9LcnXgWHxUzI1NS41ox\r\n"
"XZ3Krr0TKUQNJ1uo52icEvdYPy5yAlejj6EULg==\r\n"
"-----END CERTIFICATE-----\r\n";
#endif

#if 0
"-----BEGIN CERTIFICATE-----\r\n"
"MIICXTCCAgGgAwIBAgIBATAMBggqhkjOPQQDAgUAMGsxKDAmBgNVBAMTH1NhbXN1\r\n"
"bmcgRWxlY3Ryb25pY3MgT0NGIFJvb3QgQ0ExFDASBgNVBAsTC09DRiBSb290IENB\r\n"
"MRwwGgYDVQQKExNTYW1zdW5nIEVsZWN0cm9uaWNzMQswCQYDVQQGEwJLUjAgFw0x\r\n"
"NjExMjQwMjU1MTFaGA8yMDY5MTIzMTE0NTk1OVowazEoMCYGA1UEAxMfU2Ftc3Vu\r\n"
"ZyBFbGVjdHJvbmljcyBPQ0YgUm9vdCBDQTEUMBIGA1UECxMLT0NGIFJvb3QgQ0Ex\r\n"
"HDAaBgNVBAoTE1NhbXN1bmcgRWxlY3Ryb25pY3MxCzAJBgNVBAYTAktSMFkwEwYH\r\n"
"KoZIzj0CAQYIKoZIzj0DAQcDQgAEYsf/Kx+sUFBQESbuytTDPwLPIe0X/8/B1L7b\r\n"
"3abxE9w0gQZAfI8WYUkKfNfP7HXh1M5SCnOkfwWraltGOKTeX6OBkTCBjjAOBgNV\r\n"
"HQ8BAf8EBAMCAcYwMgYDVR0fBCswKTAnoCWgI4YhaHR0cDovL3Byb2RjYS5zYW1z\r\n"
"dW5naW90cy5jb20vY3JsMA8GA1UdEwEB/wQFMAMBAf8wNwYIKwYBBQUHAQEEKzAp\r\n"
"MCcGCCsGAQUFBzABhhtodHRwOi8vb2NzcC5zYW1zdW5naW90cy5jb20wDAYIKoZI\r\n"
"zj0EAwIFAANIADBFAiARY9aSE30q31q5v8B4sJczBqOp7AsD9o8ZIuNmH7IwSwIh\r\n"
"ALfR56jcXoFis/nDx0tQ2xTI/f0b7F6tWPqj2vyKQeNR\r\n"
"-----END CERTIFICATE-----\r\n";
#endif
#if 0
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\r\n"
	"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\r\n"
	"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\r\n"
	"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\r\n"
	"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\r\n"
	"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\r\n"
	"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\r\n"
	"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\r\n"
	"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\r\n"
	"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\r\n"
	"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\r\n"
	"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\r\n"
	"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\r\n"
	"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\r\n"
	"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\r\n"
	"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\r\n"
	"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\r\n"
	"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\r\n"
	"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\r\n"
	"-----END CERTIFICATE-----\r\n"
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIENjCCAx6gAwIBAgIBATANBgkqhkiG9w0BAQUFADBvMQswCQYDVQQGEwJTRTEU\r\n"
	"MBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFkZFRydXN0IEV4dGVybmFs\r\n"
	"IFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBFeHRlcm5hbCBDQSBSb290\r\n"
	"MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFowbzELMAkGA1UEBhMCU0Ux\r\n"
	"FDASBgNVBAoTC0FkZFRydXN0IEFCMSYwJAYDVQQLEx1BZGRUcnVzdCBFeHRlcm5h\r\n"
	"bCBUVFAgTmV0d29yazEiMCAGA1UEAxMZQWRkVHJ1c3QgRXh0ZXJuYWwgQ0EgUm9v\r\n"
	"dDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALf3GjPm8gAELTngTlvt\r\n"
	"H7xsD821+iO2zt6bETOXpClMfZOfvUq8k+0DGuOPz+VtUFrWlymUWoCwSXrbLpX9\r\n"
	"uMq/NzgtHj6RQa1wVsfwTz/oMp50ysiQVOnGXw94nZpAPA6sYapeFI+eh6FqUNzX\r\n"
	"mk6vBbOmcZSccbNQYArHE504B4YCqOmoaSYYkKtMsE8jqzpPhNjfzp/haW+710LX\r\n"
	"a0Tkx63ubUFfclpxCDezeWWkWaCUN/cALw3CknLa0Dhy2xSoRcRdKn23tNbE7qzN\r\n"
	"E0S3ySvdQwAl+mG5aWpYIxG3pzOPVnVZ9c0p10a3CitlttNCbxWyuHv77+ldU9U0\r\n"
	"WicCAwEAAaOB3DCB2TAdBgNVHQ4EFgQUrb2YejS0Jvf6xCZU7wO94CTLVBowCwYD\r\n"
	"VR0PBAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wgZkGA1UdIwSBkTCBjoAUrb2YejS0\r\n"
	"Jvf6xCZU7wO94CTLVBqhc6RxMG8xCzAJBgNVBAYTAlNFMRQwEgYDVQQKEwtBZGRU\r\n"
	"cnVzdCBBQjEmMCQGA1UECxMdQWRkVHJ1c3QgRXh0ZXJuYWwgVFRQIE5ldHdvcmsx\r\n"
	"IjAgBgNVBAMTGUFkZFRydXN0IEV4dGVybmFsIENBIFJvb3SCAQEwDQYJKoZIhvcN\r\n"
	"AQEFBQADggEBALCb4IUlwtYj4g+WBpKdQZic2YR5gdkeWxQHIzZlj7DYd7usQWxH\r\n"
	"YINRsPkyPef89iYTx4AWpb9a/IfPeHmJIZriTAcKhjW88t5RxNKWt9x+Tu5w/Rw5\r\n"
	"6wwCURQtjr0W4MHfRnXnJK3s9EK0hZNwEGe6nQY1ShjTK3rMUUKhemPR5ruhxSvC\r\n"
	"Nr4TDea9Y355e6cJDUCrat2PisP29owaQgVR1EX1n6diIWgVIEM8med8vSTYqZEX\r\n"
	"c4g/VhsxOBi0cQ+azcgOno4uG+GMmIPLHzHxREzGBHNJdmAPx/i9F4BrLunMTA5a\r\n"
	"mnkPIAou1Z5jJh5VkpTYghdae9C8x49OhgQ=\r\n"
	"-----END CERTIFICATE-----\r\n";
#endif

struct http_client_ssl_config_t g_config = {
	(char *)c_ca_crt_rsa, NULL, NULL,
	sizeof(c_ca_crt_rsa), 0, 0, WEBCLIENT_SSL_VERIFY_REQUIRED
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
static const char headerfield_authorization[] = "Authorization";
static const char headerfield_x_iot_uid[] = "X-IOT-UID";

static const char str_wget[] = "WGET";
static const char str_get[] = "GET";

//#define FOTA_CHECK_URL "https://apisd.samsungiots.com/v1/stdm/fota/target?deviceType=artik&currentVersion=1.0"
#define FOTA_CHECK_URL "https://apis.samsungiotcloud.com/v1/stdm/fota/target?deviceType=NWSP-01&currentVersion=1.0"


char *json_str;
fotahal_handle_t fotahal_handle;
unsigned int recv_size = 0;
unsigned int total_size = 0;
static bool	is_link_fail;

static void callback(struct http_client_response_t *response)
{
	if (response->status != 200) {
		is_link_fail = true;
		THINGS_LOG_E(TAG, "recv callback status %d", response->status);
		return;
	}
	

	if (download_state == FOTA_DOWNLOAD_STATE_JSON) {
		strncpy(json_str + recv_size, response->entity, response->entity_len);
	} else if (download_state == FOTA_DOWNLOAD_STATE_BINARY) {
		fotahal_write(fotahal_handle, response->entity, response->entity_len);
	}
	recv_size += response->entity_len;
	total_size = response->total_len;
	THINGS_LOG_V(TAG, ". %d / %d", recv_size, total_size);
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

#define KEY_URL "url"
#define KEY_BIN_URL "binaryUrl"
#define KEY_BIN_SIZE "binarySize"
#define KEY_BIN_VERSION "version"
#define REQUEST_URL_LEN 256
#define TOKEN_URL_LEN 256
#define DOWNLOAD_URL_LEN 512
char binary_url[500] = {0,};

static int ischeck = 0;

int fmwup_http_check_firmware()
{
	if(ischeck == 0){
		THINGS_LOG_V(TAG, "check_firmware url : %s", FOTA_CHECK_URL);

		struct http_client_request_t request;
		struct http_keyvalue_list_t headers;
		struct http_client_response_t response;
		struct http_client_ssl_config_t *ssl_config = NULL;

		int ret = -1;



		if (webclient_init_request(FOTA_CHECK_URL, &request) != 0) {
			THINGS_LOG_E(TAG, "webclient_init_request error");
			return NULL;
		}

		ssl_config = g_https ? &g_config : NULL;

		char auth[AUTH_MAX_LEN] = "Bearer ";
		things_strcat(auth, AUTH_MAX_LEN - 1, dm_get_access_token());

		/* before sending request,
		* must initialize keyvalue list for request headers
		*/
		http_keyvalue_list_init(&headers);
		http_keyvalue_list_add(&headers, headerfield_connect, headerfield_close);
		http_keyvalue_list_add(&headers, headerfield_useragent, headerfield_tinyara);
		http_keyvalue_list_add(&headers, headerfield_authorization, auth);
		http_keyvalue_list_add(&headers, headerfield_x_iot_uid, dm_get_uid());
		request.headers = &headers;

		THINGS_LOG_V(TAG, "auth : %s, uid : %s", auth, dm_get_uid());

		if (http_client_response_init(&response) < 0) {
			printf("fail to response init\n");
			goto release_out;
		}

		

		/* before sending request by sync function,
		* must initialize response structure
		*/
		if (http_client_send_request(&request, ssl_config, &response)) {
			THINGS_LOG_E(TAG, "fail to send request");
			goto release_out;
		}

		
		printf("----------sync response----------\n");
		printf("status %d %s\n", response.status, response.phrase);
		printf("%s\n", response.entity);
		printf("---------------------------------\n");

		http_client_response_release(&response);


		char json_str1[FOTA_REC_JSON_SIZE];
		cJSON *root = NULL;

		unsigned int recv_size1 = 0;
		recv_size1 = 0;
		json_str1[recv_size1] = 0;
		strncpy(json_str1 + recv_size1, response.entity, response.entity_len);
		root = cJSON_Parse((const char *)json_str1);
		cJSON *url1 = cJSON_GetObjectItem(root, KEY_BIN_URL);
		cJSON *bin_size = cJSON_GetObjectItem(root, KEY_BIN_SIZE);
		if(url1 == NULL && bin_size == NULL){
			THINGS_LOG_D(TAG, "New version not available\n");
			ret = 2;
			goto release_out;
		}
		else
			THINGS_LOG_D(TAG, "New version available\n");
		total_size = bin_size ->valueint;

		strncpy(binary_url, url1->valuestring, strlen(url1->valuestring));

		THINGS_LOG_D(TAG, "binary_url : %s", binary_url);

		ret = 1;
		ischeck = 1;

	release_out:
		/* before finish of app,
		* must release keyvalue list for request headers
		*/
		http_keyvalue_list_release(&headers);
		THINGS_LOG_D(TAG, "end request");

		return ret;
	}else{
		fmwup_http_download_file(binary_url);
	}
}


int wget_from_url(char *download_url) 
{
	THINGS_LOG_V(TAG, "download url : %s\n", download_url);

	struct http_client_request_t request;
	struct http_keyvalue_list_t headers;
	struct http_client_response_t response;
	struct http_client_ssl_config_t *ssl_config = NULL;

	int ret;
	ret = -1;

	if (webclient_init_request(download_url, &request) != 0) {
		THINGS_LOG_E(TAG, "webclient_init_request error");
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
		THINGS_LOG_E(TAG, "fail to send request");
		goto release_out;
	}
	/* sleep for end request */
	while (request.async_flag > 0) {
		usleep(100000);
	}

	if (request.async_flag < 0) {
		THINGS_LOG_E(TAG, "fail to send request");
		goto release_out;
	}

	ret = 1;

release_out:
	/* before finish of app,
	 * must release keyvalue list for request headers
	 */
	http_keyvalue_list_release(&headers);
	THINGS_LOG_D(TAG, "end request");

	return ret;
}

int fmwup_http_download_file(const char *download_url)
{
	THINGS_LOG_D(TAG, THINGS_FUNC_ENTRY);

	download_state = FOTA_DOWNLOAD_STATE_JSON;
	json_str = (char *)things_malloc(FOTA_REC_JSON_SIZE); 
	recv_size = 0;
	
	is_link_fail = false;

	// parsing json
	if (wget_from_url(download_url) < 0) {
		THINGS_LOG_E(TAG, "wget_from_url error");
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

	THINGS_LOG_D(TAG, "[recv:JSON] state : %d / recv_size : %u / total size : %u / json = %s", download_state, recv_size, total_size, json_str);

	if (recv_size != total_size) {
		THINGS_LOG_E(TAG, "[recv:JSON] file size error");
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
		THINGS_LOG_E(TAG, "wget_from_url error");

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

	THINGS_LOG_D(TAG, "[recv:BINARY] state : %d / recv_size : %u / total size : %u", download_state, recv_size, total_size, total_size);

	if (recv_size != total_size) {
		THINGS_LOG_E(TAG, "[recv:BINARY] file size error");
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
