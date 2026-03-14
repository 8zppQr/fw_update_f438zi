//#include "main.h"
//#include "http_client.h"
//
//#include <stdio.h>
//#include <string.h>
//
//#include "lwip/tcp.h"
//#include "lwip/ip_addr.h"
//
//extern UART_HandleTypeDef huart3;
//
//#define HTTP_SERVER_IP0   10
//#define HTTP_SERVER_IP1   14
//#define HTTP_SERVER_IP2   1
//#define HTTP_SERVER_IP3   111
//#define HTTP_SERVER_PORT  8000
//
//static struct tcp_pcb *http_pcb = NULL;
//
//static err_t http_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
//static err_t http_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
//static void http_client_err(void *arg, err_t err);
//
//typedef enum {
//    HTTP_IDLE = 0,
//    HTTP_CONNECTING,
//    HTTP_RECEIVING
//} http_client_state_t;
//
//static http_client_state_t http_state = HTTP_IDLE;
//
//static int header_done = 0;
//
//static uint8_t fw_header[8];
//static int fw_header_bytes = 0;
//
//void http_client_start(void)
//{
//    ip_addr_t server_ip;
//    err_t ret;
//    uint32_t seed = DWT->CYCCNT;
//    u16_t local_port;
//    local_port = 40000 + (seed % 20000);
//
//    printf("http_client_start() called, state=%d\r\n", http_state);
//
//    if (http_state != HTTP_IDLE || http_pcb != NULL) {
//        printf("HTTP client busy\r\n");
//        return;
//    }
//
//    IP4_ADDR(&server_ip, HTTP_SERVER_IP0, HTTP_SERVER_IP1, HTTP_SERVER_IP2, HTTP_SERVER_IP3);
//
//    http_pcb = tcp_new();
//    if (http_pcb == NULL) {
//        printf("tcp_new failed\r\n");
//        return;
//    }
//
////    ret = tcp_bind(http_pcb, IP_ADDR_ANY, local_port);
////    printf("tcp_bind local port = %u, ret = %d\r\n", local_port, ret);
////    if (ret != ERR_OK) {
////        tcp_abort(http_pcb);
////        http_pcb = NULL;
////        http_state = HTTP_IDLE;
////        return;
////    }
//
//    http_state = HTTP_CONNECTING;
//
//    tcp_arg(http_pcb, NULL);
//    tcp_recv(http_pcb, http_client_recv);
//    tcp_err(http_pcb, http_client_err);
//
//    printf("Connecting to %d.%d.%d.%d:%d ...\r\n",
//           HTTP_SERVER_IP0, HTTP_SERVER_IP1, HTTP_SERVER_IP2, HTTP_SERVER_IP3, HTTP_SERVER_PORT);
//
//    ret = tcp_connect(http_pcb, &server_ip, HTTP_SERVER_PORT, http_client_connected);
//    printf("tcp_connect ret = %d\r\n", ret);
//    if (ret == ERR_OK) {
//        printf("assigned local port = %u\r\n", http_pcb->local_port);
//    }
//
//    if (ret != ERR_OK) {
//        tcp_recv(http_pcb, NULL);
//        tcp_err(http_pcb, NULL);
//        tcp_arg(http_pcb, NULL);
//        tcp_abort(http_pcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//    }
//}
//
//static err_t http_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
//{
//    const char *request =
//        "GET /slot_b.bin HTTP/1.1\r\n"
//        "Host: 10.14.1.111:8000\r\n"
//        "Connection: close\r\n"
//        "\r\n";
//
//    err_t ret;
//    LWIP_UNUSED_ARG(arg);
//
//    printf("connected cb err=%d\r\n", err);
//
//    if (err != ERR_OK) {
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return err;
//    }
//
//    http_state = HTTP_RECEIVING;
//
//    printf("Connected. Sending HTTP GET...\r\n");
//
//    ret = tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
//    if (ret != ERR_OK) {
//        printf("tcp_write failed: %d\r\n", ret);
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//        tcp_abort(tpcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return ret;
//    }
//
//    ret = tcp_output(tpcb);
//    if (ret != ERR_OK) {
//        printf("tcp_output failed: %d\r\n", ret);
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//        tcp_abort(tpcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return ret;
//    }
//
//    return ERR_OK;
//}
//
//static err_t http_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
//{
//    struct pbuf *q;
//    LWIP_UNUSED_ARG(arg);
//
//    if (err != ERR_OK) {
//        printf("recv error: %d\r\n", err);
//        if (p != NULL) {
//            pbuf_free(p);
//        }
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//        tcp_abort(tpcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return err;
//    }
//
//    if (p == NULL) {
//        err_t close_ret;
//
//        printf("\r\n--- server closed connection ---\r\n");
//
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//
//        close_ret = tcp_close(tpcb);
//        printf("tcp_close ret = %d\r\n", close_ret);
//        if (close_ret != ERR_OK) {
//            tcp_abort(tpcb);
//        }
//
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return ERR_OK;
//    }
//
//    tcp_recved(tpcb, p->tot_len);
//
////    for (q = p; q != NULL; q = q->next) {
////        HAL_UART_Transmit(&huart3, (uint8_t *)q->payload, q->len, 0xFFFF);
////    }
//
//    for (q = p; q != NULL; q = q->next)
//    {
//        uint8_t *data = (uint8_t *)q->payload;
//        int len = q->len;
//
//        if (!header_done)
//        {
//            char *body = strstr((char*)data, "\r\n\r\n");
//            if (body)
//            {
//                header_done = 1;
//                body += 4;
//
//                data = (uint8_t*)body;
//                len = q->len - (body - (char*)q->payload);
//
//                printf("\nHTTP header parsed\r\n");
//            }
//            else
//            {
//                continue;
//            }
//        }
//
//        for (int i = 0; i < len; i++)
//        {
//            if (fw_header_bytes < 8)
//            {
//                fw_header[fw_header_bytes++] = data[i];
//
//                if (fw_header_bytes == 8)
//                {
//                    uint32_t msp =
//                        fw_header[0] |
//                        (fw_header[1] << 8) |
//                        (fw_header[2] << 16) |
//                        (fw_header[3] << 24);
//
//                    uint32_t reset =
//                        fw_header[4] |
//                        (fw_header[5] << 8) |
//                        (fw_header[6] << 16) |
//                        (fw_header[7] << 24);
//
//                    printf("MSP   = 0x%08lX\r\n", msp);
//                    printf("Reset = 0x%08lX\r\n", reset);
//                }
//            }
//        }
//    }
//
//    pbuf_free(p);
//    return ERR_OK;
//}
//
//static void http_client_err(void *arg, err_t err)
//{
//    LWIP_UNUSED_ARG(arg);
//    printf("tcp error callback: %d\r\n", err);
//    http_pcb = NULL;
//    http_state = HTTP_IDLE;
//}


