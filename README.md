# ATMOSPHANTASTIC

A hobby project for personal use. Built with an ESP32-C3-Zero, BME680 sensor, scraps, and powered by 3x AAA batteries (I know, I know...).
Uses the Bosch BM68x sensor library to parse measurement data. Light sleep mode to conserve battery. BLE for syncing and broadcasting data.

Connects to a local rust backend which serves a simple dashboard.

This repo was made with next to no LLM help as it's a learning project, and the makeshift dashboard is 100% LLM because I've made enough of those by hand by now.

| | |
|---|---|
| <img width="422" height="316" alt="image" src="https://github.com/user-attachments/assets/d6839243-dac9-4010-8740-f53e78ea259f" /> | <img width="203" height="323" alt="atmosphantastic" src="https://github.com/user-attachments/assets/24642ff6-c4bc-4f45-ab09-9bfaf595982d" /> |

## Light sleep

The MCU spends most of its time in light sleep mode.
Measurements are made and announced once every 300 seconds.

## Sync

Holding the boot button puts the device in sync mode.
The onboard LED will blink until a connection has been established.
When connected, the client can set the MCU's time, and request measurement history.
The client must disconnect manually when done.

## Historical measurements

A rolling history of one week's hourly measurements is saved to flash memory.
History is only saved if a client has initialized the time via BLE, to ensure accurate timestamps.
The MCU has WiFi but I wanted to limit myself to BLE.

History can be requested by writing to a characteristic, whereupon it's transmitted in chunks.

## Limitations and improvements

There's no security so anyone can connect, read data, and update the time.
The AAA batteries in series is obviously not the best setup. I'd have liked to use a rechargable 3.7V li-ion battery.
It would be nice with a battery charge indicator.
It would also be neat to have a case for the electronics and skip the whole temporary breadboard setup.
The frontend and rust "server" is a mess, made only as proof of concept.
A little OLED screen to show the latest measurement would be neat.
