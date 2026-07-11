/* ============================================================
   PROJETO: Automação IoT para Estufa FVH - Chácara Canta Galo
   EQUIPE 06 - CEEP Olegário Macedo - Castro/PR - 2026
   PROPOSTA: Sra. Marisa Diovani Seracaus Dekkers
   DESCRIÇÃO: Monitora temperatura, umidade, pressão e luminosidade
              da estufa de Forragem Verde Hidropônica de cevada,
              enviando dados para página web e Telegram.
   ============================================================ */

// ==================== BIBLIOTECAS ====================
#include <WiFi.h>              // Permite conectar o ESP32 ao Wi-Fi
#include <WebServer.h>         // Cria um servidor web no ESP32 (página de controle)
#include <HTTPClient.h>        // Permite fazer requisições HTTP (usado no Telegram)
#include <Wire.h>              // Comunicação I2C (usada pelo sensor BMP180)
#include <DHT.h>               // Biblioteca para o sensor DHT11 (temperatura e umidade)
#include <Adafruit_BMP085.h>   // Biblioteca para o sensor BMP180 (pressão e altitude)
#include <time.h>              // Para pegar a data e hora da internet (NTP)
#include <math.h>              // Para cálculos matemáticos (log, ponto de orvalho)

// ==================== DEFINIÇÃO DE PINOS ====================
// Define em quais pinos do ESP32 cada componente está conectado
#define DHTPIN 16       // Pino onde o sensor DHT11 está ligado
#define DHTTYPE DHT11   // Tipo do sensor DHT (pode ser DHT11 ou DHT22)
#define PINO_LDR 34     // Pino analógico onde o LDR (sensor de luz) está ligado
#define PINO_RELE 27    // Pino digital que controla o Relé (motobomba de irrigação)

// ==================== CONFIGURAÇÕES DE REDE ====================
// Coloque aqui o nome e senha da sua rede Wi-Fi
const char* ssid = "xxxxx";           // Nome da rede Wi-Fi (SSID)
const char* password = "xxxxxxxxxxx"; // Senha do Wi-Fi

// ==================== CONFIGURAÇÕES DO TELEGRAM ====================
// Token do Bot (fornecido pelo @BotFather no Telegram)
const char* botToken = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
// ID do chat/grupo que receberá as mensagens
const char* chatID = "xxxxxxxxxxxxxxx";

// ==================== OBJETOS DOS SENSORES ====================
DHT dht(DHTPIN, DHTTYPE);         // Cria o objeto do sensor DHT11
Adafruit_BMP085 bmp;              // Cria o objeto do sensor BMP180
WebServer server(80);             // Cria o servidor web na porta 80 (padrão HTTP)

// ==================== VARIÁVEIS GLOBAIS ====================
bool estadoRele = false;  // Guarda se o relé está ligado (true) ou desligado (false)

// ============================================================
// FUNÇÃO: calcularPontoOrvalho
// O QUE FAZ: Calcula o ponto de orvalho (temperatura em que o
//            vapor d'água começa a condensar).
// POR QUE USAR: Importante para saber se há risco de fungos na
//               estufa de FVH.
// FÓRMULA: Usa a equação de Magnus (aproximação matemática)
// ============================================================
float calcularPontoOrvalho(float temp, float umid) {
  const float a = 17.62, b = 243.5;  // Constantes da fórmula de Magnus
  // Calcula o valor alfa (parte da fórmula)
  float alpha = ((a * temp) / (b + temp)) + log(umid / 100.0);
  // Retorna o ponto de orvalho em °C
  return (b * alpha) / (a - alpha);
}

// ============================================================
// FUNÇÃO: obterHoraFormatada
// O QUE FAZ: Pega a data e hora atual da internet (servidor NTP)
//            e retorna no formato "dd/mm/aaaa HH:MM:SS"
// POR QUE USAR: Para registrar o horário exato das notificações
//               enviadas ao Telegram da Sra. Marisa
// ============================================================
String obterHoraFormatada() {
  struct tm timeinfo;  // Variável para guardar as informações de tempo
  // Se não conseguir pegar a hora, retorna mensagem de erro
  if (!getLocalTime(&timeinfo)) return "Horário indisponível";
  char buffer[30];  // Vetor para guardar a string formatada
  // Formata a data/hora no padrão brasileiro
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);  // Retorna como String do Arduino
}

