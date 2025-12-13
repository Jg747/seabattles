package org.seabattles.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.text.ParseException;
import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import org.json.JSONObject;
import org.seabattles.net.Protocol.MsgType;
import org.seabattles.net.Server.ConnStatus;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.Game.GenericStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Player;
import org.seabattles.src.Stats;

public class Client implements Runnable {
	
	private Socket sock;
	private InputStream in;
	private PrintWriter out;
	
	private boolean interrupted;
	
	private ConnStatus connStatus;
	private String errorMsg;
	
	private Game game;
	
	public Client(String ipAddress, Game game) {
		try {
			sock = new Socket(ipAddress, Server.SERVER_PORT);
			in = sock.getInputStream();
			out = new PrintWriter(sock.getOutputStream());
			
			if (!connOk(waitMsg().get())) {
				this.destroy();
			}
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println(e);
		}
		
		this.game = game;
		interrupted = false;
	}
	
	// GUI INTERFACE
	private ConnStatus getConnStatus() {
		return connStatus;
	}
	
	// GUI INTERFACE
	private String getErrorMsg() {
		return errorMsg;
	}
	
	private boolean connOk(JSONObject msg) {
		Map<MsgType, String> status = Protocol.parseConnectionMessage(msg);
		if (((MsgType[])status.keySet().toArray())[0] == MsgType.CONN_SUCCESS) {
			connStatus = Server.ConnStatus.STATUS_OK;
			return true;
		}
		
		if (((MsgType[])status.keySet().toArray())[0] == MsgType.CONN_FULL) {
			connStatus = Server.ConnStatus.STATUS_FULL;
		}
		
		if (((MsgType[])status.keySet().toArray())[0] == MsgType.CONN_MATCH_STARTED) {
			connStatus = Server.ConnStatus.STATUS_STARTED;
		}
		
		if (((MsgType[])status.keySet().toArray())[0] == MsgType.CONN_ERR) {
			connStatus = Server.ConnStatus.STATUS_ERR;
			errorMsg = status.get(((MsgType[])status.keySet().toArray())[0]);
		}
		
		return false;
	}
	
	public void destroy() {
		try {
			interrupted = true;
			in.close();
			out.close();
			sock.close();
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println(e);
		}
	}

	public Optional<JSONObject> waitMsg() {
		try {
			// TODO InputStream.readAllBytes() does NOT differentiate between data from multiple sources (problemi nel caso speciale sprites_send)
			JSONObject ret = new JSONObject(new String(in.readAllBytes()));
			return Optional.of(ret);
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println(e);
			destroy();
		}
		return Optional.empty();
	}
	
	public void sendMsg(String msg) {
		out.println(msg);
	}
	
	public void sendMsg(JSONObject msg) {
		sendMsg(msg.toString());
	}
	
	public void sendMsg(Optional<JSONObject> msg) {
		if (msg.isPresent()) {
			sendMsg(msg.get().toString());
		}
	}

	@Override
	public void run() {
		while (!interrupted && !Thread.currentThread().isInterrupted()) {
			Optional<JSONObject> ret = waitMsg();
			if (ret.isPresent()) {
				parse(ret.get());
			}
		}
	}
	
	// GUI INTERFACE
	public Player getPlayer() {
		return game.getOwnPlayer();
	}
	
	private void parseGameConfig(JSONObject obj) throws ParseException {
		GameConfig cfg = Protocol.parseConfigurationMessage(obj);
		game.setGameConfig(cfg);
	}
	
	private void parseUserList(JSONObject obj) {
		game.setPlayerList(Protocol.parseUserListMessage(obj));
	}
	
	private void parseConnErrMsg(JSONObject msg) {
		game.setErrMsg(Protocol.parseConnErrMsg(msg));
	}
	
	private void parseModMessage(JSONObject msg) {
		game.setErrMsg((Boolean) Protocol.parseModMessage(msg)[1] ? "ban" : "kick" );
		this.destroy();
	}
	
	private void getSprites(JSONObject msg) {
		HashMap<Integer, String> sprites = Protocol.parseSpritesSendMessage(msg);
		
		// TODO download sprites
	}
	
	private void sendIDRequest() {
		sendMsg(Protocol.getIDRequestMessage());
	}
	
	private void parseIDSend(JSONObject msg) {
		game.setOwnID(Protocol.parseIDSendMessage(msg));
	}
	
	private void setTurn(JSONObject msg) {
		UUID who_turn = Protocol.parseTurnMessage(msg);
		game.setTurn(who_turn);
	}
	
	private void parseBoard(JSONObject msg) {
		byte[][] board = Protocol.parseBoardMessage(msg);
		if (board[0][0] == -1) {
			game.setErrMsg("Client not available");
		} else {
			game.setResource(board);
		}
	}
	
