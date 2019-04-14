#pragma once

#include <Arduino.h>
#include <WiFiClient.h>

namespace Hueduino {
    namespace Internals {
        class WiFiClientWrapper : public Stream {
            public:
                WiFiClientWrapper(WiFiClient& client) : mClient(client) {}

                virtual int available() { 
                    return mClient.available();
                }

                virtual int read() {
                    int c = 0;
                    mClient.readBytes((uint8_t*)&c, 1); // Use ::readBytes because it uses the internal timeout
                    return c;
                }

                virtual int peek() {
                    int c = 0;                    
                    mClient.peekBytes((uint8_t*)&c, 1); // Use ::peekBytes because it uses the internal timeout
                    return c;
                }

                virtual size_t write(uint8_t byte) {
                    return mClient.write(byte);
                }
            private:
                WiFiClient& mClient;
        };
    } // Internals
} // Hueduino