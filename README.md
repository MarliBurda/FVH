# 🌱 Estufa FVH - Chácara Canta Galo
### Automação IoT para Cultivo de Forragem Verde Hidropônica de Cevada

![ESP32](https://img.shields.io/badge/ESP32-IoT-blue)
![Arduino IDE](https://img.shields.io/badge/Arduino_IDE-Compatible-green)
![Telegram Bot](https://img.shields.io/badge/Telegram-Bot-0088cc)
![License](https://img.shields.io/badge/License-MIT-yellow)
![FECCI Jovem 2026](https://img.shields.io/badge/FECCI_Jovem-2026-orange)

> 🏆 **Projeto desenvolvido pela Equipe 06** para o **FECCI Jovem 2026** - Tema: *"Agro Forte, Futuro Sustentável: Equilíbrio entre Produção e Meio Ambiente"*

---

## 📖 Sobre o Projeto

Este repositório contém o código-fonte e a documentação do projeto de **Agro Robótica** desenvolvido por estudantes do **CEEP Olegário Macedo** (Castro-PR), voltado para a automação de uma estufa de **Forragem Verde Hidropônica (FVH) de cevada**.

O sistema foi desenvolvido para atender uma demanda real da **Chácara Canta Galo**, de propriedade da produtora rural **Sra. Marisa Diovani Seracaus Dekkers**, que buscava uma solução tecnológica de baixo custo para garantir alimentação nutritiva ao seu rebanho leiteiro durante todo o ano, independentemente das oscilações climáticas da região dos Campos Gerais.

### 🎯 O Problema
As oscilações climáticas decorrentes do aquecimento global têm intensificado períodos de estiagem prolongada nas bacias leiteiras do Paraná, comprometendo a estabilidade nutricional de pastagens tradicionais.

### 💡 A Solução
Um sistema **IoT (Internet das Coisas)** que monitora e controla automaticamente:
- 🌡️ Temperatura
- 💧 Umidade relativa do ar
- 🎈 Pressão atmosférica
- 💡 Luminosidade
- 🚿 Irrigação automatizada via relé

Com notificações em tempo real enviadas via **Telegram** e painel de controle acessível por **navegador web** (hospedado no próprio ESP32).

---

## 👥 Equipe 06

| Integrante | Função |
|------------|--------|
| Maria Eduarda Santos Mira | Desenvolvedora |
| Valesca Vitória de Proença Bueno | Desenvolvedora |
| Gabriel Wilpert | Desenvolvedor |
| Julia Oliveira Silva | Desenvolvedora |
| Danilo Penteado | Desenvolvedor |

**Orientadora:** Prof.ª Marli Burda  
**Instituição:** Centro Estadual de Educação Profissional Olegário Macedo - Castro/PR  
**INEP:** 41374029

---

## ⚙️ Tecnologias Utilizadas

- **Microcontrolador:** ESP32 NodeMCU 32S
- **Linguagem:** C++ (Arduino IDE)
- **Protocolo:** HTTP / Wi-Fi / I2C
- **API:** Telegram Bot API
- **Servidor:** WebServer embarcado no ESP32
- **Sincronização:** NTP (Network Time Protocol)

---

## 🔌 Componentes Eletrônicos

| Componente | Função | Pino ESP32 |
|------------|--------|------------|
| **ESP32 NodeMCU 32S** | Microcontrolador com Wi-Fi | - |
| **DHT11** | Sensor de temperatura e umidade | GPIO 16 |
| **BMP180** | Sensor de pressão e altitude | I2C (SDA:21, SCL:22) |
| **LDR** | Sensor de luminosidade | GPIO 34 |
| **Módulo Relé 1 Canal** | Controle da motobomba | GPIO 27 |
| **Protoboard + Jumpers** | Conexões | - |

### 📐 Diagrama de Ligação (Resumo)
