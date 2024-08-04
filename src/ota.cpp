#include "ota.h"
#include "ota_credentials.h"

WiFiClient ota_client;

bool initNetwork() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(OTA_SSID,OTA_PASS);
    int tries = 50;
    while (WiFi.status() != WL_CONNECTED && tries > 0) {
        delay(250);
        tries--;
    }
    if (tries == 0) {
        Serial.println("Failed to connect to WiFi!");
        return false;
    }
    Serial.print("IP: ");
    Serial.println(WiFi.localIP().toString());
    return true;
}

void initMDns() {
    if (!MDNS.begin(hostString)) {
      Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("mDNS responder started");
}

void checkUpdates() {  
    Serial.println("Sending mDNS query");
    int n = MDNS.queryService("espupdate", "tcp");
    Serial.println("mDNS query done");
    if (n == 0) {
      Serial.println("no update services found");
      return;
    }
    else {
      Serial.print(n);
      Serial.println(" service(s) found");
      for (int i = 0; i < n; ++i) {
        // Print details for each service found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(MDNS.hostname(i));
        Serial.print(" (");
        Serial.print(MDNS.IP(i).toString());
        Serial.print(":");
        Serial.print(MDNS.port(i));
        Serial.println(")");
      }
    }
    Serial.println();
    String url = "http://"+MDNS.IP(0).toString()+":"+MDNS.port(0)+"/espupdate?n=" SW_NAME "&v=" SW_VERSION;
    Serial.println("Checking firmware update from "+url);
    t_httpUpdate_return ret = httpUpdate.update(ota_client,url);
    switch (ret) {
      case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); break;
      case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;
      case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); break;
    }
}

void ota_setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.flush();
  if (initNetwork()) {
    initMDns();
    checkUpdates();
  };
}
