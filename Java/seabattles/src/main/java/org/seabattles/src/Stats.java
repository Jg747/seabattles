package org.seabattles.src;

import org.seabattles.src.Player.PlayerGrade;

public class Stats {
	private static final double GRADE_A_TRESHOLD = 0.8;
	private static final double GRADE_B_TRESHOLD = 0.7;
	private static final double GRADE_C_TRESHOLD = 0.56;
	private static final double GRADE_D_TRESHOLD = 0.4;
	
	private short numberOfShots;		// Total shots fired
	private short numberOfHits;			// Shots that hit the target
	private short numberOfSunkShips;	// Ships sunk
	private short numberOfPlayerEliminations; // How many players eliminated by this player
	private PlayerGrade grade;			// Grade of the player
	
	public Stats() {
		numberOfShots = 0;
		numberOfHits = 0;
		numberOfSunkShips = 0;
		numberOfPlayerEliminations = 0;
		grade = null;
	}
	
	public void setNumberOfShots(short numberOfShots) {
		this.numberOfShots = numberOfShots;
	}

	public void setNumberOfHits(short numberOfHits) {
		this.numberOfHits = numberOfHits;
	}

	public void setNumberOfSunkShips(short numberOfSunkShips) {
		this.numberOfSunkShips = numberOfSunkShips;
	}

	public void setNumberOfPlayerEliminations(short numberOfPlayerEliminations) {
		this.numberOfPlayerEliminations = numberOfPlayerEliminations;
	}

	public void setGrade(PlayerGrade grade) {
		this.grade = grade;
	}

	public short getNumberOfShots() {
		return numberOfShots;
	}
	
	public short getNumberOfHits() {
		return numberOfHits;
	}
	
	public short getNumberOfSunkShips() {
		return numberOfSunkShips;
	}
	
	public short getNumberOfEliminations() {
		return numberOfPlayerEliminations;
	}
	
	public void incMiss() {
		numberOfShots++;
	}
	
	public void incHit() {
		numberOfShots++;
		numberOfHits++;
	}
	
	public void incSunk() {
		numberOfSunkShips++;
	}
	
	public void incEliminations() {
		numberOfPlayerEliminations++;
	}
	
	public String getPercentageOfHits() {
		if (numberOfShots > 0) {
			return String.format("%.2f%%", numberOfHits * 100 / numberOfShots);
		}
		return "NaN";
	}
	
	private void computeGrade() {
		double perc;
		if (numberOfShots > 0) {
			perc = numberOfHits / numberOfShots;
		} else {
			perc = -1;
		}
	
		if (perc < 0) {
			grade = PlayerGrade.NAN;
		} else if (perc >= GRADE_A_TRESHOLD) {
			grade = PlayerGrade.A;
		} else if (perc >= GRADE_B_TRESHOLD) {
			grade = PlayerGrade.B;
		} else if (perc >= GRADE_C_TRESHOLD) {
			grade = PlayerGrade.C;
		} else if (perc >= GRADE_D_TRESHOLD) {
			grade = PlayerGrade.D;
		} else {
			grade = PlayerGrade.F;
		}
	}
	
	public PlayerGrade getGrade() {
		computeGrade();
		return grade;
	}
	
	@Override
	public String toString() {
		String ret = "Number of shots: " + numberOfShots + "\n"
				   + "Number of hits: " + numberOfHits + "\n"
				   + "Number of sunk ships: " + numberOfSunkShips + "\n"
				   + "You eliminated " + numberOfPlayerEliminations + " players!\n"
				   + "\nYour grade: " + getGrade();
		return ret;
	}

}