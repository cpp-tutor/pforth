#include "pforth_config.h"

#if PFORTH_WEB_CONSOLE
#if ( defined(ARDUINO_GIGA) && defined(ARDUINO_ARCH_MBED) )
#define WEBSOCKETS_USE_GIGA_R1_WIFI 1
#elif ( defined(ARDUINO_PORTENTA_H7_M7) && defined(ARDUINO_ARCH_MBED) )
#define WEBSOCKETS_USE_PORTENTA_H7_WIFI 1
#else
#error Unsupported network device for WebSockets
#endif
#include <WiFi.h>
#include <WebSockets2_Generic.h>
#include "arduino_secrets.h"
#include "webpage.h"

const char ssid[] = SECRET_SSID, password[] = SECRET_PASS;
const uint16_t websockets_server_port = 8080;
String ip_address, inputbuf, outputbuf;

WiFiServer webserver(80);

using namespace websockets2_generic;

WebsocketsServer server;
WebsocketsClient client;
WebsocketsMessage msg;
#endif

#if PFORTH_USE_SDRAM
#include <SDRAM.h>
#endif

#include <pforth.h>
// Note: The following header is needed for extension function definition
#include <pf_all.h>

void setup() {
  Serial.begin(9600);
  while (!Serial && (millis() < 5000)) {
    delay(100);
  }
#if PFORTH_WEB_CONSOLE
  delay(1000);
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print(".");
    delay(2000);
  }

  Serial.print("\nConnected to ");
  Serial.println(ssid);

  IPAddress ip = WiFi.localIP();
  ip_address = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  server.listen(websockets_server_port);
  Serial.print("http://");
  Serial.println(ip_address);
  webserver.begin();
#endif
}

void loop() {
  char IfInit = 0;
  const char *DicName = nullptr;
  const char *SourceName = nullptr;
#if PFORTH_WEB_CONSOLE
  if (poll_webserver(10000)) {
#endif
    pfDoForth(DicName, SourceName, IfInit);
    Serial.println("Forth session ended.");
    delay(500);
#if PFORTH_WEB_CONSOLE
    server.listen(websockets_server_port);
  }
#endif
}