#include "main.h"
#include "http_client.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

extern UART_HandleTypeDef huart3;

#define HTTP_SERVER_IP0   10
#define HTTP_SERVER_IP1   14
#define HTTP_SERVER_IP2   1
#define HTTP_SERVER_IP3   111
#define HTTP_SERVER_PORT  8000

/* ===== OTA target ===== */
#define SLOT_B_ADDR       0x080C0000UL
#define SLOT_B_MAX_SIZE   (512UL * 1024UL)   /* 0x080C0000 - 0x080FFFFF */

/* ===== HTTP header buffer ===== */
#define HTTP_HEADER_BUF_SIZE 512

static struct tcp_pcb *http_pcb = NULL;

static err_t http_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t http_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void http_client_err(void *arg, err_t err);

typedef enum {
    HTTP_IDLE = 0,
    HTTP_CONNECTING,
    HTTP_RECEIVING
} http_client_state_t;

static http_client_state_t http_state = HTTP_IDLE;

/* ===== OTA receive state ===== */
static uint8_t  header_done = 0;
static char     header_buf[HTTP_HEADER_BUF_SIZE];
static uint32_t header_len = 0;

static uint32_t content_length = 0;
static uint32_t body_received  = 0;
static uint32_t flash_write_addr = SLOT_B_ADDR;
static uint8_t  flash_open = 0;

