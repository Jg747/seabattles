# PROTOCOL INFO
- Si crea il server, crea un match vuoto
- Connessione del client - man mano che i player si aggiungono scelgo i turni - il primo a connettersi è sempre l'host | `ACK [MSG_CONN_ACCEPTED | MSG_CONN_ERR | MSG_CONN_SERVER_FULL | MSG_CONN_MATCH_STARTED]`
- Get id del client `[MSG_PLAYER_GET_OWN_ID (nome del player)]` | `ACK` contenente l'ID del player (assegno il nome del player ricevuto al corrispettivo Player*)
- Invio lista dei vari giocatori (per la GUI) ogni volta che varia la lista `[MSG_PLAYER_LIST (lista giocatori)]` | `ACK`
- il client host mi invia lo start match quando gli tira `[MSG_HOST_START_MATCH]` | `ACK` oppure `ACK [MSG_MATCH_NOT_HOST]`
- Quando il match è startato avviso tutti i giocatori che il match è startato (in locale ci pensa la GUI a far piazzare le navi) `[MSG_MATCH_STARTED]` | `ACK`
- Tutti i giocatori per cazzi loro in locale inseriscono le navi (una volta terminato l'inserimento si bloccano nel loro field e fine)
- Quando i giocatori confermano il loro piazzamento inviano le coordinate e la direzione delle loro navi in un messaggio `[MSG_PLAYER_SHIP_PLACEMENT (arr[5]=>[type, x, y, orient], ...)]` | `ACK` oppure `ACK [INVALID_SHIP_PLACEMENT]`
- Una volta che tutti hanno confermato invio a tutti `[MSG_MATCH_ALL_PLACED (your_turn | true o false in base di chi sia il turno)]` | `ACK`
- Il player attacca, prima c'è un `[MSG_PLAYER_GET_BOARD]`, poi un `[MSG_PLAYER_ATTACK (chi, x, y)]` | `ACK [MSG_MATCH_ATTACK_STATUS(attacco fallito, preso niente, colpito, affondato)]` oppure `ACK [MSG_MATCH_ATTACK_ERR(not turn, can't attack same player, dead_cant_attack)]`
   Invio al difensore `[MSG_MATCH_NEW_BOARD]` | `ACK`
- I vari player possono cercare di voler vedere i vari field nel mentre che attendono quindi scelta del giocatore da 
  vedere `[MSG_PLAYER_GET_BOARD (chi)]` | `ACK (dati [ogni cella nel messaggio conterrà solo l'ID del colore da mettere])`
- In caso un giocatore vinca invio `[MSG_MATCH_WIN (stats)]` | `ACK`
	invio a tutti `[MSG_MATCH_END (stats)]` | `ACK`
-  In caso un giocatore perda `[MSG_MATCH_LOSE (stats)]` | `ACK` - rimane li a vedere i vari field, si vedono sia le navi che dove l'altro è stato attaccato
- quando ha perso un giocatore e deve vedere un field invia `[MSG_PLAYER_GET_BOARD_LOST (chi)]` | `ACK (dati [ogni cella nel messaggio conterrà solo l'ID del colore da mettere])` oppure `ACK [MSG_MATCH_NOT_DEAD]`
- quando un giocatore viene rimosso dal GAME (non dal server) invio a tutti i giocatori `[MSG_MATCH_PLAYER_REMOVED (chi - motivazione - lista giocatori)]`
- In caso un giocatore quitti invia `[MSG_PLAYER_QUIT]` | `ACK [MSG_MATCH_END (stats)]`
- In caso un host voglia kickare qualcuno (menu / in game) invia `[MSG_HOST_PLAYER_KICK (chi - motivazione)]` | `ACK` oppure `ACK [MSG_MATCH_NOT_HOST]`
	- invio al kickato `[MSG_MATCH_GOT_KICKED (motivazione)]`
	- invio a tutti i giocatori `[MSG_MATCH_PLAYER_REMOVED (chi - motivazione - lista giocatori)]`

# FORMAT
## INVIO
```
<message>
	<type>msg_type</type>
	<data>
		<data_name>value</data_name>
		<data_name>
			<row>
				<col>data</col>
			</row>
			<row>
				<col>data</col>
			</row>
			<row>
				<col>data</col>
			</row>
		</data_name>
		<data_name>
			<row>
				<col>data</col>
				<col>data</col>
				<col>data</col>
			</row>
			<row>
				<col>data</col>
				<col>data</col>
				<col>data</col>
			</row>
			<row>
				<col>data</col>
				<col>data</col>
				<col>data</col>
			</row>
		</data_name>
	</data>
</message>
```
## RISPOSTA
```
<message>
	<type>ack</type>
	<data>
		<acktype>{ack type}</acktype>
		... {data} ...
	</data>
</message>

<message>
	<type>nak</type>
</message>
```

# MESSAGGI
## CONNESSIONE
### MSG_CONN_ACCEPTED
Caso in cui avvenga una connessione senza errori
Format:
```
<message>
	<type>ack</type>
	<data>
		<acktype>MSG_CONN_ACCEPTED</acktype>
	</data>
</message>
```
Risposta: `ACK`
### MSG_CONN_ERR
Caso in cui avvenga un errore di connessione
Format:
```
<message>
	<type>ack</type>
	<data>
		<acktype>MSG_CONN_ERR</acktype>
	</data>
</message>
```
### MSG_CONN_SERVER_FULL
Caso in cui il server abbia già più di `8 clients` connessi in pre-match
Format:
```
<message>
	<type>ack</type>
	<data>
		<acktype>MSG_CONN_SERVER_FULL</acktype>
	</data>
</message>
```
Risposta: `ACK`
### MSG_CONN_MATCH_STARTED
Caso in cui un giocatore provi a connettersi al server dopo che il match è iniziato
Format:
```
<message>
	<type>ack</type>
	<data>
		<acktype>MSG_CONN_MATCH_STARTED</acktype>
	</data>
</message>
```
Risposta: `ACK`

## MESSAGGI INVIATI DAI CLIENT
### MESSAGGI NORMALI
#### MSG_PLAYER_GET_OWN_ID
Inviato dal client dopo che si è connesso al server, contiene anche lo username, così che gli altri clients potranno ricevere la player list contenente lo username del giocatore (da usare nella GUI)
Format:
```
<message>
	<type>MSG_PLAYER_GET_OWN_ID</type>
	<data>
		<username>{player_username}</username>
	</data>
</message>
```
Risposta: `ACK[MSG_MATCH_PLAYER_ID]`
#### MSG_PLAYER_SHIP_PLACEMENT
Inviato dal client una volta confermato il piazzamento delle navi
Format:
```
<message>
	<type>MSG_PLAYER_SHIP_PLACEMENT</type>
	<data>
		<ship>
			<type>{int type}</type>
			<x>{x}</x>
			<y>{y}</y>
			<orientation>{int orientation}</orientation>
		</ship>
	</data>
</message>
```
`type` corrisponde a `enum ship_e`
`x` corrisponde alla X sul piano
`y` corrisponde alla Y sul piano
`orientation` corrisponde a `enum rotation_e`
Risposta:
- `ACK` in caso di accettazione
- `ACK[INVALID_SHIP_PLACEMENT]` in caso uno o più posizionamenti sono non corretti
#### MSG_PLAYER_ATTACK
Inviato dal player che ha il turno per indicare che ha attaccato delle specifiche coordinate di un player
Format:
```
<message>
	<type>MSG_PLAYER_ATTACK</type>
	<data>
		<id>{id}</id>
		<x>{x}</x>
		<y>{y}</y>
	</data>
</message>
```
`id` è l'id del difensore
`x` è la X in cui si è attaccato
`y` è la Y in cui si è attaccato
Risposta:
- `ACK[MSG_MATCH_ATTACK_STATUS]` in caso l'attacco sia o non sia andato a buon fine
- `ACK[MSG_MATCH_ATTACK_ERR]` in caso ci sia un errore nell'attacco
#### MSG_PLAYER_GET_BOARD
Inviato dal player che ha il turno quando ha bisogno di vedere il campo avversario
Format:
```
<message>
	<type>MSG_PLAYER_GET_BOARD</type>
	<data>
		<id>{id}</id>
	</data>
</message>
```
`id` è l'id del player da vedere
Risposta: `ACK[MSG_GET_BOARD]`
#### MSG_PLAYER_GET_BOARD_LOST
Inviato da uno spettatore per vedere il campo di un giocatore (navi e colpi visibili)
Format:
```
<message>
	<type>MSG_PLAYER_GET_BOARD_LOST</type>
	<data>
		<id>{id}</id>
	</data>
</message>
```
`id` è l'id del player da vedere
Risposta: `ACK[MSG_GET_BOARD_LOST]`
#### MSG_PLAYER_QUIT
Inviato da un client quando intende uscire dal server
- Se il player è in game allora il server invierà un `MSG_MATCH_PLAYER_REMOVED` ai clients
- Se il player è uno spettatore non succede nulla
Format:
```
<message>
	<type>MSG_PLAYER_QUIT</type>
</message>
```
Risposta: `ACK[MSG_MATCH_END]`
### MESSAGGI HOST
#### MSG_HOST_START_MATCH
Inviato dal player host per avviare la partita
Format:
```
<message>
	<type>MSG_HOST_START_MATCH</type>
</message>
```
Risposta:
- `ACK` in caso il client sia effettivamente l'host
- `ACK[MSG_MATCH_NOT_HOST]` in caso il client non sia host
#### MSG_HOST_PLAYER_KICK
Inviato dal player host per kickare dal server un player
Format:
```
<message>
	<type>MSG_HOST_PLAYER_KICK</type>
	<data>
		<id>{id}</id>
		<msg>{message}</msg>
	</data>
</message>
```
`id` corrisponde all'id del giocatore da kickare
`msg` corrisponde alla motivazione del kick
Risposta:
- `ACK` in caso il client sia effettivamente l'host
- `ACK[MSG_MATCH_NOT_HOST]` in caso il client non sia host

## RISPOSTE DEL SERVER AL CLIENT
### ACK
ACK generico, serve solo di conferma
Format:
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>ack</acktype>
	</data>
</message>
```
`id` è l'id del giocatore
### ACK[MSG_MATCH_PLAYER_ID]
Inviato al client dopo aver ricevuto `MSG_PLAYER_GET_OWN_ID`
Format:
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_PLAYER_ID</acktype>
	</data>
</message>
```
`id` è l'id del giocatore
### ACK[MSG_MATCH_NOT_HOST]
Inviato al client dopo aver tentato un comando da host, ma questo non lo è
Format:
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_NOT_HOST</acktype>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
### ACK[INVALID_SHIP_PLACEMENT]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_SHIP_PLACEMENT` contenente posizioni invalide
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>INVALID_SHIP_PLACEMENT</acktype>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
### ACK[MSG_MATCH_ATTACK_STATUS]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_ATTACK` valido
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_ATTACK_STATUS</acktype>
		<status>{status}</status>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
`status` può essere
- `FAILED_ATTACK` in caso l'attacco sia effettuato su una posizione già precedentemente colpita
- `MISSED` in caso non si abbia preso nulla
- `HIT` in caso si abbia colpito una nave
- `HIT_SUNK` in caso si abbia colpito e affondato una nave
### ACK[MSG_MATCH_ATTACK_ERR]
Inviato al client dopo aver ricevuto un `MSG_MATCH_ATTACK_ERR` non valido
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_ATTACK_ERR</acktype>
		<error>{error}</error>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
`error` pul essere
- `NOT_YOUR_TURN` in caso non sia il proprio turno
- `NOT_SAME_PLAYER` in caso si abbia già attaccato quel player nello stesso turno
- `DEAD_CANNOT_ATTACK` in caso uno spettatore tenti di attaccare
- `INVALID_ATTACK` in caso si forniscano coordinate invalide
### ACK[MSG_GET_BOARD]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_GET_BOARD`
Format:
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<player>{id}</player>
		<acktype>MSG_GET_BOARD</acktype>
		<board>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			...
		</board>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
`player` è l'id del player che si sta vedendo
`color` corrisponde al codice di colore da dare alla GUI del client
### ACK[MSG_GET_BOARD_LOST]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_GET_BOARD_LOST`, è uguale a `MSG_PLAYER_GET_BOARD` ma sono in chiaro le posizioni delle navi e dove è stato colpito (è come un `MSG_MATCH_NEW_BOARD` ma rivolto ad uno spettatore)
Format:
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<player>{id}</player>
		<acktype>MSG_GET_BOARD_LOST</acktype>
		<board>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			...
		</board>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
`player` è l'id del player che si sta vedendo
`color` corrisponde al codice di colore da dare alla GUI del client
### ACK[MSG_MATCH_NOT_DEAD]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_GET_BOARD_LOST` anche se il player non è uno spettatore (morto)
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_NOT_DEAD</acktype>
	</data>
</message>
```
`id` è l'id del giocatore a cui è rivolto l'ack (da usare per controllo)
### ACK[MSG_MATCH_END]
Inviato al client dopo aver ricevuto un `MSG_PLAYER_QUIT`
```
<message>
	<type>ack</type>
	<data>
		<id>{id}</id>
		<acktype>MSG_MATCH_END</acktype>
		<grade>{grade}</grade>
		<hits>{hits}</hits>
		<misses>{misses}</misses>
		<ships>
			<sunk>{sunk}</sunk>
			<remaining>{remaining}</remaining>
		</ships>
	</data>
</message>
```
`id` è l'id del giocatore a cui si sta inviando il dato
`grade` è il grado del giocatore alla fine della partita
`hits` è il numero di di hit effettuate
`misses` è il numero di miss effettuati
`sunk` è il numero di navi affondate da lui
`remaining` è il numero delle proprie navi rimaste

## MESSAGGI INVIATI DAL SERVER
### MSG_PLAYER_LIST
Inviato dal server ogni volta che si aggiunge/toglie un giocatore dal match (spettatori esclusi)
Format:
```
<message>
	<type>MSG_PLAYER_LIST</type>
	<data>
		<playerlist>
			<player>
				<id>{id}</id>
				<username>{username}</username>
			</player>
			<player>
				<id>{id}</id>
				<username>{username}</username>
			</player>
			...
		</playerlist>
	</data>
</message>
```
`id` è l'ID del giocatore, è affiancato a `username` che è l'username fornito
Risposta: `ACK`
### MSG_MATCH_STARTED
Inviato dal server quando è stato ricevuto MSG_HOST_START_MATCH
Format:
```
<message>
	<type>MSG_MATCH_STARTED</type>
</message>
```
Risposta: `ACK`
### MSG_MATCH_ALL_PLACED
Inviato dal server quando è stato ricevuto MSG_PLAYER_SHIP_PLACEMENT (corretto) da tutti i giocatori
Format:
```
<message>
	<type>MSG_MATCH_ALL_PLACED</type>
	<data>
		<turn>{boolean your_turn}</turn>
	</data>
</message>
```
`turn` è un booleano che sarà true per chi deve iniziare il turno e false per chi deve attendere il proprio turno
Risposta: `ACK`
### MSG_MATCH_NEW_BOARD
Inviato dal server al client quando cambia la situazione nella board di quel player
Format:
```
<message>
	<type>MSG_MATCH_GET_BOARD</type>
	<data>
		<id>{id}</id>
		<attacker>{username}</attacker>
		<board>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			<row>
				<color>{color}</cololor>
				<color>{color}</cololor>
				<color>{color}</cololor>
				...
			</row>
			...
		</board>
	</data>
</message>
```
`id` è l'id del giocatore a cui si sta inviando il dato
`attacker` è lo username del giocatore che ha attaccato il client
`color` corrisponde al codice di colore da dare alla GUI del client
Risposta: `ACK`
### MSG_MATCH_WIN
Inviato dal server al giocatore che ha vinto quando un match è terminato
Format:
```
<message>
	<type>MSG_MATCH_WIN</type>
	<data>
		<id>{id}</id>
		<grade>{grade}</grade>
		<hits>{hits}</hits>
		<misses>{misses}</misses>
		<ships>
			<sunk>{sunk}</sunk>
			<remaining>{remaining}</remaining>
		</ships>
	</data>
</message>
```
`id` è l'id del giocatore a cui si sta inviando il dato
`grade` è il grado del giocatore alla fine della partita
`hits` è il numero di di hit effettuate
`misses` è il numero di miss effettuati
`sunk` è il numero di navi affondate da lui
`remaining` è il numero delle proprie navi rimaste
Risposta: `ACK`
### MSG_MATCH_LOSE
Inviato dal server al giocatore che ha perso (in caso sia l'ultimo giocatore corrisponderà anche ad un MSG_MATCH_END), in alternativa, una volta cambiata schermata, potrà rimanere nel server come spettatore
Format:
```
<message>
	<type>MSG_MATCH_LOSE</type>
	<data>
		<id>{id}</id>
		<grade>{grade}</grade>
		<hits>{hits}</hits>
		<misses>{misses}</misses>
		<ships>
			<sunk>{sunk}</sunk>
			<remaining>{remaining}</remaining>
		</ships>
	</data>
</message>
```
`id` è l'id del giocatore a cui si sta inviando il dato
`grade` è il grado del giocatore alla fine della partita
`hits` è il numero di di hit effettuate
`misses` è il numero di miss effettuati
`sunk` è il numero di navi affondate da lui
`remaining` è il numero delle proprie navi rimaste
Risposta: `ACK`
### MSG_MATCH_END
Inviato dal server a tutti gli spettatori del server quando è terminato il match
Format:
```
<message>
	<type>MSG_MATCH_END</type>
	<data>
		<id>{id}</id>
		<grade>{grade}</grade>
		<hits>{hits}</hits>
		<misses>{misses}</misses>
		<ships>
			<sunk>{sunk}</sunk>
			<remaining>{remaining}</remaining>
		</ships>
	</data>
</message>
```
`id` è l'id del giocatore a cui si sta inviando il dato
`grade` è il grado del giocatore alla fine della partita
`hits` è il numero di di hit effettuate
`misses` è il numero di miss effettuati
`sunk` è il numero di navi affondate da lui
`remaining` è il numero delle proprie navi rimaste
Risposta: `ACK`
### MSG_MATCH_PLAYER_REMOVED
Inviato dal server quando c'è una variazione della player list (player ha perso/è stato kickato) a TUTTI i client connessi
Format:
```
<message>
	<type>MSG_MATCH_PLAYER_REMOVED</type>
	<data>
		<who>{username}</who>
		<reason>{reason}</reason>
		<playerlist>
			<player>
				<id>{id}</id>
				<username>{username}</username>
			</player>
			<player>
				<id>{id}</id>
				<username>{username}</username>
			</player>
			...
		</playerlist>
	</data>
</message>
```
`who` contiene lo username del player rimosso
`reason` è la motivazione della rimozione e può essere:
- `LOST` in caso sia rimosso perché ha perso la partita (tutte le sue navi sono state affondate)
- `QUIT` in caso un player non spettatore quittasse
- `Kicked: {message}` in caso sia stato kickato dall'host
`playerlist` è la nuova player list *senza* il player rimosso `id` è l'ID del giocatore, è affiancato a `username` che è l'username fornito
Risposta: `ACK`
### MSG_MATCH_GOT_KICKED
Inviato dal server al client che è stato kickato (una volta ricevuto quello procederà con la disconnessione "forzata")
Format:
```
<message>
	<type>MSG_MATCH_GOT_KICKED</type>
	<data>
		<reason>{message}</reason>
	</data>
</message>
```
`reason` corrisponde al messaggio del kick (scritto dall'host quando intende kickare il client)
Risposta: `ACK`