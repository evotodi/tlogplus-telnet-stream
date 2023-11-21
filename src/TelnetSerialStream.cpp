#include "TelnetSerialStream.h"
namespace TLogPlusStream {
    size_t TelnetSerialStream::write(uint8_t c) {
        if (!_server)
            return 1;
        for (int i = 0; i < _maxClients; i++) {
            if (_serverClients[i] && _serverClients[i]->connected()) {
                _serverClients[i]->write(c);
            };
        };
        return 1;
    }

    size_t TelnetSerialStream::write(uint8_t *c, size_t s) {
        if (!_server)
            return 1;
        for (int i = 0; i < _maxClients; i++) {
            if (_serverClients[i] && _serverClients[i]->connected()) {
                _serverClients[i]->write(c, s);
            };
        };
        return s;
    }

    TelnetSerialStream::~TelnetSerialStream() {
        stop();
        free(_serverClients);
    }

    void TelnetSerialStream::begin() {
        if (_server != NULL)
            return;
        _server = new WiFiServer(_telnetPort);
        _server->begin();
        _serverClients = (WiFiClient **) malloc(sizeof(WiFiClient *) * _maxClients);
        if (_serverClients == NULL) {
            _maxClients = 0;
            TLogPlus::Log.println("Disabling telnet logging -- insufficient memory.");
            stop();
        };
        for (int i = 0; i < _maxClients; i++)
            _serverClients[i] = NULL;
        if (_logActions) {
            TLogPlus::Log.printf("Opened serial server on telnet://%s:%d\n", WiFi.localIP().toString().c_str(), _telnetPort);
        }
    };

    void TelnetSerialStream::stop() {
        if (!_server)
            return;
        for (int i = 0; i < _maxClients; i++) {
            if (_serverClients[i]) {
                if (_logActions) {
                    _serverClients[i]->println("Connection closed from remote end");
                }
                if (on_disconnect != NULL) {
                    on_disconnect(_serverClients[i]->remoteIP());
                }
                _serverClients[i]->stop();
                delete _serverClients[i];
                _serverClients[i] = NULL;
            }
        }
        _server->stop();
        delete _server;
        _server = NULL;
    }

    void TelnetSerialStream::loop() {
        if (!_server)
            return;

        _processClientConnection();

        //check clients for data
        _handleClientInput();
    }

    void TelnetSerialStream::_processClientConnection() {
        if (_server->hasClient()) {
            int i;
            for (i = 0; i < _maxClients; i++) {
                if (!_serverClients[i] || !_serverClients[i]->connected()) {
                    if (_serverClients[i]) {
                        _serverClients[i]->stop();
                        delete _serverClients[i];
                        _serverClients[i] = NULL;
                    };

                    _serverClients[i] = new WiFiClient(_server->available());
                    if (_logActions) {
                        _serverClients[i]->print("Telnet connection");
                        if (identifier().length()) {
                            _serverClients[i]->print(" to ");
                            _serverClients[i]->print(identifier());
                        };
                        _serverClients[i]->println();
                    }
                    if (on_connect != NULL) {
                        on_connect(_serverClients[i]->remoteIP());
                    }
                    if (_logActions) {
                        TLogPlus::Log.print(_serverClients[i]->remoteIP());
                        TLogPlus::Log.print(":");
                        TLogPlus::Log.print(_serverClients[i]->remotePort());
                        TLogPlus::Log.println(" connected by telnet");
                    }
                    break;
                };
            };
            if (i >= _maxClients) {
                //no free/disconnected spot so reject
                if (_logActions) {
                    TLogPlus::Log.println("Too many log/telnet clients. rejecting.");
                }
                _server->available().stop();
            }
        }
    }

    void TelnetSerialStream::_handleClientInput() {
        for (int i = 0; i < _maxClients; i++) {
            if (!_serverClients[i])
                continue;
            if (!_serverClients[i]->connected()) {
                if (on_disconnect != NULL) {
                    on_disconnect(_serverClients[i]->remoteIP());
                }
                if (_logActions) {
                    TLogPlus::Log.print(_serverClients[i]->remoteIP());
                    TLogPlus::Log.print(":");
                    TLogPlus::Log.print(_serverClients[i]->remotePort());
                    TLogPlus::Log.println(" closed the connection.");
                }
                _serverClients[i]->stop();
                delete _serverClients[i];
                _serverClients[i] = NULL;
                continue;
            }

            if (!_serverClients[i]->available())
                continue;
            if (on_input) {
                while (_serverClients[i]->available()) {
                    char c = _serverClients[i]->read();
                    _handleInput(c);
                }
            } else {
                char c = _serverClients[i]->read();
                if (c > 0 && c < 32) {
                    if (_logActions) {
                        Serial.println("Ignoring telnet input");
                    }
                };
            }
        }
    }

    void TelnetSerialStream::_handleInput(char c) {

        // collect string
        if (_lineMode) {
            if (c != '\n') {
                if (c >= 32 && c < 127) {
                    input += c;
                }
                // EOL -> send input
            } else {
                on_input(input);
                input = "";
            }
            // send individual characters
        } else {
            if (input.length()) {
                on_input(input + String(c));
                input = "";
            } else {
                on_input(String(c));
            }
        }
    }

    bool TelnetSerialStream::isLineModeSet() {
        return _lineMode;
    }

    void TelnetSerialStream::setLineMode(bool value /* = true */) {
        _lineMode = value;
    }

    bool TelnetSerialStream::isLogActions() {
        return _logActions;
    }

    void TelnetSerialStream::setLogActions(bool value /* = true */) {
        _logActions = value;
    }

    void TelnetSerialStream::onInputReceived(CallbackFunction f) {
        on_input = f;
    }

    void TelnetSerialStream::onConnect(TelnetSerialStream::IpCallbackFunction f) {
        on_connect = f;
    }

    void TelnetSerialStream::onDisconnect(TelnetSerialStream::IpCallbackFunction f) {
        on_disconnect = f;
    }
}