static uint8_t  fw_header[8];
static uint32_t fw_header_bytes = 0;

/* ---------- utility ---------- */

static void ota_reset_state(void)
{
    header_done = 0;
    header_len = 0;
    memset(header_buf, 0, sizeof(header_buf));

    content_length = 0;
    body_received = 0;
    flash_write_addr = SLOT_B_ADDR;

    flash_open = 0;

    fw_header_bytes = 0;
    memset(fw_header, 0, sizeof(fw_header));
}

static uint32_t get_sector(uint32_t address)
{
    if (address < 0x08004000) return FLASH_SECTOR_0;
    if (address < 0x08008000) return FLASH_SECTOR_1;
    if (address < 0x0800C000) return FLASH_SECTOR_2;
    if (address < 0x08010000) return FLASH_SECTOR_3;
    if (address < 0x08020000) return FLASH_SECTOR_4;
    if (address < 0x08040000) return FLASH_SECTOR_5;
    if (address < 0x08060000) return FLASH_SECTOR_6;
    if (address < 0x08080000) return FLASH_SECTOR_7;
    if (address < 0x080A0000) return FLASH_SECTOR_8;
    if (address < 0x080C0000) return FLASH_SECTOR_9;
    if (address < 0x080E0000) return FLASH_SECTOR_10;
    return FLASH_SECTOR_11;
}

static HAL_StatusTypeDef ota_erase_slot_b(uint32_t image_size)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sector_error = 0;
    uint32_t start_sector;
    uint32_t end_sector;
    uint32_t end_addr;

    if (image_size == 0 || image_size > SLOT_B_MAX_SIZE) {
        printf("Invalid image size: %lu\r\n", image_size);
        return HAL_ERROR;
    }

    end_addr = SLOT_B_ADDR + image_size - 1U;
    start_sector = get_sector(SLOT_B_ADDR);
    end_sector   = get_sector(end_addr);

    HAL_FLASH_Unlock();

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector       = start_sector;
    erase.NbSectors    = (end_sector - start_sector) + 1U;

    printf("Erasing flash: sector %lu -> %lu\r\n", start_sector, end_sector);

    if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK) {
        printf("Flash erase failed, sector_error=%lu\r\n", sector_error);
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    flash_open = 1;
    return HAL_OK;
}

static HAL_StatusTypeDef ota_write_byte(uint32_t addr, uint8_t b)
{
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, b);
}

static void ota_print_vectors_if_ready(void)
{
    if (fw_header_bytes == 8U) {
        uint32_t msp =
            ((uint32_t)fw_header[0])       |
            ((uint32_t)fw_header[1] << 8)  |
            ((uint32_t)fw_header[2] << 16) |
            ((uint32_t)fw_header[3] << 24);

        uint32_t reset =
            ((uint32_t)fw_header[4])       |
            ((uint32_t)fw_header[5] << 8)  |
            ((uint32_t)fw_header[6] << 16) |
            ((uint32_t)fw_header[7] << 24);

        printf("FW vectors:\r\n");
        printf("  MSP   = 0x%08lX\r\n", msp);
        printf("  Reset = 0x%08lX\r\n", reset);

        if ((msp & 0x2FFE0000U) == 0x20000000U) {
            printf("  MSP looks valid\r\n");
        } else {
            printf("  MSP looks INVALID\r\n");
        }
    }
}

static int ota_parse_content_length(void)
{
    char *p;

    p = strstr(header_buf, "Content-Length:");
    if (p == NULL) {
        printf("Content-Length not found\r\n");
        return -1;
    }

    p += strlen("Content-Length:");
    while (*p == ' ') {
        p++;
    }

    content_length = (uint32_t)strtoul(p, NULL, 10);
    printf("Content-Length = %lu\r\n", content_length);

    if (content_length == 0 || content_length > SLOT_B_MAX_SIZE) {
        printf("Invalid Content-Length\r\n");
        return -1;
    }

    return 0;
}

