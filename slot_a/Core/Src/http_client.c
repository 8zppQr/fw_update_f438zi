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
//static u16_t next_local_port = 40000;
//
//void http_client_start(void)
//{
//    ip_addr_t server_ip;
//    err_t ret;
//    u16_t local_port;
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
//    local_port = next_local_port++;
//    if (next_local_port >= 50000) {
//        next_local_port = 40000;
//    }
//
//    ret = tcp_bind(http_pcb, IP_ADDR_ANY, local_port);
//    if (ret != ERR_OK) {
//        printf("tcp_bind failed: %d\r\n", ret);
//        tcp_abort(http_pcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return;
//    }
//
//    http_state = HTTP_CONNECTING;
//
//    tcp_arg(http_pcb, NULL);
//    tcp_recv(http_pcb, http_client_recv);
//    tcp_err(http_pcb, http_client_err);
//
//    printf("Connecting to 10.14.1.111:8000 ...\r\n");
//    ret = tcp_connect(http_pcb, &server_ip, 8000, http_client_connected);
//    if (ret != ERR_OK) {
//        printf("tcp_connect failed: %d\r\n", ret);
//        tcp_abort(http_pcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//    }
//}
//
//static err_t http_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
//{
//    const char *request =
//        "GET /test.txt HTTP/1.1\r\n"
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
//
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//        tcp_abort(tpcb);
//
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return ret;
//    }
//
//    ret = tcp_output(tpcb);
//    if (ret != ERR_OK) {
//        printf("tcp_output failed: %d\r\n", ret);
//
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_arg(tpcb, NULL);
//        tcp_abort(tpcb);
//
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
//        if (p) pbuf_free(p);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return err;
//    }
//
//    if (p == NULL) {
//        printf("\r\n--- server closed connection ---\r\n");
//        tcp_recv(tpcb, NULL);
//        tcp_err(tpcb, NULL);
//        tcp_close(tpcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return ERR_OK;
//    }
//
//    tcp_recved(tpcb, p->tot_len);
//
//    for (q = p; q != NULL; q = q->next) {
//        HAL_UART_Transmit(&huart3, (uint8_t *)q->payload, q->len, 0xFFFF);
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

#include "lwip/tcp.h"
#include "lwip/ip_addr.h"

extern UART_HandleTypeDef huart3;

#define HTTP_SERVER_IP0   10
#define HTTP_SERVER_IP1   14
#define HTTP_SERVER_IP2   1
#define HTTP_SERVER_IP3   111
#define HTTP_SERVER_PORT  8000

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

void http_client_start(void)
{
    ip_addr_t server_ip;
    err_t ret;
    uint32_t seed = DWT->CYCCNT;
    u16_t local_port;
    local_port = 40000 + (seed % 20000);

    printf("http_client_start() called, state=%d\r\n", http_state);

    if (http_state != HTTP_IDLE || http_pcb != NULL) {
        printf("HTTP client busy\r\n");
        return;
    }

    IP4_ADDR(&server_ip, HTTP_SERVER_IP0, HTTP_SERVER_IP1, HTTP_SERVER_IP2, HTTP_SERVER_IP3);

    http_pcb = tcp_new();
    if (http_pcb == NULL) {
        printf("tcp_new failed\r\n");
        return;
    }

//    ret = tcp_bind(http_pcb, IP_ADDR_ANY, local_port);
//    printf("tcp_bind local port = %u, ret = %d\r\n", local_port, ret);
//    if (ret != ERR_OK) {
//        tcp_abort(http_pcb);
//        http_pcb = NULL;
//        http_state = HTTP_IDLE;
//        return;
//    }

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
        "GET /test.txt HTTP/1.1\r\n"
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
        HAL_UART_Transmit(&huart3, (uint8_t *)q->payload, q->len, 0xFFFF);
    }

    pbuf_free(p);
    return ERR_OK;
}

static void http_client_err(void *arg, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    printf("tcp error callback: %d\r\n", err);
    http_pcb = NULL;
    http_state = HTTP_IDLE;
}
