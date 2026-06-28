# JoyCube Hardware Design & Netlist

Este documento serve como um guia para ajudar a comunidade a projetar uma Placa de Circuito Impresso (PCB) (ex: encomendando na JLCPCB ou PCBWay) para o projeto JoyCube. Ele contém a tabela completa de conexões (Netlist) e boas práticas de layout.

Se você utilizar softwares como **KiCad** ou **EasyEDA**, você só precisa criar os componentes abaixo no seu esquema (Schematic) e ligá-los exatamente como descrito na tabela. O roteamento (desenho das trilhas de cobre) ficará a seu critério físico.

## 1. Componentes Necessários (BOM para PCB)
- **1x Microcontrolador:** Raspberry Pi Pico (Pico original) ou *Waveshare RP2040-Zero* (se quiser soldar a placa como módulo SMD).
- **1x CI Multiplexador:** CD74HC4067 (Pode ser o módulo pronto ou o Chip SMD `SSOP-24` / `SOIC-24` soldado diretamente).
- **Botões (12x):** Switches mecânicos ou táteis (Cherry MX, Kailh, Omron).
- **Analógicos (6x eixos):** 
  - 2x Módulos de Joystick 3D (Eixos X e Y).
  - 2x Potenciômetros lineares deslizantes (Para os gatilhos L e R Analógicos).
- *(Opcional)*: Conectores JST ou pin headers para conectar os analógicos de forma modular.

## 2. Esquemático Elétrico Lógico (Netlist)

Abaixo está o mapa exato de conexões. No seu software CAD, conecte a **Coluna A** à **Coluna B**.

### Alimentação e Terra (Power & GND)
| Coluna A (Origem) | Coluna B (Destino) | Notas |
| :--- | :--- | :--- |
| RP2040 `3V3 (Out)` | CD74HC4067 `VCC` | Alimenta o CI multiplexador. NUNCA use 5V (VBUS). |
| RP2040 `3V3 (Out)` | Potenciômetros (Pino Superior) | Alimenta os sensores analógicos. |
| RP2040 `GND` | CD74HC4067 `GND` e `EN` | O pino EN (Enable) do MUX deve estar aterrado (ativo em LOW). |
| RP2040 `GND` | Todos os 12 Botões (Pino 2) | Fechamento de circuito dos botões digitais. |
| RP2040 `GND` | Potenciômetros (Pino Inferior) | Referência terra dos analógicos. |

### Lógica Digital (Botões - Fecham com GND)
*Os botões não precisam de resistores externos no esquema, o RP2040 usará Pull-ups internos.*
| Botão (Pino 1) | Pino RP2040 |
| :--- | :--- |
| D-Pad Up | GP2 |
| D-Pad Down | GP3 |
| D-Pad Left | GP4 |
| D-Pad Right| GP5 |
| Botão A | GP6 |
| Botão B | GP7 |
| Botão X | GP8 |
| Botão Y | GP9 |
| Botão Z | GP10 |
| Start | GP11 |
| L-Click (Gatilho Fundo) | GP12 |
| R-Click (Gatilho Fundo) | GP13 |

### Lógica do Multiplexador (Controle e Sinal)
| CD74HC4067 Pino | Pino RP2040 |
| :--- | :--- |
| `S0` | GP14 |
| `S1` | GP15 |
| `S2` | GP16 |
| `S3` | GP17 |
| `SIG` (Z) | GP26 (ADC0) |

### Entradas Analógicas (Vêm dos Potenciômetros)
O pino central (Wiper) de cada potenciômetro deve ir para as seguintes portas do CD74HC4067:
| Sinal Analógico (Pino Central) | Pino CD74HC4067 |
| :--- | :--- |
| Main Stick - Eixo X | `C0` |
| Main Stick - Eixo Y | `C1` |
| C-Stick - Eixo X | `C2` |
| C-Stick - Eixo Y | `C3` |
| L-Trigger Analógico | `C4` |
| R-Trigger Analógico | `C5` |

*(As portas C6 a C15 do CD74HC4067 devem ficar desconectadas).*

---

## 3. Guia Rápido para EasyEDA (JLCPCB)

1. Crie um novo projeto no site do [EasyEDA](https://easyeda.com/).
2. No esquemático, busque pelas partes: "Raspberry Pi Pico" e "CD74HC4067" (prefira os módulos se não tiver experiência com solda SMD minúscula).
3. Desenhe os fios interligando os pinos conforme a tabela (Netlist) acima.
4. Clique em "Design -> Update PCB".
5. Na tela da placa (PCB), desenhe o formato do seu controle.
6. Posicione os botões onde seus dedos irão tocar (você pode medir ou usar templates de Joycons na internet).
7. Roteie (ligue as trilhas) com largura de pelo menos `0.254mm` para sinal e `0.5mm` para VCC/GND.
8. Gere os arquivos Gerber e submeta no site da fabricante (JLCPCB, PCBWay, etc.).
