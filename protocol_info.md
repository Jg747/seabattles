# PROTOCOL INFO
1. Si crea un match vuoto, questo crea il server
2. Connessione del client - man mano che i player si aggiungono scelgo i turni - il primo a connettersi è sempre l'host
3. Il client manda il proprio nome utente
4. Il server manda la configurazione del game (navi e board) al client + invia a tutti i clients le sprites delle navi
5. Il server manda la lista dei players ogni volta che si aggiorna la lista dei giocatori
6. L'host puo' kickare/bannare un client
7. L'host manda lo start del match al server + il server manda lo start del match ai clients
8. Ogni client manda la propria configurazione delle navi (ogni nave in ordine dal config: x, y, rot) e il server controlla la correttezza
9. Quando tutti i client sono ok allora il server manda il messaggio di start del game
10. Il server manda il messaggio del turno true al client con il turno e tutti gli altri il turno false
11. Per attaccare il client CON IL TURNO manda una richiesta della board del player che sta visualizzando e dopo un messaggio di attacco alle coordinate scelte, il server risponde confermando l'attacco oppure dando un errore (non ha il turno, coord invalide, i morti non attaccano)
12. Il server a seguito dell'attacco manda un messaggio al client attaccato con l'aggiornamento della board e chi lo ha attaccato
13. Ogni client puo' richiedere la board del client scelto
14. A match finito il Server manda ad ogni client le stats di tutti i clients ordinate per nome e il vincitore
15. Un client puo' essere eliminato
16. Un client può uscire dal gioco, prima di farlo inviera' un avviso al server che lo invierà a tutti i clients
17. Chat message
18. Il client puo' chiedere il proprio ID al server
19. Il server puo' inviare errori e messaggi di controllo

## 2 Connessione al server
### Risposta (Server)
Connessione avvenuta con successo
```json
{
	"type": "conn_success"
}
```

Server pieno 
```json
{
	"type": "conn_full"
}
```

Match in corso
```json
{
	"type": "conn_match_started"
}
```

Connessione fallita
```json
{
	"type": "conn_err",
	"msg": "{message}"
}
```

## 3 Invio del nome
### Invio (Client)
Invio del proprio nome utente
```json
{
	"type": "user_name",
	"msg": "{player_name}"
}
```

## 4 Configurazione del game
### Invio (Client)
Invio della configurazione della board, delle navi
```json
{
	"type": "config_host",
	"cfg": {
		{
			"gameDifficulty": {difficulty},
			"playerCount": {numberOfPlayers},
			"botsCount": {numberOfBots},
			"board": {
				"width": {width},
				"heigth": {heigth}
			},
			"ships": [
				{
					"id": {id},
					"length": {length},
					"width": {width},
					"sprite": {sprite_name}
				},
				// ...
			]
		}
	}
}
```
### Risposta (Server)
Il client e' host
```json
{
	"type": "config_host_accept"
}
```

Il client non e' host
```json
{
	"type": "match_not_host"
}
```

### Invio (Server)
Invio della configurazione a tutti i clients
```json
{
	"type": "config",
	"cfg": {
		{
			"gameDifficulty": {difficulty},
			"playerCount": {numberOfPlayers},
			"botsCount": {numberOfBots},
			"board": {
				"width": {width},
				"heigth": {heigth}
			},
			"ships": [
				{
					"id": {id},
					"length": {length},
					"width": {width},
					"sprite": {sprite_name}
				},
				// ...
			]
		}
	}
}
```

## 5 Invio lista dei giocatori
### Invio (Server)
Invio la lista dei giocatori
```json
{
	"type": "user_list",
	"list": [
		{
			"id": {id},
			"name": "{name}"
		},
		// ...
	]
}
```

## 6 Kick/ban
### Invio (Client)
Richiesta di kick
```json
{
	"type": "mod",
	"subtype": "kick",
	"id": {id}
}
```

Richiesta di ban
```json
{
	"type": "mod",
	"subtype": "ban",
	"id": {id}
}
```

### Risposta (Server)
Client e' host
```json
{
	"type": "mod_executed"
}
```

Client non e' host
```json
{
	"type": "match_not_host"
}
```

### Invio (Server)
Notifica di espulsione al Client (kick)
```json
{
	"type": "mod",
	"subtype": "kicked"
}
```

Notifica di espulsione al Client (ban)
```json
{
	"type": "mod",
	"subtype": "banned"
}
```

## 7 Invio dello start
### Invio (Client)
```json
{
	"type": "match_plcm_start"
}
```

### Risposta (Server)
Il client e' host
```json
{
	"type": "match_plcm_started"
}
```

Il client non e' host
```json
{
	"type": "match_not_host"
}
```

### Invio (Server)
Invio a tutti i clients connessi
```json
{
	"type": "match_plcm_started"
}
```

