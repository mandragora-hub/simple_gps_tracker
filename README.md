# Simple gps tracker

Chip: T-A7670SA R2 With GPS [Q335]

Lilygo documentation: https://wiki.lilygo.cc/products/t-sim-series/t-a7670/t-a7670sa/
SIMCom AT command references: (https://github.com/Xinyuan-LilyGO/LilyGo-Modem-Series/blob/main/datasheet/SIM767X/SIM767XX%20Series_AT_Command_Manual_V1.06.pdf)

Interesting AT commands:

* ATI show manufactur information
* AT+CRESET reset the module
* AT+CGMR Request revision identification
* AT+CPIN=? check sim is ready
* AT+CPING ping destination address
* AT+CEREG? EPS network registration status
* AT+CGATT? Packet domain attach or detach
* AT+COPS? Operator selection


## Actions

- Send SMS messages

```bash
AT+CPIN? # check SIM is ready

AT+CSCS="GSM"
AT+CSCA? # check sms services center

AT+CMGF=1
AT+CMGS="+18298086111"
# then Ctrl+Z or ESC
```

- Enter to CUSD menu

```bash
AT+CSCS="GSM" # select TE characters
AT+CUSD=? # verify CUSD is valid
AT+CUSD=1,"*111#",15

AT+CUSD=1,"2",15 # select option 2
AT+CUSD=2 # cancel
```

- Http request (get metheod)

```bash
AT+HTTPINIT
AT+HTTPPARA="URL","http://example.com"
AT+HTTPACTION=0 
AT+HTTPREAD
```

- Ping

```bash
AT+CPING="www.google.com",1,4,64,1000,10000,255
```

### Goals

- When it is being powered via USB, connect to wifi and deploy a API and a SPA web page. When is using the battery, try to save battery consumption
- Implement deep sleep
- Store in a sd card memory logs, and gps entries, and everthing else, and show up in the web page.
- 1 minute poll for the gps location, if location is the same as the las one does not send to the server.
- Implement basic sms commands, in the format [command][password]
    - check1234 status
    - smslink1234 return a url with the current location

