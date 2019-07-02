#include "ServerWeb.h"
WiFiServer server(80);
void webserver :: web_init()
{
        NVS.begin();
        Serial.println("Configuring access point...");
        // You can remove the password parameter if you want the AP to be open.
        WiFi.softAP(ssid, password);
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(myIP);
        server.begin();
        Serial.println("Server started");
}

void webserver :: web_server()
{
        WiFiClient client = server.available(); // listen for incoming clients

        if (client) {           // if you get a client,
                Serial.println("New Client."); // print a message out the serial port

                String currentLine = ""; // make a String to hold incoming data from the client
                while (client.connected()) { // loop while the client's connected
                        if (client.available()) { // if there's bytes to read from the client,
                                char c = client.read(); // read a byte, then
                                Serial.write(c); // print it out the serial monitor
                                if (c == '\n') { // if the byte is a newline character

                                        // if the current line is blank, you got two newline characters in a row.
                                        // that's the end of the client HTTP request, so send a response:
                                        if (currentLine.length() == 0) {
                                                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                                // and a content-type so the client knows what's coming, then a blank line:
                                                client.println("HTTP/1.1 200 OK");
                                                client.println("Content-type:text/html");
                                                client.println();

                                                client.println("<!DOCTYPE html><html>");
                                                client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                                                client.println("<link rel=\"icon\" href=\"data:,\">");
                                                client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                                                client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
                                                client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                                                client.println(".button2 {background-color: #555555;}</style></head>");

                                                // the content of the HTTP response follows the header:
                                                client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
                                                client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
                                                client.print("Click <a href=\"/J\">here</a> to turn OFF the LED.<br>");
                                                client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
                                                client.print(save_data);
                                                // The HTTP response ends with another blank line:
                                                client.println();
                                                // break out of the while loop:
                                                break;
                                        } else { // if you got a newline, then clear currentLine:
                                                currentLine = "";
                                        }
                                } else if (c != '\r') { // if you got anything else but a carriage return character,
                                        currentLine += c; // add it to the end of the currentLine
                                }

                                // Check to see if the client request was "GET /H" or "GET /L":
                                if (currentLine.endsWith("GET /H")) {
                                        digitalWrite(LED_BUILTIN, HIGH); // GET /H turns the LED on
                                        save_data = "Test_oK";
                                        test_int = 7;
                                        NVS.setInt("Sp_St", test_int);
                                        Serial.println(save_data);
                                }
                                if (currentLine.endsWith("GET /J")) {
                                        digitalWrite(LED_BUILTIN, HIGH); // GET /H turns the LED on
                                        save_data = "Test_oK_2";
                                        test_int = 8;
                                        NVS.setInt("Sp_Lt", test_int);
                                        Serial.println(save_data);
                                }
                                if (currentLine.endsWith("GET /K")) {
                                        digitalWrite(LED_BUILTIN, HIGH); // GET /H turns the LED on
                                        save_data = "Test_oK_3";
                                        test_int = 9;
                                        NVS.setInt("Am_car", test_int);
                                        Serial.println(save_data);
                                }

                                if (currentLine.endsWith("GET /L")) {
                                        save_data = "Test_faile_32";
                                        test_int = 10;
                                        digitalWrite(LED_BUILTIN, LOW); // GET /L turns the LED off
                                }
                        }
                }
                // close the connection:
                client.stop();
                Serial.println(save_data);
                Serial.println("Client Disconnected.");
        }
}
webserver web;
