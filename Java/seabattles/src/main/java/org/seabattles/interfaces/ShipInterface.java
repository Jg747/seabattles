package org.seabattles.interfaces;

import org.seabattles.src.Ship;
import org.seabattles.src.Ship.ShipAction;

public interface ShipInterface {
	public void shipAction(ShipAction action);
	public boolean isSunk();
	public boolean collideWith(Ship other);
}
