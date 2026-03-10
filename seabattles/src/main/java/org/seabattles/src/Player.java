package org.seabattles.src;

import java.util.Map;
import java.util.UUID;

import org.seabattles.interfaces.PlayerInterface;
import org.seabattles.src.Board.AttackStatus;

public class Player implements PlayerInterface {
	
	public enum PlayerStatus {
		NOT_READY,		// Ships placing
		READY,			// Ships placed
		HAS_TURN,		// It's his turn
		WINNER,			// He won
		LOSER,			// He lost
		QUIT			// He left the game
	};
	
	public enum PlayerGrade {
		A,				// > 80% shots hit
		B,				// 79% - 70% shots hit
		C,				// 69% - 56% shots hit
		D,				// 55% - 40% shots hit
		F,				// < 40% shots hit
		NAN
	};

	public static byte id_counter = 0;
	
	protected UUID id;
	protected String userName;
	protected Board board;
	protected Stats stats;
	protected PlayerStatus status;
	
	public Player() {
		id = UUID.randomUUID();
		status = PlayerStatus.NOT_READY;
	}
	
	public Player(Map<Integer, ShipConfig> shipsConfig) {
		id = UUID.randomUUID();
		status = PlayerStatus.NOT_READY;
		
		init(shipsConfig);
	}
	
	public Player(UUID id, String userName) {
		this.id = id;
		this.userName = userName;
	}
	
	public void setUsername(String name) {
		userName = name;
	}
	
	public boolean isDead() {
		return status == PlayerStatus.LOSER;
	}
	
	public boolean didQuit() {
		return status == PlayerStatus.QUIT;
	}
	
	// GUI interface
	public String getUsername() {
		return userName;
	}
	
	public void setID(UUID id) {
		this.id = id;
	}
	
	public void setConfig(GameConfig cfg) {
		init(cfg.getShipsConfig());
	}
	
	public void setBoard(Board b) {
		this.board = b;
	}
	
	private void init(Map<Integer, ShipConfig> shipsConfig) {
		board = new Board(shipsConfig);
		stats = new Stats();
	}
	
	public void setStatus(PlayerStatus newStatus) {
		status = newStatus;
	}
	
	// GUI interface
	public PlayerStatus getStatus() {
		return status;
	}
	
	// GUI interface
	public UUID getID() {
		return id;
	}
	
	// GUI interface
	@Override
	public Stats getStats() {
		return stats;
	}
	
	public void generateBoard() {
		board.createEmptyBoard();
	}
	
	protected void updateStats(Player who, AttackStatus status) {
		switch (status) {
			case HIT:
				stats.incHit();
				break;
			case MISS:
				stats.incMiss();
				break;
			case SUNK:
				stats.incHit();
				stats.incSunk();
				if (who.getBoard().allShipsGone()) {
					stats.incEliminations();
				}
				break;
			default:
				break;
		}
	}
	
	protected AttackStatus updateBoardUponAttack(int x, int y) {
		AttackStatus status = board.attackAt(x, y);
		if (status == AttackStatus.SUNK && board.allShipsGone()) {
			this.status = PlayerStatus.LOSER;
		}
		return status;
	}
	
	// GUI interface
	@Override
	public AttackStatus attack(Player who, int x, int y) {
		if (this.getStatus() != PlayerStatus.HAS_TURN) {
			return AttackStatus.INVALID;
		}
		
		if (who.getStatus() != PlayerStatus.READY) {
			return AttackStatus.INVALID;
		}
		
		if (who.id.equals(this.id)) {
			return AttackStatus.INVALID;
		}
		
		AttackStatus status = who.updateBoardUponAttack(x, y);
		
		updateStats(who, status);
		
		return status;
	}
	
	@Override
	public String toString() {
		return "ID: " + id + "\nName: " + userName;
	}

	// GUI interface
	@Override
	public Board getBoard() {
		return board;
	}
	
	// GUI interface
	@Override
	public boolean placeShip(Ship s) {
		return board.placeShip(s);
	}
}