#if PFORTH_WEB_CONSOLE
int poll_webserver(unsigned long poll_time) {
  unsigned long until = millis() + poll_time;
  while (millis() < until) {
    // listen for incoming clients
    WiFiClient webclient = webserver.available();
    if (webclient) {
      boolean currentLineIsBlank = true;
      while (webclient.connected()) {
        if (webclient.available()) {
          char c = webclient.read();
          if (c == '\n' && currentLineIsBlank) {
            // send a standard HTTP response header
            webclient.println("HTTP/1.1 200 OK");
            webclient.println("Content-Type: text/html");
            webclient.println("Connection: close");  // the connection will be closed after completion of the response
            webclient.println();
            String ip_and_port = ip_address + ":" + String(websockets_server_port), html = webpage;
            html.replace("%LOCAL_ADDRESS_AND_PORT%", ip_and_port);
            webclient.println(html);
            Serial.println("Page served.");
            break;
          }
          if (c == '\n') {
            currentLineIsBlank = true;
          } else if (c != '\r') {
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      delay(10);
      webclient.stop();
      return 1;
    }
    delay(20);
  }
  return 0;
}
#endif

extern "C" {

// note sd prefix is "System Dependent"

int sdTerminalOut(char c) {
#if PFORTH_WEB_CONSOLE
  if (c && (c != '\n')) {
    outputbuf += c;
    return c;
  }
  outputbuf += '\n';
  if (client.available()) {
    client.send(outputbuf.c_str(), outputbuf.length());
    Serial.print(outputbuf);
  }
  else {
    Serial.println("Unable to send.");
  }
  outputbuf = "";
#else
  Serial.write(c);
#endif
  return c;
}

int sdTerminalEcho(char c) {
  return !PFORTH_WEB_CONSOLE;  // Note: Don't echo to Web Console
}

int sdTerminalIn(void) {
#if PFORTH_WEB_CONSOLE
  if (!inputbuf.length()) {
    while (!client.available()) {
      Serial.println("Reconnecting to terminal input.");
      client = server.accept();
      poll_webserver(100);
    }
    while (!inputbuf.length()) {
      WebsocketsMessage msg = client.readNonBlocking();
      inputbuf = msg.data();
      poll_webserver(50);
    }
  }
  int c = inputbuf.charAt(0);
  inputbuf = inputbuf.substring(1); 
  return c;
#else
  for (;;) {
    if (Serial.available() > 0) {
      return Serial.read();
    }
    delay(10);
  }
#endif
}

int sdQueryTerminal(void) {
  return 0;
}

int sdTerminalFlush(void) {
#if PFORTH_WEB_CONSOLE
  if (client.available()) {
    client.send(outputbuf.c_str(), outputbuf.length());
  }
  else {
    Serial.println("Unable to flush.");
  }
  outputbuf = "";
#else
  Serial.flush();
#endif
  return 0;
}

void sdTerminalInit(void) {
#if PFORTH_WEB_CONSOLE
  Serial.println("Connecting to terminal input.");
  client = server.accept();
  while (!client.available()) {
    poll_webserver(100);
    client = server.accept();
  }
#endif
}

void sdTerminalTerm(void) {
#if PFORTH_WEB_CONSOLE
  Serial.println("Disconnecting from terminal input.");
  client.close();
#endif
}

cell_t sdSleepMillis(cell_t msec) {
  delay(msec);
  return msec;
}

int sdInit() {
#if PFORTH_USE_SDRAM
  return SDRAM.begin();
#endif
}

void *sdMalloc(size_t sz) {
#if PFORTH_USE_SDRAM
  return SDRAM.malloc(sz);
#else
  return malloc(sz);
#endif
}

void sdFree(void *ptr) {
#if PFORTH_USE_SDRAM
  SDRAM.free(ptr);
#else
  free(ptr);
#endif
}

#if PFORTH_FUNCTIONS

static cell_t pf_millis();
static cell_t pf_micros();
static void pf_delay(cell_t ms);
static void pf_delayMicroseconds(cell_t us);
static void pf_pinMode(cell_t pin, cell_t mode);
static void pf_digitalWrite(cell_t pin, cell_t level);
static cell_t pf_digitalRead(cell_t pin);
static void pf_analogWriteResolution(cell_t bits);
static void pf_analogWrite(cell_t pin, cell_t level);
static void pf_analogReadResolution(cell_t bits);
static cell_t pf_analogRead(cell_t pin);
static void pf_analogReference(cell_t r);

static cell_t pf_millis() {
    return millis();
}

static cell_t pf_micros() {
    return micros();
}

static void pf_delay(cell_t ms) {
  delay(ms);
}

static void pf_delayMicroseconds(cell_t us) {
  delayMicroseconds(us);
}

static void pf_pinMode(cell_t pin, cell_t mode) {
  pinMode(pin, mode);
}

static void pf_digitalWrite(cell_t pin, cell_t level) {
  digitalWrite(pin, level);
}

static cell_t pf_digitalRead(cell_t pin) {
  return digitalRead(pin);
}

static void pf_analogWriteResolution(cell_t bits) {
  analogWriteResolution(bits);
}

static void pf_analogWrite(cell_t pin, cell_t level) {
  analogWrite(pin, level);
}

static void pf_analogReadResolution(cell_t bits) {
  analogReadResolution(bits);
}

static cell_t pf_analogRead(cell_t pin) {
  return analogRead(pin);
}

static void pf_analogReference(cell_t r) {
  //analogReference(r);
}

// Note: order of these is significant
CFunc0 CustomFunctionTable[] =
{
  (CFunc0) pf_millis,
  (CFunc0) pf_micros,
  (CFunc0) pf_delay,
  (CFunc0) pf_delayMicroseconds,
  (CFunc0) pf_pinMode,
  (CFunc0) pf_digitalWrite,
  (CFunc0) pf_digitalRead,
  (CFunc0) pf_analogWriteResolution,
  (CFunc0) pf_analogWrite,
  (CFunc0) pf_analogReadResolution,
  (CFunc0) pf_analogRead,
  (CFunc0) pf_analogReference
};

#else

CFunc0 CustomFunctionTable[12] = { nullptr };

#endif

Err CompileCustomFunctions( void ) { return 0; }

} // extern "C"
