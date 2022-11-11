# SOCKET TO STRING
characters from `SOCKET_MSG` to `STRING`
```
	{'\x01', '\n'}, // LINE_START
	{'\x02', '('},  // OPEN_LEN
	{'\x03', ')'},  // CLOSE_LEN
	{'\x04', '{'},  // VALUE_START
	{'\x05', '}'},  // VALUE_STOP
	{'\x06', '['},  // VALUE_OPEN
	{'\x07', ']'},  // VALUE_CLOSE
	{'\x08', ','},  // VALUE_SEPARATOR
	{'\x09', '='},  // MSG TYPE
```

# STRING TO SOCKET
characters from `STRING` to `SOCKET_MSG`
```
	{'\n', '\x01'}, // LINE_START
	{'(', '\x02'},  // OPEN_LEN
	{')', '\x03'},  // CLOSE_LEN
	{'{', '\x04'},  // VALUE_START
	{'}', '\x05'},  // VALUE_STOP
	{'[', '\x06'},  // VALUE_OPEN
	{']', '\x07'},  // VALUE_CLOSE
	{',', '\x08'},  // VALUE_SEPARATOR
	{'=', '\x09'}   // MSG TYPE
```

# FORMAT
## RICHIESTA
```
{=s<msg_type>
...}
```
## RISPOSTA
```
{=r<msg_type>
...}
```

# ESEMPI
un solo campo
```
{=sMSG_INFO
msg(1){[<msg>]}}
```

un array
```
{=sMSG_INFO
array(3){[1],[2],[3]}}
```

una matrice
```
{=sMSG_INFO
matrix(3,3){[[1],[2],[3]],[[4],[5],[6]],[[7],[8],[9]]}}
```

un messaggio composto da piu campi
```
{=sMSG_INFO
msg(1){[<msg>]}
array(3){[1],[2],[3]}
matrix(3,3){[[1],[2],[3]],[[4],[5],[6]],[[7],[8],[9]]}}
```

# MESSAGGI
## SCAMBIO DATI (CLIENT)
### MSG_GET_OWN_ID
Format:
```
{=sMSG_GET_OWN_ID}
```
Response:
```
{=rMSG_GET_OWN_ID
id(1){[<own_id>]}}
```
### MSG_GET_PLAYER_LIST
Format:
```
{=sMSG_GET_PLAYER_LIST}
```
Response:
```
{=rMSG_GET_PLAYER_LIST
players(<num>,2){[[<player_name>],[<player_id>]],[[<player_name>],[<player_id>]],[<player_name>],[<player_id>]}}
```
### MSG_GET_PLAYER_INFO
Format:
```
{=sMSG_GET_PLAYER_INFO
id(1){[<player_id>]}}
```
Response:
```
{=rMSG_GET_PLAYER_INFO
id(1){[<player_id>]}
<stat_1>(1){[<data>]}
<stat_2>(1){[<data>]}
<stat_3>(1){[<data>]}
<stat_4>(1){[<data>]}
...}
```
### MSG_GET_PLAYER_BOARD
Format:
```
{=sMSG_GET_PLAYER_BOARD
id(1){[<player_id>]}}
```
Response:
```
{=rMSG_GET_PLAYER_BOARD
id(1){[<player_id>]}
board(10,10){[[...],[...],...],[[...],[...],...],...}}
```
### MSG_SHIPS_PLACEMENT
Format:
```
{=sMSG_SHIPS_PLACEMENT
id(1){[<player_id>]}
<ship_enum>(2){[<ship_x>],[<ship_y>]}
<ship_enum>(2){[<ship_x>],[<ship_y>]}
<ship_enum>(2){[<ship_x>],[<ship_y>]}
<ship_enum>(2){[<ship_x>],[<ship_y>]}
<ship_enum>(2){[<ship_x>],[<ship_y>]}}
```
Response: `ack` | `nak`
field `msg` will be `ok` or `invalid`
### MSG_CAN_ATTACK
Format:
```
{=sMSG_CAN_ATTACK}
```
Response:
```
{=rMSG_CAN_ATTACK
can_attack(1){[<status>]}}
```
field `status` will be `1` (true) or `0` (false)
### MSG_ATTACK_PLAYER
Format:
```
{=sMSG_ATTACK_PLAYER
attacker(1){[<player_id>]}
defender(1){[<player_id>]}
x(1){[<board_x>]}
y(1){[<board_y>]}}
```
Response:
```
{=rMSG_ATTACK_PLAYER
id(1){[<player_id>]}
status(1){[<status>]}
```
ID value: `attacker`
Status values: `hit` | `not_hit` | `sunk`
### MSG_FORFEIT
Format:
```
{=sMSG_FORFEIT}
```
Response: `ack` | `nak`
## SCAMBIO DATI (SERVER)
### MSG_MATCH_START
Format:
```
{=sMSG_MATCH_START}
```
Response: `ack` | `nak`
### MSG_MATCH_END
Format:
```
{=sMSG_MATCH_END}
```
Response: `MSG_GET_PLAYER_INFO (<own_id>)`
### MSG_PLAYER_WIN
Format:
```
{=sMSG_PLAYER_WIN
id(1){[<winner_id>]}}
```
Response: `MSG_GET_PLAYER_INFO (<own_id>)`
### MSG_PLAYER_LOSE
Format:
```
{=sMSG_PLAYER_LOSE
id(1){[<loser_id>]}}
```
Response: `MSG_GET_PLAYER_INFO (<own_id>)`
## INFO
### MSG_INFO
Format:
```
{=sMSG_INFO
msg(1){[<message>]}}
```
Response: `ack` | `nak`
### MSG_CONN_ACCEPTED
Format:
```
{=sMSG_CONN_ERR
msg(1){[connection accepted]}}
```
Response: `ack` | `nak`
## ERRORI
### MSG_CONN_ERR
Format:
```
{=sMSG_CONN_ERR
msg(1){[connection error]}}
```
Response: `ack` | `nak`
### MSG_CONN_SERVER_FULL
Format:
```
{=sMSG_CONN_ERR
msg(1){[server is full]}}
```
Response: `ack` | `nak`
## RISPOSTE GENERICHE
### MSG_RSP_ACK
Format:
```
{=rMSG_RSP_ACK
to(1){[<msg_type>]}
msg(1){[<info>]}}
```
### MSG_RSP_NAK
Format:
```
{=rMSG_RSP_NAK
to(1){[<msg_type>]}
msg(1){[<info>]}}
```