package org.seabattles.src;

import java.util.Map;
import java.util.Optional;

import org.seabattles.interfaces.BoardInterface;

public class Board implements BoardInterface {
	
	public enum AttackStatus {
		HIT,
		MISS,
		SUNK,
		INVALID,
		NOT_TURN,
		DEAD,
		ERROR,
		NOTHING
	};
	
	public static final Map<AttackStatus, String> statusStrings= Map.ofEntries(
		Map.entry(AttackStatus.HIT, "hit"),
		Map.entry(AttackStatus.MISS, "miss"),
		Map.entry(AttackStatus.SUNK, "sunk"),
		Map.entry(AttackStatus.INVALID, "invalid"),
		Map.entry(AttackStatus.NOT_TURN, "not_turn"),
		Map.entry(AttackStatus.DEAD, "dead"),
		Map.entry(AttackStatus.ERROR, "error")
	);
	
	private static final short MAX_VALUE = 100;
	private static final short DEFAULT_WIDTH = 10;
	private static final short DEFAULT_HEIGTH = 10;
	
	public static final short NOTHING_FLAG = 0;
	public static final short HIT_FLAG = 1;
	public static final short SHIP_FLAG = 2;
	public static final short SHIP_HIT_FLAG = 3;
	public static final short SHIP_SUNK_FLAG = 4;
	
	private static int heigth = DEFAULT_HEIGTH;
	private static int width = DEFAULT_WIDTH;
	
	private Ship[] ships;
	private byte[][] hits;
	private short shipsRemaining;
	
	public boolean doesExist(Ship s) {
		for (Ship i : ships) {
			if (i.equals(s)) {
				return true;
			}
		}
		return false;
	}
	
	public Board(ShipConfig[] shipsConfig) {
		ships = new Ship[shipsConfig.length];
		for (int i = 0; i < shipsConfig.length; i++) {
			ships[i] = new Ship(shipsConfig[i]);
		}
		
		shipsRemaining = (short) shipsConfig.length;
		createEmptyBoard();
	}
	
	public static short getHeigth() {
		return (short) heigth;
	}
	
	public static short getWidth() {
		return (short) width;
	}
	
	public static void setHeigth(int heigth) throws IllegalArgumentException {
		if (heigth < 1 || heigth > MAX_VALUE) {
			throw new IllegalArgumentException("Board heigth must be 0 < heigth <= " + MAX_VALUE);
		}
		Board.heigth = heigth;
	}
	
	public static void setWidth(int width) throws IllegalArgumentException {
		if (width < 1 || width > MAX_VALUE) {
			throw new IllegalArgumentException("Board width must be 0 < width <= " + MAX_VALUE);
		}
		Board.width = width;
	}
	
	public void createEmptyBoard() {
		hits = new byte[heigth][width];
	}
	
	public void unplaceShip(Ship ship) {
		int heigth = ((ship.getRotation() % 2 == 0) ? ship.getWidth() : ship.getLength());
		int width = ((ship.getRotation() % 2 == 0) ? ship.getLength() : ship.getWidth());
		
		for (int i = ship.getY(); i < ship.getY() + heigth; i++) {
			for (int j = ship.getX(); j < ship.getX() + width; j++) {
				hits[i][j] = NOTHING_FLAG;
			}
		}
		
		ship.unplace();
	}
	
	public static boolean checkPosition(Ship s) {
		switch (s.getRotation()) {
			case 0:
			case 2:
				if (s.getX() + s.getLength() > Board.width) {
					return false;
				}
				
				if (s.getY() + s.getWidth() > Board.heigth) {
					return false;
				}
				break;
			case 1:
			case 3:
				if (s.getX() + s.getWidth() > Board.width) {
					return false;
				}
				
				if (s.getY() + s.getLength() > Board.heigth) {
					return false;
				}
				break;
			default:
				return false;
		}
		return true;
	}
	
