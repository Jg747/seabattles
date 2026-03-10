package org.seabattles.interfaces;

import java.time.Duration;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;

import org.seabattles.main.Main;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;

public interface GUI {
	public static final String TITLE = Main.name + " - v" + Main.version;
	public static final int MAX_CHAT_SIZE = 10;
	public static final int INVALID_ATTACK_ANIMATION_TIMER = 500;
	
	public enum GuiAction {
		SHOW_FIELD,
		ATTACK,
		QUIT,
		CHAT,
		NONE
	};
	
	// GUI start
	public void startGUI();
	public void stopGUI();
	public void raiseAsyncEvent(String event, Object... params);
	public void releaseFromInput();
	public void initGUI();
	public void setGame(Game g);
	
	// Multiplayer
	public int getMode();
	public GameConfig configGame() throws Exception;
	public String[] getMultiplayerMode();	// null = back, String[0] = Username, String[1] = IPv4 String[1] = null host
	public boolean waitStartingGame(boolean isHost) throws Exception;
	public void executeMod(boolean ban);
	public void waitGameStartAfterPlacement();
	public String sendChat();
	public void addChatMessage(UUID from, String msg);
	public void setWaitingTurn(boolean state);
	
	// Singleplayer
	public Board placeShips();
	public GuiAction getAction(boolean waitingForTurn);
	public Optional<Player> getWhoPlayer(String msg, Map<UUID, Player> players, Set<UUID> ignore);
	public void showField(Optional<Player> who, boolean own, boolean waitInput);
	public Object[] attack(Optional<Player> who);	// int[0] = id, int[1] = x, int[2] = y
	public void invalidAttack(String msg);
	
	// Other
	public void endScreen(UUID myID, PlayerStatus myStatus, Duration duration, Map<UUID, Object[]> playerStats);
	public void errorScreen(String msg);
	public void threadWrite(String msg);
	public void threadWriteDebug(String msg);
}
