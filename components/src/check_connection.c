/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Checks network connectivity by pinging configured host

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "ping/ping_sock.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "esp_wifi.h"

#define SUCCESS (1)
#define FAIL (2)
#define HOST "www.google.com"
#define DEFAULT_SCAN_LIST_SIZE 5
extern uint8_t isWifiConnected;
static const char *TAG = "check_connection";

bool conncetion_check = false;

static void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    ESP_LOGI(TAG, "%" PRIu32 " bytes from %s icmp_seq=%d ttl=%d time=%" PRIu32 " ms",
             recv_len, inet_ntoa(target_addr.u_addr.ip4), seqno, ttl, elapsed_time);
    conncetion_check = false;
}

static void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    ESP_LOGE(TAG, "From %s icmp_seq=%d timeout", inet_ntoa(target_addr.u_addr.ip4), seqno);
    conncetion_check = true;
}

static void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    EventGroupHandle_t events = args;
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    if (IP_IS_V4(&target_addr))
    {
        ESP_LOGI(TAG, "\n--- %s ping statistics ---", inet_ntoa(*ip_2_ip4(&target_addr)));
    }
    else
    {
        ESP_LOGI(TAG, "\n--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
    }
    ESP_LOGI(TAG, "%" PRIu32 " packets transmitted, %" PRIu32 " received, %" PRIu32 "%% packet loss, time %" PRIu32 "ms\n",
             transmitted, received, loss, total_time_ms);
    xEventGroupSetBits(events, received == 0 ? FAIL : SUCCESS);
}

esp_err_t check_connectivity(const char *host)
{
    EventGroupHandle_t events = xEventGroupCreate();

    ip_addr_t target_addr;
    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    memset(&target_addr, 0, sizeof(target_addr));
    /* convert domain name to IP address */
    if (getaddrinfo(host, NULL, &hint, &res) != 0)
    {
        ESP_LOGE(TAG, "ping: unknown host %s\n", host);
        return ESP_ERR_NOT_FOUND;
    }
    if (res->ai_family == AF_INET)
    {
        struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
        inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    }
    else
    {
        struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
        inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
    }
    freeaddrinfo(res);

    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr = target_addr;
    // config.interval_ms = 10000;
    config.timeout_ms = 10000;
    // config.count = 10;

    esp_ping_callbacks_t cbs = {
        .on_ping_success = cmd_ping_on_ping_success,
        .on_ping_timeout = cmd_ping_on_ping_timeout,
        .on_ping_end = cmd_ping_on_ping_end,
        .cb_args = events};
    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);
    vTaskDelay(pdMS_TO_TICKS(config.count * config.interval_ms));
    EventBits_t bits = xEventGroupWaitBits(events, FAIL | SUCCESS, pdFALSE, pdFALSE, portMAX_DELAY);

    vEventGroupDelete(events);
    esp_ping_delete_session(ping);

    return bits == SUCCESS ? ESP_OK : ESP_FAIL;
}

void signal_strength(int8_t *rssi)
{
    if((*rssi > -90) && (*rssi <= -80))
    {
        ESP_LOGI(TAG, "Very Poor Signal Strength");
    }
    else if((*rssi > -80) && (*rssi <= -70))
    {
        ESP_LOGI(TAG, "Poor Signal Strength");
    }
    else if((*rssi > -70) && (*rssi <= -60))
    {
        ESP_LOGI(TAG, "Fair Signal Strength");
    }
    else if((*rssi > -60) && (*rssi <= -50))
    {
        ESP_LOGI(TAG, "Good Signal Strength");
    }
    else 
    {
        ESP_LOGI(TAG, "Excellent Signal Strength");
    }
return;
}

uint8_t ping_check(void)
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    if (isWifiConnected)
    {
        esp_wifi_scan_start(NULL, true);

        ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
        ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
        for (int i = 0; i < number; i++)
        {
            ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
            ESP_LOGI(TAG, "Channel \t\t%d", ap_info[i].primary);
            ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
            signal_strength(&ap_info[i].rssi);
        }

        esp_err_t connect_status = check_connectivity(HOST);
        if (connect_status == ESP_OK)
        {
            // connectivity ok
            return 1;
        }
    }
    return 0;
}