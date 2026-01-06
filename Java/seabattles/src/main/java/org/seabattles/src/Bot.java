package org.seabattles.src;

import java.util.HashMap;
import java.util.Random;
import java.util.UUID;

import org.seabattles.src.Board.AttackStatus;
import org.seabattles.src.GameConfig.GameDifficulty;

public class Bot extends Player {
	
	private static GameDifficulty botDiff = GameDifficulty.NORMAL;
	
	public static void setBotDifficulty(GameDifficulty diff) {
		botDiff = diff;
	}
	
	private Random rndGen;
	private HashMap<UUID, Integer[]> lastAttacks;
	
	public Bot(ShipConfig[] shipsConfig) {
		super(shipsConfig);
		userName = "Bot-" + id.toString().substring(0, 5);
		rndGen = new Random();
		lastAttacks = null;
	}
	
	public void placeShips() {
		Ship[] ships = board.getShips();
		
		for (Ship s : ships) {
			do {
				try {
					s.unplace();
					int rndRot = rndGen.nextInt(4);
					int rndX = rndGen.nextInt(Board.getWidth());
					int rndY = rndGen.nextInt(Board.getHeigth());
					/*int rndRot = 0; // TODO TESTING
					int rndX = 0;
					int rndY = 0;*/
					s.rotate(rndRot);
					s.setX(rndX);
					s.setY(rndY);
				} catch (IllegalArgumentException e) {}
			} while (!board.placeShip(s));
		}
		
		this.status = PlayerStatus.READY;
	}
	
	public AttackStatus attack(Player who) {
		if (this.getStatus() != PlayerStatus.HAS_TURN) {
			return AttackStatus.INVALID;
		}
		
		if (who.getStatus() != PlayerStatus.READY) {
			return AttackStatus.INVALID;
		}
		
		if (who.id == this.id) {
			return AttackStatus.INVALID;
		}
		
		if (botDiff == GameDifficulty.HARD && lastAttacks == null) {
			lastAttacks = new HashMap<>();
		}
		
		// System.err.println(id + ": attacked " + who.getID());
		
		AttackStatus status = AttackStatus.ERROR;
		
		int bwidth = Board.getWidth();
		int bheight = Board.getHeigth();
		byte[][] attackBoard = who.getBoard().getHits();
		
		Integer[] lastAttack = null;
		if (botDiff == GameDifficulty.HARD) {
			if (lastAttacks.containsKey(who.getID())) {
				lastAttack = lastAttacks.get(who.getID());
			} else {
				lastAttack = new Integer[5];
				lastAttack[0] = -1; // starting x
				lastAttack[1] = -1; // starting y
				lastAttack[2] = -1; // starting rot
				lastAttack[3] = -1; // last x
				lastAttack[4] = -1; // last y
			}
		}
		
		int x = -1, y = -1;
		int rotation = 0;
		
		do {
			switch (botDiff) {
				case NORMAL:
					x = rndGen.nextInt(bwidth);
					y = rndGen.nextInt(bheight);
					break;
				case HARD:
					if (lastAttack[0] == -1 && (status == AttackStatus.ERROR || status == AttackStatus.INVALID)) {
						x = rndGen.nextInt(bwidth);
						y = rndGen.nextInt(bheight);
					} else if (lastAttack[0] != -1 && (status == AttackStatus.ERROR || status == AttackStatus.INVALID)) {
						if (lastAttack[3] == -1) {
							x = lastAttack[0];
							y = lastAttack[1];
						} else {
							x = lastAttack[3];
							y = lastAttack[4];
						}
						rotation = lastAttack[2];
						switch (rotation) {
							case 0:
								x = x + 1;
								break;
							case 1:
								y = y + 1;
								break;
							case 2:
								x = x - 1;
								break;
							case 3:
								y = y - 1;
								break;
							case 4:
								return AttackStatus.ERROR;
						}
					}
					break;
				case IMPOSSIBLE:
					for (int i = 0; i < attackBoard.length; i++) {
						for (int j = 0; j < attackBoard[i].length; j++) {
							if (attackBoard[i][j] == Board.SHIP_FLAG) {
								y = i;
								x = j;
								break;
							}
						}
						if (x >= 0 && y >= 0) {
							break;
						}
					}
					break;
				default: break;
			}
			
			status = who.updateBoardUponAttack(x, y);
			
			if (botDiff == GameDifficulty.HARD && status == AttackStatus.INVALID) {
				int abs = rotation % 2 == 0 ? Math.abs(lastAttack[0] - x) : Math.abs(lastAttack[1] - y);
				if (abs > 1) {
					lastAttack[2] = (rotation + 2) % 4;
				} else {
					lastAttack[2] = rotation + 1;
				}
				lastAttack[3] = -1;
				lastAttack[4] = -1;
			}
		} while (status == AttackStatus.INVALID);
		
		if (status == AttackStatus.ERROR) {
			return status;
		}
		
		if (botDiff == GameDifficulty.HARD) {
			switch (status) {
				case MISS:
					if (lastAttack[0] != -1) {
						int abs = rotation % 2 == 0 ? Math.abs(lastAttack[0] - x) : Math.abs(lastAttack[1] - y);
						if (abs > 1) {
							lastAttack[2] = (rotation + 2) % 4;
						} else {
							lastAttack[2] = rotation + 1;
						}
						lastAttack[3] = -1;
						lastAttack[4] = -1;
					}
					break;
				case HIT:
					if (lastAttack[0] == -1) {
						lastAttack[0] = x;
						lastAttack[1] = y;
						lastAttack[2] = rotation;
						lastAttack[3] = -1;
						lastAttack[4] = -1;
					} else {
						lastAttack[2] = rotation;
						lastAttack[3] = x;
						lastAttack[4] = y;
					}
					break;
				default:
					lastAttack[0] = -1;
					lastAttack[1] = -1;
					lastAttack[2] = -1;
					lastAttack[3] = -1;
					lastAttack[4] = -1;
					break;
			}
			lastAttacks.put(who.getID(), lastAttack);
		}
		
		updateStats(who, status);
		
		return status;
	}
}
