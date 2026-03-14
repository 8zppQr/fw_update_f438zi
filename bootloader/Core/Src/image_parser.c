#include "image_parser.h"

#include <stdio.h>

static int image_is_range_valid_u32(uint32_t start, uint32_t size)
{
    if (size == 0U) {
        return 1;
    }

    if ((start + size) < start) {
        return 0; /* overflow */
    }

    return 1;
}

int image_get_layout(uint32_t slot_base, image_layout_t *out)
{
    const image_header_t *hdr;
    const image_tlv_info_t *info;
    uint32_t vector_addr;
    uint32_t tlv_info_addr;
    uint32_t tlv_end;

    if (out == NULL) {
        return 0;
    }

    hdr = (const image_header_t *)slot_base;

    if (hdr->ih_magic != IMAGE_MAGIC) {
        return 0;
    }

    vector_addr = slot_base + (uint32_t)hdr->ih_hdr_size;
    tlv_info_addr = vector_addr + (uint32_t)hdr->ih_img_size;

    if (!image_is_range_valid_u32(slot_base, (uint32_t)hdr->ih_hdr_size + (uint32_t)hdr->ih_img_size)) {
        return 0;
    }

    info = (const image_tlv_info_t *)tlv_info_addr;

    if (info->magic != TLV_INFO_MAGIC) {
        return 0;
    }

    tlv_end = tlv_info_addr + (uint32_t)info->tlv_tot;
    if (tlv_end < tlv_info_addr) {
        return 0;
    }

    out->slot_base = slot_base;
    out->hdr = hdr;

    out->vector_addr = vector_addr;
    out->img_start   = vector_addr;
    out->img_end     = vector_addr + (uint32_t)hdr->ih_img_size;

    out->tlv_info_addr = tlv_info_addr;
    out->tlv_info      = info;
    out->tlv_start     = tlv_info_addr + (uint32_t)sizeof(image_tlv_info_t);
    out->tlv_end       = tlv_end;

    return 1;
}

int image_find_tlv(const image_layout_t *img,
                   uint16_t type,
                   const uint8_t **value,
                   uint16_t *len)
{
    uint32_t p;

    if (img == NULL || value == NULL || len == NULL) {
        return 0;
    }

    p = img->tlv_start;

    while ((p + sizeof(image_tlv_t)) <= img->tlv_end) {
        const image_tlv_t *tlv = (const image_tlv_t *)p;
        uint32_t value_addr = p + (uint32_t)sizeof(image_tlv_t);
        uint32_t next = value_addr + (uint32_t)tlv->len;

        if (next < value_addr || next > img->tlv_end) {
            return 0;
        }

        if (tlv->type == type) {
            *value = (const uint8_t *)value_addr;
            *len   = tlv->len;
            return 1;
        }

        p = next;
    }

    return 0;
}

const char *image_tlv_type_to_str(uint16_t type)
{
    switch (type) {
    case TLV_TYPE_KEYHASH:      return "KEYHASH";
    case TLV_TYPE_SHA256:       return "SHA256";
    case TLV_TYPE_RSA2048_PSS:  return "RSA2048_PSS";
    case TLV_TYPE_ECDSA224:     return "ECDSA224";
    case TLV_TYPE_ECDSA256:     return "ECDSA256";
    case TLV_TYPE_RSA3072_PSS:  return "RSA3072_PSS";
    case TLV_TYPE_ED25519:      return "ED25519";
    default:                    return "UNKNOWN";
    }
}

uint32_t image_get_msp(const image_layout_t *img)
{
    if (img == NULL) {
        return 0U;
    }

    return *(const uint32_t *)(img->vector_addr + 0U);
}

uint32_t image_get_reset_handler(const image_layout_t *img)
{
    if (img == NULL) {
        return 0U;
    }

    return *(const uint32_t *)(img->vector_addr + 4U);
}

void image_dump_header(const image_layout_t *img)
{
    uint32_t msp;
    uint32_t rh;

    if (img == NULL || img->hdr == NULL) {
        printf("image_dump_header: invalid image\r\n");
        return;
    }

    msp = image_get_msp(img);
    rh  = image_get_reset_handler(img);

    printf("=== Image header @ 0x%08lX ===\r\n", (unsigned long)img->slot_base);
    printf("magic:              0x%08lX\r\n", (unsigned long)img->hdr->ih_magic);
    printf("load_addr:          0x%08lX\r\n", (unsigned long)img->hdr->ih_load_addr);
    printf("hdr_size:           0x%04X\r\n",  (unsigned)img->hdr->ih_hdr_size);
    printf("protected_tlv_size: 0x%04X\r\n",  (unsigned)img->hdr->ih_protect_tlv_size);
    printf("img_size:           0x%08lX\r\n", (unsigned long)img->hdr->ih_img_size);
    printf("flags:              0x%08lX\r\n", (unsigned long)img->hdr->ih_flags);
    printf("version:            %u.%u.%u+%lu\r\n",
           (unsigned)img->hdr->iv_major,
           (unsigned)img->hdr->iv_minor,
           (unsigned)img->hdr->iv_revision,
           (unsigned long)img->hdr->iv_build_num);
    printf("vector_addr:         0x%08lX\r\n", (unsigned long)img->vector_addr);
    printf("vector[0] MSP:       0x%08lX\r\n", (unsigned long)msp);
    printf("vector[1] ResetHdlr: 0x%08lX\r\n", (unsigned long)rh);
}

void image_dump_tlvs(const image_layout_t *img)
{
    uint32_t p;

    if (img == NULL || img->tlv_info == NULL) {
        printf("image_dump_tlvs: invalid image\r\n");
        return;
    }

    printf("=== Image TLV dump ===\r\n");
    printf("TLV start:            0x%08lX\r\n", (unsigned long)img->tlv_info_addr);
    printf("TLV magic:            0x%04X\r\n", (unsigned)img->tlv_info->magic);
    printf("TLV total size:       0x%04X\r\n", (unsigned)img->tlv_info->tlv_tot);

    p = img->tlv_start;

    while ((p + sizeof(image_tlv_t)) <= img->tlv_end) {
        const image_tlv_t *tlv = (const image_tlv_t *)p;
        uint32_t value_addr = p + (uint32_t)sizeof(image_tlv_t);
        uint32_t next = value_addr + (uint32_t)tlv->len;

        if (next < value_addr || next > img->tlv_end) {
            printf("TLV range error\r\n");
            return;
        }

        printf("TLV type:             0x%04X (%s)\r\n",
               (unsigned)tlv->type,
               image_tlv_type_to_str(tlv->type));
        printf("TLV len:              0x%04X\r\n", (unsigned)tlv->len);

        p = next;
    }
}
