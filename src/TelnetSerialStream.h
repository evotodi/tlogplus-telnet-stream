#ifndef _H_TELNET_LOGTEE_PLUS
#define _H_TELNET_LOGTEE_PLUS

#include <TLogPlus.h>

#  if defined(ESP32)
#    include <WebServer.h>
#  elif defined(ESP8266)
#    include <ESP8266WebServer.h>
typedef ESP8266WebServer WebServer;
#  else
#    error "Must be ESP32 or ESP8266"
#  endif


#ifndef MAX_SERIAL_TELNET_CLIENTS
#  define MAX_SERIAL_TELNET_CLIENTS (4)
#endif
namespace TLogPlusStream {
    class TelnetSerialStream : public TLogPlus::TLog {
        typedef void (*CallbackFunction)(String str);

        typedef void (*IpCallbackFunction)(IPAddress ipAddress);

    public:
        TelnetSerialStream() {};
        TelnetSerialStream(const uint16_t telnetPort, const uint16_t maxClients) : _telnetPort(telnetPort), _maxClients(maxClients) {};
        TelnetSerialStream(bool logTelnetActions) : _logActions(logTelnetActions) {};
        TelnetSerialStream(const uint16_t telnetPort, const uint16_t maxClients, bool logTelnetActions)
            :
            _telnetPort(telnetPort),
            _maxClients(maxClients),
            _logActions(logTelnetActions) {};

        ~TelnetSerialStream();

        void onConnect(IpCallbackFunction f);

        void onDisconnect(IpCallbackFunction f);

        void onInputReceived(CallbackFunction f);

        bool isLineModeSet();

        void setLineMode(bool value = true);

        bool isLogActions();

        void setLogActions(bool value = true);

    private:
        uint16_t _telnetPort = 23;
        uint16_t _maxClients = MAX_SERIAL_TELNET_CLIENTS;
        WiFiServer *_server = NULL;
        WiFiClient **_serverClients;
        bool _logActions = false;

        IpCallbackFunction on_connect = NULL;
        IpCallbackFunction on_disconnect = NULL;
        CallbackFunction on_input = NULL;

        virtual size_t write(uint8_t c);

        virtual size_t write(uint8_t *buff, size_t len);

        virtual void begin();

        virtual void loop();

        virtual void stop();

        void _processClientConnection();

        void _handleInput(char c);

        void _handleClientInput();

    protected:
        String input = "";
        bool _lineMode = true;
    };
}
#endif