// ============================================================
// FUNÇÃO: enviarTelegram
// O QUE FAZ: Envia uma mensagem de texto para o grupo do Telegram
//            configurado (chatID) usando o Bot (botToken)
// COMO FUNCIONA: Monta a URL da API do Telegram e faz uma
//                requisição HTTP GET
// ============================================================
void enviarTelegram(String mensagem) {
  HTTPClient http;  // Cria um objeto HTTP
  // Monta a URL completa da API do Telegram com o token, chatID e mensagem
  String url = "https://api.telegram.org/bot" + String(botToken) 
               + "/sendMessage?chat_id=" + String(chatID) 
               + "&text=" + mensagem;
  http.begin(url);  // Inicia a conexão com a URL
  http.GET();       // Envia a requisição GET (envia a mensagem)
  http.end();       // Encerra a conexão
}

// ============================================================
// FUNÇÃO: ligarRele
// O QUE FAZ: Liga o relé (aciona a motobomba de irrigação da FVH)
//            e envia notificação ao Telegram
// QUANDO É CHAMADA: Quando o usuário clica no botão "Ligar" na
//                   página web do ESP32
// ============================================================
void ligarRele() {
  estadoRele = true;                  // Atualiza a variável de estado
  digitalWrite(PINO_RELE, HIGH);      // Envia sinal HIGH para o pino do relé (LIGA)
  // Envia mensagem ao Telegram informando o horário
  enviarTelegram("💧 Irrigação da FVH iniciada em " + obterHoraFormatada());
  // Redireciona o navegador de volta para a página principal
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");  // Código 302 = redirecionamento
}

