# Acionna

Acionna is a firmware implementation to control and control and monitoring loads (machine, contactors, magnetic switch, etc). It can monitoring variables readed by sensors. The connection is over WiFi and based in websocket protocol.

# The system works above layers of control


# Features
	- intake air detect;

# TODO list:
- FSM valves needs works jointly with FSM machine control;
- enhance low pressure dectection:
	- broke main pipe;
    - while running some valve sector, identifies if has some broken pipe;
    - prevent occurs when change valve sector.

*Acionna
- Add eeprom functionality to store automatic turn on time/mode list;

*Valves/Pipe
- average pressure to specific valve sector to log history

- There is a problem when turn irrigation system off because valves turn off before pump. Pipe line stay pressure and when occurs a new departure the system notice high pressure.