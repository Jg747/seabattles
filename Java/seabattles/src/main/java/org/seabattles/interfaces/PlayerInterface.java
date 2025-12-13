package org.seabattles.interfaces;

import org.seabattles.src.Board;
import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.Player;
import org.seabattles.src.Ship;
import org.seabattles.src.Stats;

public interface PlayerInterface {
	public Board getBoard();
	public boolean placeShip(Ship s);
	public AttackStatus attack(Player who, int x, int y);
	public Stats getStats();
}