static int ota_process_body_bytes(const uint8_t *data, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++) {
        uint8_t b = data[i];

        if (fw_header_bytes < 8U) {
            fw_header[fw_header_bytes++] = b;
            if (fw_header_bytes == 8U) {
                ota_print_vectors_if_ready();
            }
        }

        if (body_received >= content_length) {
            printf("Received more than Content-Length\r\n");
            return -1;
        }

        if (ota_write_byte(flash_write_addr, b) != HAL_OK) {
            printf("Flash write failed at 0x%08lX\r\n", flash_write_addr);
            return -1;
        }

        flash_write_addr++;
        body_received++;
    }

    return 0;
}

static void ota_finish(int ok)
{
    if (flash_open) {
        HAL_FLASH_Lock();
        flash_open = 0;
    }

    if (ok) {
        printf("Download complete: %lu / %lu bytes\r\n", body_received, content_length);
        printf("Image written to 0x%08lX\r\n", SLOT_B_ADDR);

        printf("Rebooting...\r\n");
        HAL_Delay(100);
        NVIC_SystemReset();
    } else {
        printf("Download failed: %lu / %lu bytes\r\n", body_received, content_length);
    }
}

/* ---------- HTTP client ---------- */

void http_client_start(void)
{
    ip_addr_t server_ip;
    err_t ret;

    printf("http_client_start() called, state=%d\r\n", http_state);

    if (http_state != HTTP_IDLE || http_pcb != NULL) {
        printf("HTTP client busy\r\n");
        return;
    }

    ota_reset_state();

    IP4_ADDR(&server_ip, HTTP_SERVER_IP0, HTTP_SERVER_IP1, HTTP_SERVER_IP2, HTTP_SERVER_IP3);

    http_pcb = tcp_new();
    if (http_pcb == NULL) {
        printf("tcp_new failed\r\n");
        return;
    }

    http_state = HTTP_CONNECTING;

    tcp_arg(http_pcb, NULL);
    tcp_recv(http_pcb, http_client_recv);
    tcp_err(http_pcb, http_client_err);

    printf("Connecting to %d.%d.%d.%d:%d ...\r\n",
           HTTP_SERVER_IP0, HTTP_SERVER_IP1, HTTP_SERVER_IP2, HTTP_SERVER_IP3, HTTP_SERVER_PORT);

    ret = tcp_connect(http_pcb, &server_ip, HTTP_SERVER_PORT, http_client_connected);
    printf("tcp_connect ret = %d\r\n", ret);

    if (ret == ERR_OK) {
        printf("assigned local port = %u\r\n", http_pcb->local_port);
    }

    if (ret != ERR_OK) {
        tcp_recv(http_pcb, NULL);
        tcp_err(http_pcb, NULL);
        tcp_arg(http_pcb, NULL);
        tcp_abort(http_pcb);
        http_pcb = NULL;
        http_state = HTTP_IDLE;
    }
}

static err_t http_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    const char *request =
        "GET /slot_b.bin HTTP/1.1\r\n"
        "Host: 10.14.1.111:8000\r\n"
        "Connection: close\r\n"
        "\r\n";

    err_t ret;
    LWIP_UNUSED_ARG(arg);

    printf("connected cb err=%d\r\n", err);

    if (err != ERR_OK) {
        http_pcb = NULL;
        http_state = HTTP_IDLE;
        return err;
    }

    http_state = HTTP_RECEIVING;

    printf("Connected. Sending HTTP GET...\r\n");

    ret = tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (ret != ERR_OK) {
        printf("tcp_write failed: %d\r\n", ret);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_arg(tpcb, NULL);
        tcp_abort(tpcb);
        http_pcb = NULL;
        http_state = HTTP_IDLE;
        return ret;
    }

    ret = tcp_output(tpcb);
    if (ret != ERR_OK) {
        printf("tcp_output failed: %d\r\n", ret);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_arg(tpcb, NULL);
        tcp_abort(tpcb);
        http_pcb = NULL;
        http_state = HTTP_IDLE;
        return ret;
    }

    return ERR_OK;
}