## 8 Invio della configurazione della propria board
### Invio (Client)
Invio della propria board al server
```json
{
	"type": "config_board",
	"ships": [
		{
			"id": {id},
			"x": {x},
			"y": {y},
			"r": {r}
		},
		// ...
	]
}
```

### Risposta (Server)
Configurazione ok
```json
{
	"type": "config_board_ok"
}
```

Configurazione non valida
```json
{
	"type": "config_board_err"
}
```

## 9 Invio del messaggio dello start del game in se'
### Invio (Server)
Game start
```json
{
	"type": "match_start"
}
```

## 10 Invio del turno
### Invio (Server)
Invio ad ogni client lo status del turno
```json
{
	"type": "turn",
	"who": {id}
}
```

## 11 Attacco
### Invio (Client)
Invio della richiesta della board
```json
{
	"type": "board_request",
	"id": {id},
	"debug": "null",
}
```

### Risposta (Server)
Invio della board del client richiesto
```json
{
	"type": "board",
	"board": [
		// unidimensional array with board values, BOARD_SHIP_FLAG replaced with BOARD_NOTHING
	]
}
```

Il client richiesto non e' disponibile
```json
{
	"type": "board",
	"board": []
}
```

### Invio (Client)
Invio delle coordinate di attacco
```json
{
	"type": "attack",
	"id": {id},
	"x": {x},
	"y": {y}
}
```

### Risposta (Server)
Il client ha il turno, attacco MISS, HIT, SUNK
```json
{
	"type": "attack_status",
	"subtype": "ok",
	"status": "{AttackStatus}"
}
```

Il client non ha il turno/ha il turno, INVLID, DEAD, ERROR
```json
{
	"type": "attack_status",
	"subtype": "error",
	"status": "{AttackStatus}"
}
```

## 12 Aggiornamento board del client attaccato
### Invio (Server)
A seguito di un attacco il client attaccato viene notificato con l'aggiornamento della propria board
```json
{
	"type": "got_attacked",
	"from": {id},
	"new_board": [
		// unidimensional array with board values
	]
}
```

## 13 Richiesta della board
### Invio (Client)
Il client non ha il cheat/debug attivo
```json
{
	"type": "board_request",
	"id": {id},
	"debug": "null"
}
```

Il client ha il cheat attivo
```json
{
	"type": "board_request",
	"id": {id},
	"debug": "{hardcoded_debug_password}"
}
```

### Risposta (Server)
Il client non ha il cheat/debug attivo
```json
{
	"type": "board",
	"board": [
		// unidimensional array with board values, BOARD_SHIP_FLAG replaced with BOARD_NOTHING
	]
}
```

Il client ha il cheat/debug attivo
```json
{
	"type": "board",
	"board": [
		// unidimensional array with board values
	]
}
```

Il client richiesto non e' disponibile
```json
{
	"type": "board",
	"board": []
}
```

## 14 Fine match
### Invio (Server)
Match terminato
```json
{
	"type": "match_end",
	"duration": {duration},
	"players": [
		{
			"id": {id},
			"name": {name},
			"stats": {
				"shots": {numberOfShots},
				"hits": {numberOfHits},
				"sunk": {numberOfSunkShips},
				"eliminations": {numberOfPlayerEliminations},
				"grade": "{grade}"
			}
		},
		// ...
	]
}
```

## 15 Eliminazione
### Invio (Server)
Il client e' appena stato eliminato da qualcuno
Il client non ha il turno
```json
{
	"type": "eliminated",
	"by": {id}
}
```

## 16 Uscita dal match
### Invio (Client)
Il client vuole uscire dal server, il server provvedera' a chiudere il Socket
```json
{
	"type": "quit",
}
```

### Invio (Server)
Invio a tutti i client rimasti che un giocatore e' uscito
```json
{
	"type": "left",
	"id": {id}
}
```

## 17 Chat message
### Invio (Client)
Invio al server un messaggio dalla chat
```json
{
	"type": "chat_send",
	"msg": "{text}"
}
```

### Invio (Server)
Invio a tutti i client il messaggio
Il client richiesto non e' disponibile
```json
{
	"type": "chat_recv",
	"from": {id},
	"msg": "{text}"
}
```

## 18 Richiesta ID
### Invio (Client)
Invio richiesta del proprio ID all'interno del server
```json
{
	"type": "id_request",
}
```

### Risposta (Server)
Risposta contenente l'ID del giocatore all'interno del server (serve per la sincronizzazione)
```json
{
	"type": "id_send",
	"id": {id}
}
```

## 19 Errori e messaggi di controlli
### Invio (Server)
Invio di un messaggio di errore generico
```json
{
	"type": "error",
	"msg": "{basic error explanation}"
}
```

### Invio (Server)
Invio del messaggio per avvisare i client che sta per trasmettere le sprites
```json
{
	"type": "sprites_send",
	"sprites": [
		{
			"id": {ship_id},
			"name": {fileName}
		}
	]
}
```