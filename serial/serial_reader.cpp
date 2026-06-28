#include "SimpleSerial.h"
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    char* com_port = (char*)"\\\\.\\COM7";
    DWORD baud_rate = CBR_9600;

    if (argc > 1) {
        com_port = argv[1];
    }
    if (argc > 2) {
        baud_rate = atoi(argv[2]);
    }

    SimpleSerial serial(com_port, baud_rate);

    if (!serial.connected_) {
        cerr << "Error: no se pudo abrir " << com_port << endl;
        return 1;
    }

    cout << "Conectado a " << com_port << " @ " << baud_rate << " baudios" << endl;
    cout << "Presiona Ctrl+C para salir..." << endl;

    while (true) {
        string data = serial.ReadSerialPort(1, "json");
        if (!data.empty()) {
            cout << "Recibido: " << data << endl;
        }
    }

    return 0;
}