	private String parseAttackStatus(JSONObject msg) {
		return Board.statusStrings.get(Protocol.parseAttackStatusMessage(msg));
	}
	
	private void parseGotAttacked(JSONObject msg) {
		Object[] ret = Protocol.parseGotAttackedMessage(msg);
		
		UUID from = (UUID) ret[0];
		byte[][] board = (byte[][]) ret[1];
		
		game.getOwnPlayer().getBoard().setBoardMatrix(board);
		
		// TODO notify GUI
	}
	
	private void parseMatchEnd(JSONObject msg) {
		Object[] ret = Protocol.parseMatchEndMessage(msg);
		
		Duration d = Duration.ofNanos((Integer) ret[0]);
		UUID[] ids = (UUID[]) ret[1];
		String[] names = (String[]) ret[2];
		Stats[] stats = (Stats[]) ret[3];
		
		// TODO notify GUI
	}
	
	private void parsePlayerQuit(JSONObject msg) {
		UUID who = Protocol.parseLeftMessage(msg);
		
		// TODO notify GUI
	}
	
	private void handleChat(JSONObject msg) {
		Object[] ret = Protocol.parseChatRecvMessage(msg);
		UUID from = (UUID) ret[0];
		String message = (String) ret[1];
		
		// TODO notify GUI
	}
	
	private void parse(JSONObject msg) {
		try {
			MsgType type = Protocol.getMessageType(msg);
			switch (type) {
				case CONN_SUCCESS:
					Thread.sleep(100);
					sendIDRequest();
					break;
				case CONN_FULL:
					game.setErrMsg("Server full");
					this.destroy();
					break;
				case CONN_MATCH_STARTED:
					game.setErrMsg("Match has already started");
					this.destroy();
					break;
				case CONN_ERR:
					parseConnErrMsg(msg);
					this.destroy();
					break;
				case USER_LIST:
					parseUserList(msg);
					break;
				case ID_SEND:
					parseIDSend(msg);
					break;
				case MATCH_NOT_HOST:
					changeStatus("not host");
					break;
				case CONFIG_HOST_ACCEPT:
					changeStatus("ok");
					break;
				case MOD_EXECUTED:
					changeStatus("ok");
					break;
				case CONFIG:
					parseGameConfig(msg);
					break;
				case MOD:
					parseModMessage(msg);
					break;
				case MATCH_PLCM_STARTED:
					changeStatus("ok");
					break;
				case SPRITES_SEND:
					getSprites(msg);
					break;
				case MATCH_START:
					changeStatus("ok");
					break;
				case TURN:
					setTurn(msg);
					break;
				case BOARD:
					parseBoard(msg);
					changeStatus("ok");
					break;
				case ATTACK_STATUS:
					changeStatus(parseAttackStatus(msg));
					break;
				case GOT_ATTACKED:
					parseGotAttacked(msg);
					break;
				case MATCH_END:
					parseMatchEnd(msg);
					break;
				case LEFT:
					parsePlayerQuit(msg);
					break;
				case CHAT_RECV:
					handleChat(msg);
					break;
				default:
					break;
			}
		} catch (Exception e) {
			this.destroy();
		}
	}
	
	private void changeStatus(String str) {
		while (game.getGenericStatus() != GenericStatus.CHANGING);
		game.setErrMsg(str);
		game.changeGenericStatus();
	}
	
	public void sendConfiguration(GameConfig config) {
		sendMsg(Protocol.getConfigurationMessage(config));
	}
	
	public void sendUsername() {
		sendMsg(Protocol.getUserNameMessage(game.getOwnPlayer().getUsername()));
	}
	
	// GUI INTERFACE
	public void sendKick(UUID id) {
		sendMsg(Protocol.getModMessage(id, false));
	}
	
	// GUI INTERFACE
	public void sendBan(UUID id) {
		sendMsg(Protocol.getModMessage(id, true));
	}
	
	// GUI INTERFACE
	public void sendStart() {
		sendMsg(Protocol.getMatchPlcmStartMessage());
	}

	// GUI INTERFACE
	public boolean sendBoard(Board board) {
		Optional<JSONObject> ret = Protocol.getConfigBoardMessage(board);
		if (ret.isPresent()) {
			sendMsg(ret.get());
			return true;
		}
		return false;
	}
	
	// GUI INTERFACE
	public void sendBoardRequest(UUID id, String debug) {
		sendMsg(Protocol.getBoardRequestMessage(id, debug));
	}
	
	// GUI INTERFACE
	public void sendAttack(UUID id, int x, int y) {
		sendMsg(Protocol.getAttackMessage(id, x, y));
	}
	
	// GUI INTERFACE
	public void sendQuit() {
		sendMsg(Protocol.getQuitMessage());
	}
	
	// GUI INTERFACE
	public void sendChatMessage(String msg) {
		sendMsg(Protocol.getChatSendMessage(msg));
	}
	
}
