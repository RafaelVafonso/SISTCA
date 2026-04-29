caso se use uma virtualização do genero wsl, de modo a aceder ao ficheiro de comunicação serial do esp32 é necessario:

no powershell:
usbipd list

Connected:
BUSID  VID:PID    DEVICE                                                        STATE
2-1    1a86:7523  USB-SERIAL CH340 (COM3)                                       Shared
2-6    1bcf:28cf  Integrated Webcam                                             Not shared
2-8    0a5c:5843  Dell ControlVault w/ Fingerprint Touch Sensor, Contacted ...  Not shared
2-10   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared

Persisted:
GUID                                  DEVICE

usbipd attach --wsl --busid 2-1

usbipd list
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
2-1    1a86:7523  USB-SERIAL CH340 (COM3)                                       Attached
2-6    1bcf:28cf  Integrated Webcam                                             Not shared
2-8    0a5c:5843  Dell ControlVault w/ Fingerprint Touch Sensor, Contacted ...  Not shared
2-10   8087:0026  Intel(R) Wireless Bluetooth(R)                                Not shared

Persisted:
GUID                                  DEVICE

assim o dispositivo é compartilhado pelo windows e pelo linux

1º compilar e dar upload ao codigo esp32

2º passos em cima

3º compilar e correr codigo pc