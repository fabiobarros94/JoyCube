#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

// Mapeamento de Pinos - Botões Digitais
#define BTN_UP      2
#define BTN_DOWN    3
#define BTN_LEFT    4
#define BTN_RIGHT   5
#define BTN_A       6
#define BTN_B       7
#define BTN_X       8
#define BTN_Y       9
#define BTN_Z       10
#define BTN_START   11
#define BTN_L_CLICK 12
#define BTN_R_CLICK 13

// Mapeamento de Pinos - Multiplexador (CD74HC4067)
#define MUX_S0      14
#define MUX_S1      15
#define MUX_S2      16
#define MUX_S3      17
#define MUX_SIG     26  // ADC0

// Array de botões para inicialização fácil
const uint8_t buttons[] = {
    BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT,
    BTN_A, BTN_B, BTN_X, BTN_Y, BTN_Z,
    BTN_START, BTN_L_CLICK, BTN_R_CLICK
};
#define NUM_BUTTONS (sizeof(buttons)/sizeof(buttons[0]))

// Variáveis Globais de Estado do Protocolo WUP-028
static bool wup_polling_enabled = false;
static uint32_t last_report_time_ms = 0;

// Função para configurar os pinos
void board_hardware_init(void) {
    // Configura os botões digitais como entrada com pull-up
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }

    // Configura os pinos de seleção do multiplexador
    uint8_t mux_pins[] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
    for (int i = 0; i < 4; i++) {
        gpio_init(mux_pins[i]);
        gpio_set_dir(mux_pins[i], GPIO_OUT);
        gpio_put(mux_pins[i], 0);
    }

    // Configura o ADC
    adc_init();
    adc_gpio_init(MUX_SIG);
    adc_select_input(0); // Seleciona ADC0 (GP26)
}

// Função para selecionar o canal no multiplexador (0 a 15)
void select_mux_channel(uint8_t channel) {
    gpio_put(MUX_S0, (channel & 0x01) ? 1 : 0);
    gpio_put(MUX_S1, (channel & 0x02) ? 1 : 0);
    gpio_put(MUX_S2, (channel & 0x04) ? 1 : 0);
    gpio_put(MUX_S3, (channel & 0x08) ? 1 : 0);
    
    // Pequeno delay para estabilização do sinal analógico
    sleep_us(2);
}

// Lê o ADC e converte a resolução de 12-bits (0-4095) para 8-bits (0-255)
// O GameCube nativamente usa 8-bits por eixo.
uint8_t read_analog_8bit(void) {
    uint16_t raw_adc = adc_read();
    return (uint8_t)(raw_adc >> 4); // Divide por 16
}

// Processa dados recebidos do Switch/PC (Comandos do Adaptador)
void process_vendor_commands(void) {
    if (tud_vendor_available()) {
        uint8_t buf[37];
        uint32_t count = tud_vendor_read(buf, sizeof(buf));
        
        if (count > 0) {
            uint8_t cmd = buf[0];
            if (cmd == 0x13) {
                // Comando 0x13: Inicializar polling
                wup_polling_enabled = true;
            }
            if (cmd == 0x11) {
                // Comando 0x11: Comando de Rumble
                // Para 4 controles, os bytes seguintes controlam os motores de vibração.
                // Como não implementamos rumble no hardware, podemos ignorar em silêncio.
            }
        }
    }
}

// Envia o status do GameCube
void wup_task(void) {
    // 1000Hz polling -> Enviar a cada 1ms
    if ( board_millis() - last_report_time_ms < 1) return; 

    // Se o console/host ainda não enviou o comando de início de polling, não enviar status.
    if (!wup_polling_enabled) return;

    if (!tud_vendor_write_available()) return;

    last_report_time_ms = board_millis();

    wup_028_report_t report;
    memset(&report, 0, sizeof(report));

    report.command = 0x21; // Resposta de status dos controles

    // Inicializando as 4 portas (Portas 2, 3 e 4 ficam desconectadas - 0x00)
    for (int i = 0; i < 4; i++) {
        report.port[i].status = GC_STATUS_DISCONNECTED;
        report.port[i].stick_x = 128;   // Posição neutra
        report.port[i].stick_y = 128;
        report.port[i].c_stick_x = 128;
        report.port[i].c_stick_y = 128;
    }

    // Configurando apenas o Controle 1 (Porta 0)
    report.port[0].status = GC_STATUS_CONNECTED;

    // Lendo Botões
    if (!gpio_get(BTN_A))       report.port[0].buttons1 |= (1 << 0);
    if (!gpio_get(BTN_B))       report.port[0].buttons1 |= (1 << 1);
    if (!gpio_get(BTN_X))       report.port[0].buttons1 |= (1 << 2);
    if (!gpio_get(BTN_Y))       report.port[0].buttons1 |= (1 << 3);
    if (!gpio_get(BTN_LEFT))    report.port[0].buttons1 |= (1 << 4);
    if (!gpio_get(BTN_RIGHT))   report.port[0].buttons1 |= (1 << 5);
    if (!gpio_get(BTN_DOWN))    report.port[0].buttons1 |= (1 << 6);
    if (!gpio_get(BTN_UP))      report.port[0].buttons1 |= (1 << 7);

    if (!gpio_get(BTN_START))   report.port[0].buttons2 |= (1 << 0);
    if (!gpio_get(BTN_Z))       report.port[0].buttons2 |= (1 << 1);
    if (!gpio_get(BTN_R_CLICK)) report.port[0].buttons2 |= (1 << 2);
    if (!gpio_get(BTN_L_CLICK)) report.port[0].buttons2 |= (1 << 3);

    // Lendo Eixos Analógicos via Multiplexador
    select_mux_channel(0); report.port[0].stick_x = read_analog_8bit();
    select_mux_channel(1); report.port[0].stick_y = read_analog_8bit();
    select_mux_channel(2); report.port[0].c_stick_x = read_analog_8bit();
    select_mux_channel(3); report.port[0].c_stick_y = read_analog_8bit();
    select_mux_channel(4); report.port[0].l_analog = read_analog_8bit();
    select_mux_channel(5); report.port[0].r_analog = read_analog_8bit();

    // Enviar o report de 37 bytes pelo endpoint IN (Vendor interface)
    tud_vendor_write(&report, sizeof(report));
    tud_vendor_flush(); // Garante o envio imediato
}

int main(void) {
    board_init();
    board_hardware_init();
    tusb_init();

    while (1) {
        tud_task(); // TinyUSB device task
        process_vendor_commands();
        wup_task(); // Logica principal de leitura e envio
    }

    return 0;
}