static err_t http_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct pbuf *q;
    LWIP_UNUSED_ARG(arg);

    if (err != ERR_OK) {
        printf("recv error: %d\r\n", err);
        if (p != NULL) {
            pbuf_free(p);
        }
        ota_finish(0);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_arg(tpcb, NULL);
        tcp_abort(tpcb);
        http_pcb = NULL;
        http_state = HTTP_IDLE;
        return err;
    }

    if (p == NULL) {
        err_t close_ret;

        printf("\r\n--- server closed connection ---\r\n");

        if (header_done && (body_received == content_length) && (content_length != 0U)) {
            ota_finish(1);
        } else {
            ota_finish(0);
        }

        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_arg(tpcb, NULL);

        close_ret = tcp_close(tpcb);
        printf("tcp_close ret = %d\r\n", close_ret);
        if (close_ret != ERR_OK) {
            tcp_abort(tpcb);
        }

        http_pcb = NULL;
        http_state = HTTP_IDLE;
        return ERR_OK;
    }

    tcp_recved(tpcb, p->tot_len);

    for (q = p; q != NULL; q = q->next) {
        uint8_t *data = (uint8_t *)q->payload;
        uint32_t len = q->len;
        uint32_t i = 0;

        while (i < len) {
            if (!header_done) {
                if (header_len >= (HTTP_HEADER_BUF_SIZE - 1U)) {
                    printf("HTTP header too large\r\n");
                    pbuf_free(p);
                    ota_finish(0);
                    tcp_recv(tpcb, NULL);
                    tcp_err(tpcb, NULL);
                    tcp_arg(tpcb, NULL);
                    tcp_abort(tpcb);
                    http_pcb = NULL;
                    http_state = HTTP_IDLE;
                    return ERR_ABRT;
                }

                header_buf[header_len++] = (char)data[i++];

                if (header_len >= 4U &&
                    header_buf[header_len - 4] == '\r' &&
                    header_buf[header_len - 3] == '\n' &&
                    header_buf[header_len - 2] == '\r' &&
                    header_buf[header_len - 1] == '\n') {

                    header_buf[header_len] = '\0';
                    header_done = 1;

                    printf("HTTP header parsed\r\n");
                    printf("%s", header_buf);

                    if (ota_parse_content_length() != 0) {
                        pbuf_free(p);
                        ota_finish(0);
                        tcp_recv(tpcb, NULL);
                        tcp_err(tpcb, NULL);
                        tcp_arg(tpcb, NULL);
                        tcp_abort(tpcb);
                        http_pcb = NULL;
                        http_state = HTTP_IDLE;
                        return ERR_ABRT;
                    }

                    if (ota_erase_slot_b(content_length) != HAL_OK) {
                        pbuf_free(p);
                        ota_finish(0);
                        tcp_recv(tpcb, NULL);
                        tcp_err(tpcb, NULL);
                        tcp_arg(tpcb, NULL);
                        tcp_abort(tpcb);
                        http_pcb = NULL;
                        http_state = HTTP_IDLE;
                        return ERR_ABRT;
                    }
                }
            } else {
                uint32_t body_len = len - i;
                if (ota_process_body_bytes(&data[i], body_len) != 0) {
                    pbuf_free(p);
                    ota_finish(0);
                    tcp_recv(tpcb, NULL);
                    tcp_err(tpcb, NULL);
                    tcp_arg(tpcb, NULL);
                    tcp_abort(tpcb);
                    http_pcb = NULL;
                    http_state = HTTP_IDLE;
                    return ERR_ABRT;
                }
                i = len;
            }
        }
    }

    pbuf_free(p);
    return ERR_OK;
}

static void http_client_err(void *arg, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    printf("tcp error callback: %d\r\n", err);
    ota_finish(0);
    http_pcb = NULL;
    http_state = HTTP_IDLE;
}
