#include <Arduino.h>
#include <WiFi.h>	  //default wifi library for uno R4
#include <WiFiUdp.h>  //https://docs.arduino.cc/language-reference/en/functions/wifi/udp/ --> use to send and received udp messages over wifi
#include "WString.h"

enum PacketType { Handshake, Command, State };

enum WifiControllerType { Client, Server };

template <typename DataPacket>
class WifiController {
	protected:
		WiFiUDP udp;

		WifiControllerType dir;

		const unsigned int server_port
			= 8888;	 // UDP port to listen on --> port for Rx
		const unsigned int client_port = 8889;	// Arbitrary local port for TX

		const String clientHostname = "rover\n";
		const String serverHostname = "remote\n";

		char* clientIP = "192.168.4.2";
		char* serverIP = "192.168.4.1";

		DataPacket outboundData;
		bool outboundNew = false;
		DataPacket inboundData;
		bool inboundNew = false;

		class handshake;
		handshake handshake = {this};

		unsigned long lastConnectionAttempt;

		String ssid;
		String password;

		void clientConnectToWiFi() {
			Serial.print("Client: Connecting to AP ");
			Serial.print(ssid);
			Serial.print(" with hostname ");
			Serial.println(clientHostname);

			WiFi.begin(ssid.c_str(), password.c_str());
			unsigned long startAttemptTime = millis();
			while (WiFi.status() != WL_CONNECTED
				   && millis() - startAttemptTime < 15000) {
				// Attempts to connect to wifi for 15s before restarting
				// connection attempt if failed | if successful terminates
				// immediately
				delay(500);
				Serial.print(".");
			}
			if (WiFi.status() == WL_CONNECTED) {
				udp.begin(client_port);	 // initializes Tx port
				Serial.println("\nClient: Connected!");
				Serial.print("Client: IP Address: ");
				Serial.println(WiFi.localIP());
			} else {
				Serial.println("\nClient: Failed to connect!");
			}
		}

	public:
		WifiController()  = default;
		~WifiController() = default;

		void begin(String ssid, String password, WifiControllerType dir) {
			pinMode(13, OUTPUT);  // built-in LED --> visual of connection
								  // status between client and server
			if (password.length() < 8) {
				while (true) {
					Serial.println("Password: <" + password + "> is too short, please increase to 8 characters minimum.");
					delay(1000);
				}
			}

			this->ssid	   = ssid;
			this->password = password;
			this->dir	   = dir;

			handshake.client = dir == WifiControllerType::Client;

			if (dir == WifiControllerType::Server) {
				Serial.println("Server: Starting up...");

				WiFi.setHostname(serverHostname.c_str());

				// Start the board as a Soft Access Point (SoftAP), i.e., create
				// its own hot spot
				// this allows the device, specifically the Server, to act as a
				// temporary wirelss network that other devices can connect to
				if (WiFi.beginAP(ssid.c_str(), password.c_str())) {
					// if the ssid and the password exist for our soft access
					// point
					Serial.println("Server: SoftAP started successfully.");
					Serial.print("Server: AP IP: ");
					Serial.println(WiFi.softAPIP());

					// display AP information
				} else {
					while (true) {
						Serial.println("Server: Failed to start AP!");
						delay(1000);
					}
				}
				udp.begin(server_port);
			} else {
				// DONT USE PIN13 FOR ANY SENSOR OR ACTUATORS --> visual
				// connection
				WiFi.setHostname(clientHostname.c_str());
				Serial.println("Client: Starting up...");
				clientConnectToWiFi();
			}
		}

		void tick() {
			switch (dir) {
				case WifiControllerType::Server:
					{
						handshake.tickServer();
						break;
					}
				case WifiControllerType::Client:
					{
						handshake.tickClient();
						break;
					}
			}

			if (!handshake.handshooked && dir == WifiControllerType::Client
				&& millis() - lastConnectionAttempt
					   > handshake.MAX_HANDSHAKE_TIME * 2) {
				Serial.println("Handshake Failed.");
				if (WiFi.status() != WL_CONNECTED) {
					Serial.println(
						"Client: WiFi disconnected! Reconnecting...");
					clientConnectToWiFi();
				} else {
					Serial.println("Client: WiFi OK.");
				}
				lastConnectionAttempt = millis();
			}

			if (outboundNew && handshake.handshooked) {
				outboundNew = false;
				sendData(outboundData);
			}
			readData();
		}

		bool available() { return inboundNew; }

		bool active() { return handshake.handshooked; }

		DataPacket getData() {
			inboundNew = false;
			return inboundData;
		}

		void setData(const DataPacket& data) {
			outboundData = data;
			outboundNew	 = true;
		}

	protected:
		bool sendData(const DataPacket& data) {
			switch (dir) {
				case WifiControllerType::Server:
					{
						udp.beginPacket(clientIP, client_port);
						break;
					}
				case WifiControllerType::Client:
					{
						udp.beginPacket(serverIP, server_port);
						break;
					}
			}
			udp.write((uint8_t*)&data, sizeof(DataPacket));
			return udp.endPacket();
		}

		void readData() {
			int packetSize
				= udp.parsePacket();  // parse out the packet size transmitted,
									  // i.e., data Packet Struct size
			if (packetSize >= sizeof(
					DataPacket)) {	// if sizes match --> we have a transmission
				// within this if statement is the ONLY place we want to do
				// stuff!!!
				udp.read((uint8_t*)&inboundData,
						 sizeof(DataPacket));  // unpack everything in the data
											   // DataPacket

				if (inboundData.type == PacketType::Handshake) {
					handshake.received();
				} else {
					inboundNew = true;
				}
			}
		}

		struct handshake {
				WifiController* controller;
				bool client;
				const unsigned long MAX_HANDSHAKE_TIME = 5000;
				bool handshooked   = false;	 // True on handshake receive
				bool sendHandshake = false;	 // Did we send a request
				unsigned long lastHandshakeTime
					= 0;  // Last time a handshake succeded
				unsigned long handshakeSendTime
					= 0;  // When we sent the handshake packet

				DataPacket handshakePacket = {PacketType::Handshake};

				void send() {
					handshakeSendTime = millis();
					sendHandshake	  = true;
					Serial.println("Server: Sent handshake");
					controller->sendData(handshakePacket);
				}

				void received() {
					lastHandshakeTime = millis();
					handshooked		  = true;
					sendHandshake	  = false;
					Serial.println("Server: Received handshake");
					if (!client) send();
				}

				void tickServer() {
					if (millis() - lastHandshakeTime
						> MAX_HANDSHAKE_TIME + 300) {
						handshooked = false;
					}
				}

				void tickClient() {
					bool time
						= millis() - lastHandshakeTime < MAX_HANDSHAKE_TIME;
					if (time) { return; }
					if (!sendHandshake) {
						send();
						return;
					}
					if (millis() - handshakeSendTime > 500) {
						handshooked = false;
						send();
					}
				}
		};
};
