/*
 *  Classic "Hello, world" demonstration program
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "mbedtls/build_info.h"
#include "mbedtls/platform.h"
#include "mbedtls/md.h"
#include "psa/crypto.h"

#if !(defined(MBEDTLS_MD_SOME_PSA) || defined(MBEDTLS_MD_SOME_LEGACY))
int main(void)
{
    mbedtls_printf("MD not supported.\n");
    mbedtls_exit(MBEDTLS_EXIT_FAILURE);
}

#else

int main(void)
{
    int i;
    unsigned char digest[MBEDTLS_MD_MAX_SIZE];
    char str[] = "Hello, world!";
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

    psa_crypto_init();

    mbedtls_printf("\n  MD('%s') = ", str);

    if (mbedtls_md(mbedtls_md_info_from_type(md_type),
                   (unsigned char *) str, sizeof(str), digest) != 0) {
        mbedtls_exit(MBEDTLS_EXIT_FAILURE);
    }

    for (i = 0; i < mbedtls_md_get_size_from_type(md_type); i++) {
        mbedtls_printf("%02x", digest[i]);
    }

    mbedtls_printf("\n\n");

    mbedtls_exit(MBEDTLS_EXIT_SUCCESS);
}
#endif /* MBEDTLS_MD5_C */
