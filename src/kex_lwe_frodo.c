/********************************************************************************************
* Frodo: a post-quantum key exchange based on the Learning with Errors (LWE) problem.
*
* Abstract: key exchange
*           Instantiates "kex_lwe_frodo_macrify.c" with the necessary kex functions
*********************************************************************************************/

#if defined(WINDOWS)
#define UNUSED
#else
#define UNUSED __attribute__ ((unused))
#endif

#include <stdlib.h>
#include <string.h>
#if !defined(WINDOWS)
#include <strings.h>
#include <unistd.h>
#endif

#include "kex.h"
#include "random/rand.h"
#include "kex_lwe_frodo.h"
#include "frodo_macrify.h"

#if defined(USE_AVX2)
#if defined(WINDOWS)
#include <malloc.h>
#else
#include <mm_malloc.h>
#endif
#endif

#define LWE_DIV_ROUNDUP(x, y) (((x) + (y)-1) / y)


OQS_KEX *OQS_KEX_lwe_frodo_new(OQS_RAND *rand, const uint8_t *seed, const size_t seed_len, const char *named_parameters) 
{ // Create a new Frodo structure
    OQS_KEX *k;
    struct oqs_kex_lwe_frodo_params *params;

    if ((seed_len == 0) || (seed == NULL)) {
        return NULL;
    }

    k = malloc(sizeof(OQS_KEX));
    if (k == NULL) {
        goto err;
    }
    k->named_parameters = NULL;
    k->method_name = NULL;

    k->params = malloc(sizeof(struct oqs_kex_lwe_frodo_params));
    if (NULL == k->params) {
        goto err;
    }
    params = (struct oqs_kex_lwe_frodo_params*)k->params;
    params->cdf_table = NULL;
    params->seed = NULL;
    params->param_name = NULL;

    k->rand = rand;
    k->ctx = NULL;
    k->alice_priv_free = &OQS_KEX_lwe_frodo_alice_priv_free;
    k->free = &OQS_KEX_lwe_frodo_free;

    if (strcmp(named_parameters, "recommended") == 0) 
    {
        k->alice_0 = &OQS_KEX_lwe_frodo_alice_0_recommended;
        k->bob = &OQS_KEX_lwe_frodo_bob_recommended;
        k->alice_1 = &OQS_KEX_lwe_frodo_alice_1_recommended;

        k->method_name = strdup("LWE Frodo recommended");
        if (NULL == k->method_name) {
            goto err;
        }
        k->estimated_classical_security = 144;
        k->estimated_quantum_security = 130;
        k->named_parameters = strdup(named_parameters);
        if (k->named_parameters == NULL) {
            goto err;
        }

        params->seed = malloc(seed_len);
        if (NULL == params->seed) {
            goto err;
        }
        memcpy(params->seed, seed, seed_len);
        params->seed_len = seed_len;
        params->param_name = strdup("recommended");
        if (NULL == params->param_name) {
            goto err;
        }
        params->log2_q = 15;
        params->q = 1 << params->log2_q;
        params->n = 752;
        params->extracted_bits = 4;
        params->nbar = 8;
        params->key_bits = 256;
        params->rec_hint_len = LWE_DIV_ROUNDUP(params->nbar * params->nbar, 8);                       
        params->pub_len = LWE_DIV_ROUNDUP(params->n * params->nbar * params->log2_q, 8);
        params->stripe_step = 8;
        params->sampler_num = 12;
        params->cdf_table_len = 6;
        params->cdf_table = malloc(params->cdf_table_len * sizeof(uint16_t));
        if (NULL == params->cdf_table) {
            goto err;
        }
        uint16_t cdf_table_tmp[6] = {602, 1521, 1927, 2031, 2046, 2047};
        memcpy(params->cdf_table, cdf_table_tmp, sizeof(cdf_table_tmp));
    } else {
        goto err;
    }

    return k;

err:
    if (k) {
        if (k->params) {
            free(params->cdf_table);
            free(params->seed);
            free(params->param_name);
            free(k->params);
        }
        free(k->named_parameters);
        free(k->method_name);
        free(k);
    }
    return NULL;
}


// Pre-process code to obtain "recommended" functions
#define MACRIFY(NAME) NAME ## _recommended
#define PARAMS_N 752
#define PARAMS_NBAR 8
#define PARAMS_EXTRACTED_BITS 4
#include "kex_lwe_frodo_macrify.c"
// Undefine macros to avoid any confusion later
#undef PARAMS_N
#undef PARAMS_NBAR
#undef MACRIFY


void OQS_KEX_lwe_frodo_alice_priv_free(UNUSED OQS_KEX *k, void *alice_priv)
{
    if (alice_priv) {
#if !defined(USE_AVX2)
        free(alice_priv);
#else
        _mm_free(alice_priv);
#endif
    }
}


void OQS_KEX_lwe_frodo_free(OQS_KEX *k) 
{ // Free Frodo structure
    if (!k) {
        return;
    }
    if (k->params) {
        struct oqs_kex_lwe_frodo_params *params = (struct oqs_kex_lwe_frodo_params*)k->params;
        free(params->cdf_table);
        params->cdf_table = NULL;
        free(params->seed);
        params->seed = NULL;
        free(params->param_name);
        params->param_name = NULL;
        free(k->params);
        k->params = NULL;
    }
    free(k->named_parameters);
    k->named_parameters = NULL;
    free(k->method_name);
    k->method_name = NULL;
    free(k);
}
