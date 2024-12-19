/*
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <stdio.h>
#include <string.h>

#define WIFI_SSID "NomeDaRedeWiFi"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "SenhaDaRedeWiFi" // Substitua pela senha da sua rede Wi-Fi
#define TCP_PORT 80                // Porta para o servidor HTTP

// Função de callback para o servidor HTTP
static err_t http_server_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }
    // Processar a requisição
    const char *response = 
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html><body><h1>Pico W HTTP Server</h1>"
        "<p><a href=\"/led/on\">Ligar LED</a></p>"
        "<p><a href=\"/led/off\">Desligar LED</a></p></body></html>";

    char *request = (char *)p->payload;
    printf("Requisição recebida:\n%s\n", request);

    // Checar comandos de ligar/desligar o LED
    if (strstr(request, "GET /led/on")) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
    } else if (strstr(request, "GET /led/off")) {
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
    }

    // Enviar resposta
    tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_recved(tpcb, p->len);
    pbuf_free(p);

    return ERR_OK;
}

// Função para inicializar o servidor TCP
static void init_http_server() {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar socket TCP\n");
        return;
    }

    if (tcp_bind(pcb, IP_ADDR_ANY, TCP_PORT) != ERR_OK) {
        printf("Erro ao vincular à porta %d\n", TCP_PORT);
        return;
    }

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_server_callback);
    printf("Servidor HTTP inicializado na porta %d\n", TCP_PORT);
}

// Função principal
int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar CYW43\n");
        return -1;
    }

    printf("Conectando ao Wi-Fi...\n");
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha na conexão Wi-Fi!\n");
        return -1;
    }

    printf("Conectado ao Wi-Fi. IP: %s\n", ip4addr_ntoa(netif_default->ip_addr));

    // Configurar LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Iniciar servidor HTTP
    init_http_server();

    while (true) {
        cyw43_arch_poll();
        sleep_ms(100);
    }

    cyw43_arch_deinit();
    return 0;
}

*/
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

#define LED_PIN 12          // Define o pino do LED
#define WIFI_SSID "Mauro"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "mprilip62165886" // Substitua pela senha da sua rede Wi-Fi

// Buffer para respostas HTTP
/*
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" \
                      "<!DOCTYPE html><html><body>" \
                      "<h1>Controle do LED</h1>" \
                      "<p><a href=\"/led/on\">Ligar LED</a></p>" \
                      "<p><a href=\"/led/off\">Desligar LED</a></p>" \
                      "</body></html>\r\n"

*/
#define HTTP_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"\
                      "<html>"\
                      "<head>"\
                      "<style>body { font-family: Arial; }</style>"\
                      "</head>"\
                      "<body>"\
                      "<h1>Servidor da Pico W</h1>"\
                      "<button onclick=\"fetch('/led/on')\">Ligar LED</button>"\
                      "<button onclick=\"fetch('/led/off')\">Desligar LED</button>"\
                      "</body>"\
                      "</html>"



// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Processa a requisição HTTP
    char *request = (char *)p->payload;

    if (strstr(request, "GET /led/on")) {
        gpio_put(LED_PIN, 1);  // Liga o LED
    } else if (strstr(request, "GET /led/off")) {
        gpio_put(LED_PIN, 0);  // Desliga o LED
    }

    // Envia a resposta HTTP
    tcp_write(tpcb, HTTP_RESPONSE, strlen(HTTP_RESPONSE), TCP_WRITE_FLAG_COPY);

    // Libera o buffer recebido
    pbuf_free(p);

    return ERR_OK;
}

// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    return ERR_OK;
}

// Função de setup do servidor TCP
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Liga o servidor na porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }

    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão

    printf("Servidor HTTP rodando na porta 80...\n");
}

int main() {
    stdio_init_all();  // Inicializa a saída padrão
    sleep_ms(10000);
    printf("Iniciando servidor HTTP\n");

    // Inicializa o Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    printf("Wi-Fi conectado!\n");
    printf("Para ligar ou desligar o LED acesse o Endereço IP seguido de /led/on ou /led/off\n");

    // Configura o LED como saída
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicia o servidor HTTP
    start_http_server();
    
    // Loop principal
    while (true) {
        cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);
    }

    cyw43_arch_deinit();  // Desliga o Wi-Fi (não será chamado, pois o loop é infinito)
    return 0;
}