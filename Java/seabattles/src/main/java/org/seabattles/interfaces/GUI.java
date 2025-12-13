package org.seabattles.interfaces;

import java.time.Duration;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;

import org.seabattles.src.Board;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;
import org.seabattles.src.Stats;

public interface GUI {
	public enum GuiAction {
		SHOW_FIELD,
		ATTACK,
		QUIT,
		CHAT,
		NONE
	};
	
	public int getMode();
	public GameConfig configGame() throws Exception;
	public Board placeShips();
	public void showPlayerField();
	public GuiAction getAction();
	public Optional<Player> getWhoPlayer(String msg, Map<UUID, Player> players, Set<UUID> ignore);
	public void showField(Player who, boolean waitInput);
	public Object[] attack(Optional<Player> who);	// int[0] = id, int[1] = x, int[2] = y
	public void invalidAttack(String msg);
	public void endScreen(UUID myID, PlayerStatus myStatus, Duration duration, UUID[] ids, String[] names, Stats[] stats);
}