	public boolean placeShip(Ship ship) {
		if (!ship.isPlaced()) {
			return false;
		}
		
		if (!checkPosition(ship)) {
			return false;
		}
		
		for (Ship s : ships) {
			if (!ship.equals(s) && ship.collideWith(s)) {
				return false;
			}
		}
		
		int heigth = ((ship.getRotation() % 2 == 0) ? ship.getWidth() : ship.getLength());
		int width = ((ship.getRotation() % 2 == 0) ? ship.getLength() : ship.getWidth());
		
		for (int i = ship.getY(); i < ship.getY() + heigth; i++) {
			for (int j = ship.getX(); j < ship.getX() + width; j++) {
				hits[i][j] = SHIP_FLAG;
			}
		}
		
		return true;
	}
	
	@Override
	public Ship[] getShips() {
		return ships;
	}
	
	private Optional<Ship> updateShipsHealth(int posX, int posY) {
		for (Ship s : ships) {
			int heigth = ((s.getRotation() % 2 == 0) ? s.getWidth() : s.getLength());
			int width = ((s.getRotation() % 2 == 0) ? s.getLength() : s.getWidth());

			if (posX >= s.getX() && posX < s.getX() + width &&
				posY >= s.getY() && posY < s.getY() + heigth) {
				s.addHit();
				return Optional.of(s);
			}
		}
		return Optional.empty();
	}
	
	public byte[][] getHits() {
		return hits;
	}
	
	public byte[][] getObfuscatedHits() {
		byte[][] ret = new byte[hits.length][hits[0].length];
		for (int i = 0; i < hits.length; i++) {
			for (int j = 0; j < hits[i].length; j++) {
				if (hits[i][j] == Board.SHIP_FLAG) {
					ret[i][j] = Board.NOTHING_FLAG;
				} else {
					ret[i][j] = hits[i][j];
				}
			}
		}
		return ret;
	}
	
	public void setBoardMatrix(byte[][] new_board) {
		hits = new_board;
	}
	
	public boolean allShipsGone() {
		return shipsRemaining == 0;
	}
	
	private void updateSunk(Ship s) {
		updateSunkPos(s);
		
		if (shipsRemaining > 0) {
			shipsRemaining--;
		}
	}
	
	private void updateSunkPos(Ship s) {
		int heigth = ((s.getRotation() % 2 == 0) ? s.getWidth() : s.getLength());
		int width = ((s.getRotation() % 2 == 0) ? s.getLength() : s.getWidth());
		
		for (int i = s.getY(); i < s.getY() + heigth; i++) {
			for (int j = s.getX(); j < s.getX() + width; j++) {
				hits[i][j] = SHIP_SUNK_FLAG;
			}
		}
	}
	
	public AttackStatus attackAt(int posX, int posY) {
		if (posX < 0 || posX >= Board.width || posY < 0 || posY >= Board.heigth) {
			return AttackStatus.INVALID;
		}
		
		switch (hits[posY][posX]) {
			case NOTHING_FLAG:
				hits[posY][posX] = HIT_FLAG;
				return AttackStatus.MISS;
			case HIT_FLAG:
			case SHIP_HIT_FLAG:
			case SHIP_SUNK_FLAG:
				return AttackStatus.INVALID;
			case SHIP_FLAG:
				hits[posY][posX] = SHIP_HIT_FLAG;
				Optional<Ship> s = updateShipsHealth(posX, posY);
				if (s.isPresent()) {
					if (s.get().isSunk()) {
						updateSunk(s.get());
						return AttackStatus.SUNK;
					}
					return AttackStatus.HIT;
				}
				return AttackStatus.ERROR;
			default:
				return AttackStatus.ERROR;
		}
	}
	
	public boolean checkAndPlace() {
		for (Ship s : ships) {
			if (s.getID() < 0 || s.getID() >= ships.length || !placeShip(s)) {
				return false;
			}
		}
		return true;
	}

	@Override
	public String toString() {
		String ret = "";
		for (int i = 0; i < hits.length; i++) {
			for (int j = 0; j < hits[i].length; j++) {
				ret += hits[i][j] + "\t";
			}
			ret += "\n";
		}
		return ret;
	}
	
}
