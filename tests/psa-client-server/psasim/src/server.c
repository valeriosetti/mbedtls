/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#include <unistd.h>
#include <stdio.h>

/* Includes from mbedtls */
#include "mbedtls/version.h"
#include "psa/crypto.h"

/* Includes from psasim */
#include "server.h"
#include "error_ext.h"
#include "util.h"
#include "psa_functions_codes.h"

static int kill_on_disconnect = 0; /* Kill the server on client disconnection. */

extern psa_status_t psa_crypto_call(void);
extern void psa_crypto_close(void);

void parse_input_args(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "k")) != -1) {
        switch (opt) {
            case 'k':
                kill_on_disconnect = 1;
                break;
            default:
                PRINT("Usage: %s [-k]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

static psa_status_t manage_connection(void)
{
    psa_status_t status;

    do {
        INFO("Wait for command");
        status = psa_wait_for_command();
        if (status != PSA_SUCCESS) {
            INFO("Connection dropped");
            return PSA_ERROR_COMMUNICATION_FAILURE;
        }

        INFO("Processing command");
        status = psa_crypto_call();
        if (status != PSA_SUCCESS) {
            INFO("PSA command execution failed (%d)", status);
            /* That's not a real issue, the command failed, but the connection
             * didn't drop. Just wait for the next command. */
        }

        INFO("Sending reply");
        status = psa_send_reply();
        if (status != PSA_SUCCESS) {
            ERROR("Unable to send the reply to the client.");
            return PSA_ERROR_COMMUNICATION_FAILURE;
        }
    } while (1);
}

int main(int argc, char *argv[])
{
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;
    char mbedtls_version[18];
    int ret = EXIT_SUCCESS;

    mbedtls_version_get_string_full(mbedtls_version);
    INFO("%s", mbedtls_version);

    parse_input_args(argc, argv);

    INFO("Creating socket");
    status = psa_setup_socket();
    if (status != PSA_SUCCESS) {
        ret = EXIT_FAILURE;
        goto exit;
    }

    do {
        INFO("Wait for connection");
        status = psa_wait_for_connection();
        if (status != PSA_SUCCESS) {
            ret = EXIT_FAILURE;
            goto exit;
        }
        INFO("Client connected");

        /* Intentionally ignoring the return value here because the only way
         * to return from this call is when the client disconnects or there is
         * an issue with the communication. In both cases we'll just want
         * to restart a new connection (if kill_on_disconnect is not set,
         * of course). */
        manage_connection();

        INFO("Connection closed");
        psa_close_connection();

        if (kill_on_disconnect) {
            break;
        }
    } while (1);

exit:
    psa_close_connection();
    psa_close_socket();
    return EXIT_SUCCESS;
}
