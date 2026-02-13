# Verisure-Packet-Sniffer
A frame sniffer for Verisure ES700MGLS opening detectors and PIRs

Need a ESP32 Dev Module and a cc1101 rf transceiver module

Pin connections
CC1101 model E07-M1101D      ESP32
      1-GND                  GND
      2-VCC                  3V3
      3-GDO0                 D4
      4-CSN                  D5
      5-SCK                  D18
      6-MOSI                 D23
      7-MISO                 D19
      8-GDO2                 --

<img width="1050" height="609" alt="esp32_verisure_pkt_sniffer fzpz_bb" src="https://github.com/user-attachments/assets/827fa6d4-72aa-48cb-bbd2-dc0e91708c11" />

The data is displayed on the Serial Monitor output of the Arduino IDE (speed 115200 bauds)

<img width="919" height="246" alt="Capture d’écran du 2026-02-13 09-35-19" src="https://github.com/user-attachments/assets/4dc81fa1-92b8-453e-b7ba-df0461ecc5ec" />

<img width="503" height="170" alt="Capture d’écran du 2026-02-13 09-33-25" src="https://github.com/user-attachments/assets/fd18f8c1-5fbb-48d3-aa2c-57b763b0c7d6" />
