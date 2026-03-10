package org.seabattles.src;

import org.json.JSONException;
import org.json.JSONObject;

public class ShipConfig {
	private short id;
	private short length;
	private short width;
	private String sprite;
	
	public ShipConfig(JSONObject obj) throws JSONException {
		setID((short) obj.getInt("id"));
		setWidth((short) obj.getInt("width"));
		setLength((short) obj.getInt("length"));
		setSprite(obj.getString("sprite"));
	}
	
	public void setLength(short length) throws IllegalArgumentException {
		if (length < 1) {
			throw new IllegalArgumentException("Ship length must be > 0");
		}
		
		int min = Math.min(Board.getHeigth(), Board.getWidth());
		if (width >= min) {
			throw new IllegalArgumentException("Ship length must be < " + min + " (board constraints)");
		}
		
		this.length = length;
	}
	
	public void setWidth(short width) throws IllegalArgumentException {
		if (width < 1) {
			throw new IllegalArgumentException("Ship width must be > 0");
		}
		
		int min = Math.min(Board.getHeigth(), Board.getWidth());
		if (width >= min) {
			throw new IllegalArgumentException("Ship length must be < " + min + " (board constraints)");
		}
		
		this.width = width;
	}
	
	public void setSprite(String path) {
		if (path.isEmpty()) {
			this.sprite = "null";
		} else {
			this.sprite = path;
		}
	}
	
	public void setID(short id) {
		this.id = id;
	}
	
	public int getID() {
		return id;
	}
	
	public int getLength() {
		return length;
	}
	
	public int getWidth() {
		return width;
	}
	
	public String getSpritePath() {
		return sprite;
	}

	@Override
	public String toString() {
		return "ShipConfig [id=" + id + ", length=" + length + ", width=" + width + ", sprite=" + sprite + "]";
	}
	
}