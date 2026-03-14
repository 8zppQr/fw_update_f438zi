#include "image_verify.h"

#include <stdio.h>
#include <string.h>

#include <tinycrypt/sha256.h>
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dsa.h>

#ifndef IMAGE_HASH_SIZE
#define IMAGE_HASH_SIZE 32U
#endif

static void verify_print_hex(const uint8_t *buf, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\r\n");
}

static int der_ecdsa_sig_to_rs64(const uint8_t *der, uint32_t der_len, uint8_t out_rs[64])
{
    uint32_t p = 0;
    uint32_t seq_len;
    uint32_t r_len;
    uint32_t s_len;
    const uint8_t *r_ptr;
    const uint8_t *s_ptr;

    memset(out_rs, 0, 64);

    if (der == NULL || out_rs == NULL || der_len < 8U) {
        return 0;
    }

    if (der[p++] != 0x30U) {
        printf("DER: not SEQUENCE\r\n");
        return 0;
    }

    seq_len = der[p++];
    if ((seq_len + 2U) != der_len) {
        printf("DER: bad sequence length\r\n");
        return 0;
    }

    if (p >= der_len || der[p++] != 0x02U) {
        printf("DER: r not INTEGER\r\n");
        return 0;
    }

    if (p >= der_len) {
        return 0;
    }

    r_len = der[p++];
    if (r_len == 0U || (p + r_len) > der_len) {
        printf("DER: bad r length\r\n");
        return 0;
    }

    r_ptr = &der[p];
    p += r_len;

    if (p >= der_len || der[p++] != 0x02U) {
        printf("DER: s not INTEGER\r\n");
        return 0;
    }

    if (p >= der_len) {
        return 0;
    }

    s_len = der[p++];
    if (s_len == 0U || (p + s_len) > der_len) {
        printf("DER: bad s length\r\n");
        return 0;
    }

    s_ptr = &der[p];
    p += s_len;

    if (p != der_len) {
        printf("DER: trailing bytes exist\r\n");
        return 0;
    }

    while ((r_len > 0U) && (*r_ptr == 0x00U)) {
        r_ptr++;
        r_len--;
    }

    while ((s_len > 0U) && (*s_ptr == 0x00U)) {
        s_ptr++;
        s_len--;
    }

    if (r_len > 32U || s_len > 32U) {
        printf("DER: r/s too large\r\n");
        return 0;
    }

    memcpy(&out_rs[32U - r_len], r_ptr, r_len);
    memcpy(&out_rs[64U - s_len], s_ptr, s_len);

    return 1;
}

static int der_pubkey_find_point65(const uint8_t *pubkey_der,
                                   uint32_t pubkey_der_len,
                                   const uint8_t **point65)
{
    uint32_t i;

    if (pubkey_der == NULL || point65 == NULL || pubkey_der_len < 65U) {
        return 0;
    }

    for (i = 0; i + 65U <= pubkey_der_len; i++) {
        if (pubkey_der[i] == 0x04U) {
            *point65 = &pubkey_der[i];
            return 1;
        }
    }

    printf("Public key point not found in DER\r\n");
    return 0;
}

int image_calc_hash(const image_layout_t *img, uint8_t out_hash[32])
{
    struct tc_sha256_state_struct s;
    uint32_t hash_len;

    if (img == NULL || out_hash == NULL || img->hdr == NULL) {
        return 0;
    }

    hash_len = (uint32_t)img->hdr->ih_hdr_size + (uint32_t)img->hdr->ih_img_size;

    if (!tc_sha256_init(&s)) {
        printf("tc_sha256_init failed\r\n");
        return 0;
    }

    if (!tc_sha256_update(&s, (const uint8_t *)img->slot_base, hash_len)) {
        printf("tc_sha256_update failed\r\n");
        return 0;
    }

    if (!tc_sha256_final(out_hash, &s)) {
        printf("tc_sha256_final failed\r\n");
        return 0;
    }

    return 1;
}

