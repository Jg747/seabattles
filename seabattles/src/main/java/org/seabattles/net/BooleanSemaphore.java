package org.seabattles.net;

import java.util.concurrent.Semaphore;

public class BooleanSemaphore {
	private Semaphore sem;
	private final String name;
	
	public BooleanSemaphore(String name) {
		sem = new Semaphore(0);
		this.name = name;
	}
	
	public void acquire() {
		if (sem.availablePermits() >= 0) {
			try {
				sem.acquire();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void release() {
		if (sem.availablePermits() == 0) {
			sem.release();
		}
	}
	
	public boolean isAcquired() {
		return sem.hasQueuedThreads();
	}
	
	public boolean isReleased() {
		return !sem.hasQueuedThreads();
	}
	
	public int permits() {
		return sem.availablePermits();
	}
	
	public String getName() {
		return name;
	}

	@Override
	public String toString() {
		return "BooleanSemaphore [name=" + name + ", permits=" + sem.availablePermits() + "]";
	}
	
	
}
