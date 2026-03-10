package org.seabattles.src;

import java.util.Objects;

import org.seabattles.interfaces.ShipInterface;

public class Ship implements ShipInterface {
	
	public enum ShipAction {
		ACT_PLACE,
		ACT_UNPLACE,
		ACT_INCX,
		ACT_INCY,
		ACT_DECX,
		ACT_DECY,
		ACT_ROTATE,
	};
	
	private short width;
	private short length;
	
	private short id;
	
	private short x;
	private short y;
	private short rotation; // 0) >, 1) v, 2) <, 3) ^ 
	
	private short health;
	
	private String sprite;
	
	public Ship(ShipConfig cfg) {
		this.width = (short) cfg.getWidth();
		this.length = (short) cfg.getLength();
		this.sprite = cfg.getSpritePath();
		
		id = (short) cfg.getID();
		x = -1;
		y = -1;
		rotation = 0;
		health = (short)(width * length);
	}
	
	public void setSprite(String path) {
		sprite = path;
	}
	
	public void rotate() {
		rotation = (short)((rotation + 1) % 4);
	}
	
	public void rotate(int rotation) throws IllegalArgumentException {
		if (rotation < 0 || rotation > 3) {
			throw new IllegalArgumentException("Illegal ship rotation [" + id + "]");
		}
		this.rotation = (short) rotation;
	}
	
	public void unplace() {
		x = -1;
		y = -1;
	}
	
	public void place() {
		setX(0);
		setY(0);
		rotate(0);
	}
	
	public boolean isPlaced() {
		return x >= 0 && y >= 0 && x < Board.getWidth() && y < Board.getHeigth();
	}
	
	public boolean occupiesCoords(int x, int y) {
		if (!isPlaced()) {
			return false;
		}
		
		int x1 = this.x;
		int y1 = this.y;
		int x2 = this.x + (this.rotation % 2 == 0 ? this.length : this.width) - 1;
		int y2 = this.y + (this.rotation % 2 == 0 ? this.width : this.length) - 1;
		
		return x1 <= x && x <= x2 && y1 <= y && y <= y2;		
	}
	
	@Override
	public void shipAction(ShipAction action) {
		try {
			switch (action) {
				case ACT_PLACE:
					place();
					break;
				case ACT_UNPLACE:
					unplace();
					break;
				case ACT_ROTATE:
					rotate();
					break;
				case ACT_INCX:
					setX(x + 1);
					break;
				case ACT_DECX:
					setX(x);
					break;
				case ACT_INCY:
					setY(y + 1);
					break;
				case ACT_DECY:
					setY(y - 1);
					break;
			}
		} catch (IllegalArgumentException e) {}
	}
	
	public void setX(int x) throws IllegalArgumentException {
		boolean bad = false;
		
		short bwidth = Board.getWidth();
		
		if (x < 0 || x > bwidth) {
			bad = true;
		}
		
		switch (rotation) {
			case 0:
			case 2:
				if (x + length > bwidth) {
					bad = true;
				}
				break;
			case 1:
			case 3:
				if (x + width > bwidth) {
					bad = true;
				}
				break;
			default: break;
		}
		
		if (bad) {
			throw new IllegalArgumentException("Illegal ship position (x) [" + id +"]");
		}
		
		this.x = (short) x;
	}
	
	public void setY(int y) throws IllegalArgumentException {
		boolean bad = false;
		
		short bheigth = Board.getHeigth();
		
		if (y < 0 || y > bheigth) {
			bad = true;
		}
		
		switch (rotation) {
			case 0:
			case 2:
				if (y + width > bheigth) {
					bad = true;
				}
				break;
			case 1:
			case 3:
				if (y + length > bheigth) {
					bad = true;
				}
				break;
			default: break;
		}
		
		if (bad) {
			throw new IllegalArgumentException("Illegal ship position (y) [" + id + "]");
		}
		
		this.y = (short) y;
	}
	
	public void addHit() {
		if (health > 0) {
			health--;
		}
	}
	
	@Override
	public boolean isSunk() {
		return health == 0;
	}
	
	public int getRotation() {
		return rotation;
	}
	
	public int getX() {
		return x;
	}
	
	public int getY() {
		return y;
	}
	
	public int getLength() {
		return length;
	}
	
	public int getWidth() {
		return width;
	}
	
	public int getID() {
		return id;
	}
	
	@Override
	public boolean collideWith(Ship other) {
		if (other == null) {
			return false;
		}
		
		if (!other.isPlaced()) {
			return false;
		}
		
		int x1 = this.x;
		int y1 = this.y;
		int x2 = this.x + (this.rotation % 2 == 0 ? this.length : this.width) - 1;
		int y2 = this.y + (this.rotation % 2 == 0 ? this.width : this.length) - 1;
		
		int otherx1 = other.x;
		int othery1 = other.y;
		int otherx2 = other.x + (other.rotation % 2 == 0 ? other.length : other.width) - 1;
		int othery2 = other.y + (other.rotation % 2 == 0 ? other.width : other.length) - 1;
		
		if (x2 < otherx1 || otherx2 < x1) {
			return false;
		}
		
		if (y2 < othery1 || othery2 < y1) {
			return false;
		}
		
		return true;
	}
	
	

	@Override
	public String toString() {
		return "Ship [width=" + width + ", length=" + length + ", id=" + id + ", x=" + x + ", y=" + y + ", rotation="
				+ rotation + ", health=" + health + ", sprite=" + sprite + "]";
	}

	@Override
	public int hashCode() {
		return Objects.hash(health, id, length, rotation, width, x, y);
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Ship other = (Ship) obj;
		return health == other.health && id == other.id && length == other.length && rotation == other.rotation
				&& width == other.width && x == other.x && y == other.y;
	}
}