int image_verify_hash(const image_layout_t *img)
{
    const uint8_t *expected;
    uint16_t len;
    uint8_t calculated[IMAGE_HASH_SIZE];

    if (img == NULL) {
        return 0;
    }

    printf("=== Image hash verify ===\r\n");

    if (!image_find_tlv(img, TLV_TYPE_SHA256, &expected, &len)) {
        printf("SHA256 TLV not found\r\n");
        return 0;
    }

    if (len != IMAGE_HASH_SIZE) {
        printf("SHA256 TLV length invalid: %u\r\n", (unsigned)len);
        return 0;
    }

    if (!image_calc_hash(img, calculated)) {
        return 0;
    }

    printf("Expected SHA256:\r\n");
    verify_print_hex(expected, len);

    printf("Calculated SHA256:\r\n");
    verify_print_hex(calculated, sizeof(calculated));

    if (memcmp(expected, calculated, sizeof(calculated)) == 0) {
        printf("HASH MATCH\r\n");
        return 1;
    }

    printf("HASH MISMATCH\r\n");
    return 0;
}

int image_verify_keyhash(const image_layout_t *img,
                         const uint8_t *pubkey_der,
                         uint32_t pubkey_der_len)
{
    const uint8_t *expected;
    uint16_t len;
    uint8_t calculated[IMAGE_HASH_SIZE];
    struct tc_sha256_state_struct s;

    if (img == NULL || pubkey_der == NULL || pubkey_der_len == 0U) {
        return 0;
    }

    printf("=== Public key hash verify ===\r\n");

    if (!image_find_tlv(img, TLV_TYPE_KEYHASH, &expected, &len)) {
        printf("KEYHASH TLV not found\r\n");
        return 0;
    }

    if (len != IMAGE_HASH_SIZE) {
        printf("KEYHASH TLV length invalid: %u\r\n", (unsigned)len);
        return 0;
    }

    if (!tc_sha256_init(&s)) {
        printf("tc_sha256_init failed\r\n");
        return 0;
    }

    if (!tc_sha256_update(&s, pubkey_der, pubkey_der_len)) {
        printf("tc_sha256_update failed\r\n");
        return 0;
    }

    if (!tc_sha256_final(calculated, &s)) {
        printf("tc_sha256_final failed\r\n");
        return 0;
    }

    printf("Expected KEYHASH:\r\n");
    verify_print_hex(expected, len);

    printf("Calculated KEYHASH:\r\n");
    verify_print_hex(calculated, sizeof(calculated));

    if (memcmp(expected, calculated, sizeof(calculated)) == 0) {
        printf("KEYHASH MATCH\r\n");
        return 1;
    }

    printf("KEYHASH MISMATCH\r\n");
    return 0;
}

int image_verify_signature(const image_layout_t *img,
                           const uint8_t *pubkey_der,
                           uint32_t pubkey_der_len)
{
    const uint8_t *sig_der;
    const uint8_t *pub_point65;
    uint16_t sig_der_len;
    uint8_t sig_rs[64];
    uint8_t hash[IMAGE_HASH_SIZE];
    int ok;

    if (img == NULL || pubkey_der == NULL || pubkey_der_len == 0U) {
        return 0;
    }

    printf("=== Image signature verify ===\r\n");

    if (!image_find_tlv(img, TLV_TYPE_ECDSA256, &sig_der, &sig_der_len)) {
        printf("ECDSA256 TLV not found\r\n");
        return 0;
    }

    printf("ECDSA DER signature:\r\n");
    verify_print_hex(sig_der, sig_der_len);

    if (!der_ecdsa_sig_to_rs64(sig_der, sig_der_len, sig_rs)) {
        printf("DER to raw(rs) failed\r\n");
        return 0;
    }

    printf("ECDSA raw r||s:\r\n");
    verify_print_hex(sig_rs, sizeof(sig_rs));

    if (!image_calc_hash(img, hash)) {
        return 0;
    }

    printf("Image hash:\r\n");
    verify_print_hex(hash, sizeof(hash));

    if (!der_pubkey_find_point65(pubkey_der, pubkey_der_len, &pub_point65)) {
        return 0;
    }

    ok = uECC_verify(&pub_point65[1],
                     hash,
                     sizeof(hash),
                     sig_rs,
                     uECC_secp256r1());

    if (ok) {
        printf("SIGNATURE VERIFY OK\r\n");
        return 1;
    }

    printf("SIGNATURE VERIFY NG\r\n");
    return 0;
}
