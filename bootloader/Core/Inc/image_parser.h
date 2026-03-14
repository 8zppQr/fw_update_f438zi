#ifndef IMAGE_PARSER_H
#define IMAGE_PARSER_H


#include <stdint.h>
#include <stddef.h>

/* ===== MCUboot image constants ===== */
#define IMAGE_MAGIC         0x96F3B83DU
#define TLV_INFO_MAGIC      0x6907U

/* ===== Known TLV types ===== */
#define TLV_TYPE_KEYHASH       0x0001U
#define TLV_TYPE_SHA256        0x0010U
#define TLV_TYPE_RSA2048_PSS   0x0020U
#define TLV_TYPE_ECDSA224      0x0021U
#define TLV_TYPE_ECDSA256      0x0022U
#define TLV_TYPE_RSA3072_PSS   0x0023U
#define TLV_TYPE_ED25519       0x0024U

#pragma pack(push, 1)
typedef struct {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;
    uint16_t ih_protect_tlv_size;
    uint32_t ih_img_size;
    uint32_t ih_flags;
    uint8_t  iv_major;
    uint8_t  iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
    uint32_t _pad1;
} image_header_t;

typedef struct {
    uint16_t magic;
    uint16_t tlv_tot;
} image_tlv_info_t;

typedef struct {
    uint16_t type;
    uint16_t len;
} image_tlv_t;
#pragma pack(pop)

typedef struct {
    uint32_t slot_base;
    const image_header_t *hdr;

    uint32_t vector_addr;
    uint32_t img_start;
    uint32_t img_end;

    uint32_t tlv_info_addr;
    const image_tlv_info_t *tlv_info;
    uint32_t tlv_start;
    uint32_t tlv_end;
} image_layout_t;

/* Build image layout from slot base. Returns 1 on success, 0 on failure. */
int image_get_layout(uint32_t slot_base, image_layout_t *out);

/* Find a TLV by type. Returns 1 on success, 0 if not found / invalid. */
int image_find_tlv(const image_layout_t *img,
                   uint16_t type,
                   const uint8_t **value,
                   uint16_t *len);

/* Helpers */
const char *image_tlv_type_to_str(uint16_t type);

/* Optional dump helpers */
void image_dump_header(const image_layout_t *img);
void image_dump_tlvs(const image_layout_t *img);

/* Basic vector accessors */
uint32_t image_get_msp(const image_layout_t *img);
uint32_t image_get_reset_handler(const image_layout_t *img);

#endif /* IMAGE_PARSER_H */