// ============================================================
// FUNÇÃO: desligarRele
// O QUE FAZ: Desliga o relé (para a motobomba) e notifica o Telegram
// QUANDO É CHAMADA: Quando o usuário clica em "Desligar" na web
// ============================================================
void desligarRele() {
  estadoRele = false;                 // Atualiza a variável de estado
  digitalWrite(PINO_RELE, LOW);       // Envia sinal LOW para o pino do relé (DESLIGA)
  enviarTelegram("✅ Irrigação OK, desligando em " + obterHoraFormatada());
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// ============================================================
// FUNÇÃO: handleRoot
// O QUE FAZ: Gera a página HTML principal (painel de controle)
//            que é exibida quando o usuário acessa o IP do ESP32
// CONTEÚDO: Mostra todos os dados dos sensores e o botão de controle
// ============================================================
void handleRoot() {
  // ----- LEITURA DOS SENSORES -----
  float t = dht.readTemperature();          // Lê a temperatura (°C)
  float u = dht.readHumidity();             // Lê a umidade relativa (%)
  float hi = dht.computeHeatIndex(t, u, false);  // Calcula sensação térmica
  float dp = calcularPontoOrvalho(t, u);    // Calcula ponto de orvalho
  float p = bmp.readPressure() / 100.0;     // Lê pressão (converte para hPa)
  float alt = bmp.readAltitude();           // Lê altitude estimada
  int ldrRaw = analogRead(PINO_LDR);        // Lê valor bruto do LDR (0-4095)
  float ldrPercent = (ldrRaw / 4095.0) * 100.0;  // Converte para porcentagem

  // ----- MONTAGEM DA PÁGINA HTML -----
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Estufa FVH - Chácara Canta Galo</title>";
  // Meta tag para página responsiva (funciona bem no celular)
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  
  // ----- ESTILO CSS (cores e formatação) -----
  // Fundo verde claro (remete à agricultura), textos em verde escuro
  html += "<style>body{font-family:Arial;background:#e8f5e9;padding:20px;text-align:center;}";
  html += "h2{color:#2e7d32;} h4{color:#1b5e20;}";
  html += "ul{list-style:none;padding:0;text-align:left;display:inline-block;}";
  html += "li{margin:8px 0;}";
  // Estilo do botão (verde, grande, arredondado)
  html += ".botao{padding:15px 30px;font-size:20px;background:#2e7d32;color:white;border:none;border-radius:10px;cursor:pointer;margin-top:20px;}</style>";
  html += "</head><body>";
  
  // ----- CABEÇALHO DA PÁGINA -----
  html += "<h2>🌱 Estufa FVH - Chácara Canta Galo</h2>";
  html += "<h4>Produtora: Sra. Marisa Dekkers</h4><ul>";
  
  // ----- LISTA DE DADOS DOS SENSORES -----
  html += "<li>🌡️ Temperatura: " + String(t) + " °C</li>";
  html += "<li>💧 Umidade: " + String(u) + "%</li>";
  html += "<li>🔥 Sensação térmica: " + String(hi) + " °C</li>";
  html += "<li>💦 Ponto de orvalho: " + String(dp) + " °C</li>";
  html += "<li>🎈 Pressão: " + String(p) + " hPa</li>";
  html += "<li>🏔️ Altitude: " + String(alt) + " m</li>";
  html += "<li>💡 Luminosidade: " + String(ldrPercent, 1) + "%</li>";
  // Mostra se a irrigação está ligada ou desligada
  html += "<li>🚿 Status Irrigação: <b>" + String(estadoRele ? "Ligado" : "Desligado") + "</b></li>";
  html += "</ul>";

  // ----- BOTÃO DE CONTROLE DO RELÉ -----
  // O botão muda de texto conforme o estado atual do relé
  html += "<form action='/" + String(estadoRele ? "desligar" : "ligar") + "' method='get'>";
  html += "<button class='botao'>" + String(estadoRele ? "Desligar Irrigação" : "Ligar Irrigação") + "</button></form>";
  
  // ----- RODAPÉ COM DATA/HORA -----
  html += "<p style='color:gray;'>Data e hora: " + obterHoraFormatada() + "</p>";
  html += "</body></html>";
  
  // Envia a página HTML completa para o navegador do usuário
  server.send(200, "text/html", html);  // Código 200 = sucesso
}

// ============================================================
// FUNÇÃO: setup
// O QUE FAZ: Executa UMA VEZ ao ligar o ESP32.
//            Configura todos os componentes e faz as conexões.
// ============================================================
void setup() {
  // Inicia a comunicação serial para mostrar mensagens no monitor
  Serial.begin(115200);
  
  // Inicia a comunicação I2C nos pinos 21 (SDA) e 22 (SCL) do ESP32
  Wire.begin(21, 22);
  
  // Inicia o sensor DHT11
  dht.begin();
  
  // Configura os pinos como entrada ou saída
  pinMode(PINO_LDR, INPUT);       // LDR é entrada (lê valor)
  pinMode(PINO_RELE, OUTPUT);     // Relé é saída (controla componente)
  digitalWrite(PINO_RELE, LOW);   // Garante que o relé comece DESLIGADO

  // ----- VERIFICA SE O SENSOR BMP180 ESTÁ CONECTADO -----
  if (!bmp.begin()) {
    Serial.println("BMP180 não detectado. Verifique as conexões.");
    while (true);  // Trava o programa se o sensor não for encontrado
  }

  // ----- CONECTA AO WI-FI -----
  WiFi.begin(ssid, password);
  Serial.print("Conectando Wi-Fi");
  // Aguarda até conseguir conectar (mostra pontinhos no monitor)
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado! IP: http://" + WiFi.localIP().toString());

  // ----- SINCRONIZA O RELÓGIO VIA INTERNET (NTP) -----
  // Ajusta para o fuso horário de Brasília (UTC-3)
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // Aguarda até receber a hora correta
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nHorário sincronizado");

  // ----- CONFIGURA AS ROTAS DO SERVIDOR WEB -----
  // Quando acessar "/" (raiz), chama a função handleRoot
  server.on("/", handleRoot);
  // Quando acessar "/ligar", chama a função ligarRele
  server.on("/ligar", ligarRele);
  // Quando acessar "/desligar", chama a função desligarRele
  server.on("/desligar", desligarRele);
  
  // Inicia o servidor web
  server.begin();
  Serial.println("Servidor web iniciado!");
}

// ============================================================
// FUNÇÃO: loop
// O QUE FAZ: Executa EM LOOP CONTÍNUO enquanto o ESP32 estiver ligado.
//            Fica "escutando" as requisições do navegador.
// ============================================================
void loop() {
  // Fica verificando se algum cliente (celular/PC) acessou o ESP32
  server.handleClient();
  
  // Observação: Poderíamos adicionar aqui leituras periódicas dos
  // sensores e envio automático de alertas ao Telegram caso a
  // temperatura ou umidade saiam da faixa ideal para a cevada.
}
