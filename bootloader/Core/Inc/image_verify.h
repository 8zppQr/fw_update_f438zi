#ifndef IMAGE_VERIFY_H
#define IMAGE_VERIFY_H

#include <stdint.h>
#include "image_parser.h"

/* Verify image hash against SHA256 TLV */
int image_verify_hash(const image_layout_t *img);

/* Verify public key hash against KEYHASH TLV */
int image_verify_keyhash(const image_layout_t *img,
                         const uint8_t *pubkey_der,
                         uint32_t pubkey_der_len);

/* Verify ECDSA-P256 signature TLV using DER public key */
int image_verify_signature(const image_layout_t *img,
                           const uint8_t *pubkey_der,
                           uint32_t pubkey_der_len);

/* Utility: calculate SHA256(header + image) */
int image_calc_hash(const image_layout_t *img, uint8_t out_hash[32]);

#endif /* IMAGE_VERIFY_H */
