#include "ServerManager.h"

ServerManager serverManager;

void setup() {
  Serial.begin(9600);
  // Initialize the output variables as outputs
  serverManager.ConnectToWiFi("Quevedo's Family", "42393445");
  serverManager.CreateServer();
}

void loop(){

}
