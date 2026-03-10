package org.seabattles.gui.ascii;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;

public class AsciiController {
	private class InputThread extends Thread {
		private BufferedReader read;
		
		public InputThread() {
			read = new BufferedReader(new InputStreamReader(System.in));
		}
		
		@Override
		public void run() {
			try {
				while (threadRunning && !Thread.currentThread().isInterrupted()) {
					// idk m8, gl hf <o/
					input = read.readLine();
					gotInput = true;
				}
				read.close();
			} catch (IOException e) {}
		}
		
		public void stopThread() {
			threadRunning = false;
			try {
				read.close();
			} catch (IOException e) {}
		}
	};
	
	private class ConsoleClearer {
		private boolean allowClear = true;
		private boolean ansi;
		private ProcessBuilder console;
		
		private ConsoleClearer() {
			if (System.console() != null && System.getenv().get("TERM") != null) {
				ansi = true;
			} else {
				ansi = false;
				console = new ProcessBuilder("cmd", "/c", "cls").inheritIO();
			}
		}
		
		private void clear() {
			if (!allowClear) {
				return;
			}
			
			try {
				if (ansi) {
					System.out.print("\033[H\033[2J");
				    System.out.flush();
				} else {
					console.start().waitFor();
				}
			} catch (IOException | InterruptedException e) {}
		}
	};
	
	public enum InputType {
		INPUT_STRING,
		INPUT_NUMBER,
		INPUT_RANGED_NUMBER,
		INPUT_BOOLEAN
	};
	
	private static final int UPS = 20;		// Target Updates Per Second
	private static final int FPS = 20;		// Target Frames Per Second
	
	private int frames;						// current FPS
	private int ticks;						// current TPS
	
	private AsciiGUI gui;
	private Semaphore haltCallerThread;
	private List<Object[]> pendingEvents;
	private ExecutorService service;
	private InputThread inputThread;
	private ConsoleClearer clearer;
	
	private boolean threadRunning = false;
	
	private boolean refresh = false;		// flag mark that next frame have to be rendered (updated)
	private boolean waitInput = false;		// flag mark that a function is waiting for input (used in update())
	private boolean gotInput = false;		// flag mark that user pressed ENTER (got input)
	private boolean usedInput = true;		// flag mark that input provided got consumed
	
	private String input;					// temp variable containing userInput at any time
	private String currentInput;			// variable updated from getInput() [combo with waitInput = true]
	private StringBuilder syncBuffer;		// buffer written by synchronous calls
	private StringBuilder buffer;			// current frame buffer (sync + async calls)
	private String renderFrame;				// current rendered frame (on screen)
	
	private Object[] params;				// calling method name + params
	private Object returnObj;				// Object containing return data for the caller
	
	public AsciiController(AsciiGUI g) {
		haltCallerThread = new Semaphore(0);
		pendingEvents = new LinkedList<>();
		clearer = new ConsoleClearer();
		gui = g;
	}
	
	public void start() {
		if (!threadRunning) {
			threadRunning = true;
			createInputHandler();
			gameLoop();
		}
	}
	
	public void stop() {
		threadRunning = false;
		inputThread.stopThread();
		service.shutdown();
	}
	
	private void createInputHandler() {
		service = Executors.newSingleThreadExecutor();
		inputThread = new InputThread();
		service.execute(inputThread);
	}
	
	private void gameLoop() {
		final double timeU = 1000000000 / UPS;
		final double timeF = 1000000000 / FPS;
		
		double deltaU = 0, deltaF = 0;
		
		long initialTime = System.nanoTime();
		long timer = System.currentTimeMillis();
		
		frames = 0;
		ticks = 0;
		
		try {
			render();
			while (threadRunning) {
			    long currentTime = System.nanoTime();
			    deltaU += (currentTime - initialTime) / timeU;
			    deltaF += (currentTime - initialTime) / timeF;
			    initialTime = currentTime;

			    if (deltaU >= 1) {
			        getInput();
			        update();
			        ticks++;
			        deltaU--;
			    }

			    if (deltaF >= 1) {
			        render();
			        frames++;
			        deltaF--;
			    }

			    if (System.currentTimeMillis() - timer > 1000) {
			        frames = 0;
			        ticks = 0;
			        timer += 1000;
			    }
			}
		} catch (Exception e) {}
		service.shutdown();
	}
	
	/************************* THREAD SYNC *************************/
	private void waitGUI() {
		try {
			haltCallerThread.drainPermits();
			haltCallerThread.acquire();
		} catch (Exception e) {}
	}
	
	private void releaseGUI() {
		if (haltCallerThread.availablePermits() == 0) {
			haltCallerThread.release();
		}
	}
	
	public void releaseWaitingThread() {
		clearParams();
		waitInput = false;
		releaseGUI();
	}
	
	/************************* INPUT SECTION *************************/
	private void getInput() {
		if (gotInput) {
			currentInput = new String(input);
			gotInput = false;
			usedInput = false;
		}
	}
	
	private void consumeInput() {
		if (params == null || params.length == 0) {
			return;
		}
		
		InputType type = (InputType) params[0];
		if (!usedInput) {
			switch (type) {
				case INPUT_RANGED_NUMBER:
					if (rangedNumberInput((Integer) params[1], (Integer) params[2], true, true)) {
						releaseWaitingThread();
					}
					break;
				case INPUT_NUMBER:
					if (numberInput()) {
						releaseWaitingThread();
					}
					break;
				case INPUT_STRING:
					stringInput();
					releaseWaitingThread();
					break;
				case INPUT_BOOLEAN:
					if (booleanInput()) {
						releaseWaitingThread();
					}
					break;
				default:
					break;
			}
			usedInput = true;
		}
	}
	
	/************************* INPUT TYPES *************************/
	private boolean booleanInput() {
		String in = currentInput;
		if (in.equalsIgnoreCase("s") || 
			in.equalsIgnoreCase("y") || 
			in.equalsIgnoreCase("1") || 
			in.equalsIgnoreCase("si") || 
			in.equalsIgnoreCase("yes")) {
			setReturnObject(true);
			return true;
		}
		
		if (in.equalsIgnoreCase("n") || 
			in.equalsIgnoreCase("0") || 
			in.equalsIgnoreCase("no")) {
			setReturnObject(false);
			return true;
		}
		
		setUpdateFrame();
		return false;
	}
	
	private boolean numberInput() {
		try {
			int ret = Integer.parseInt(currentInput);
			setReturnObject(ret);
			return true;
		} catch (NumberFormatException e) {
			setUpdateFrame();
		}
		return false;
	}
	
	private boolean rangedNumberInput(int min, int max, boolean includeMin, boolean includeMax) {
		try {
			int ret = Integer.parseInt(currentInput);
			if (ret >= min && ret <= max) {
				if (!includeMin && ret == min) {
					setUpdateFrame();
					return false;
				}
				if (!includeMax && ret == max) {
					return false;
				}
				
				setReturnObject(ret);
				return true;
			} else {
				setUpdateFrame();
			}
		} catch (NumberFormatException e) {
			setUpdateFrame();
		}
		return false;
	}
	
	private void stringInput() {
		setReturnObject(currentInput);
	}
	
	/************************* GUI MANAGEMENT *************************/
	private void clearEvents() {
		/*int size = pendingEvents.size();
		for (int i = 0; i < size; i++) {
			pendingEvents.remove(i);
		}*/
		pendingEvents.remove(0);
	}
	
	private void setUpdateFrame() {
		refresh = true;
	}
	
	private void clearParams() {
		params = null;
	}
	
	private void setReturnObject(Object ret) {
		returnObj = ret;
	}
	
	public Object getReturnObject() /*throws NoSuchElementException*/ {
		Object ret = returnObj;
		returnObj = null;
		/*if (ret == null) {
			throw new NoSuchElementException();
		}*/
		return ret;
	}
	
	private void setWaitInput() {
		waitInput = true;
	}
	
	private void callMethod(InputType type, Object... objs) {
		params = new Object[1 + objs.length];
		for (int i = 0; i < objs.length; i++) {
			params[i + 1] = objs[i];
		}
		params[0] = (Object) type;
	}
	
	private Object[] getEventObj(String caller, Object[] params) {
		Object[] ret = new Object[1 + params.length];
		for (int i = 0; i < params.length; i++) {
			ret[i + 1] = params[i];
		}
		ret[0] = (Object) caller;
		return ret;
	}
	
	public void raiseAsyncEvent(String caller, Object... params) {
		Object[] event = getEventObj(caller, params);
		synchronized (pendingEvents) {
			pendingEvents.add(event);
			if (gui.getLastEvent() == null || (gui.getLastEvent() != null && !caller.equals((gui.getLastEvent())[0]))) {
				gui.setLastEvent(event);
			}
		}
	}
	
	public void raiseEvent(String caller, Object... params) {
		raiseAsyncEvent(caller, params);
		waitGUI();
	}
	
	public void release() {
		releaseWaitingThread();
		setUpdateFrame();
	}
	
	/************************* UPDATE SECTION *************************/
	private void invokeReflMethod() {
		try {
			if (pendingEvents.size() > 0) {
				clearBuffer();

				List<Object[]> temp;
				synchronized (pendingEvents) {
					temp = List.copyOf(pendingEvents);
				}
				for (Object[] method : temp) {
					Object[] params = Arrays.copyOfRange(method, 1, method.length);
					Arrays.stream(gui.getClass().getDeclaredMethods())
						.filter(e -> e.getName().equals((String) method[0]))
						.findFirst()
						.ifPresentOrElse(m -> {
							try {
								m.invoke(gui, params);
								//gui.setLastEvent(method);
							} catch (IllegalAccessException | InvocationTargetException e) {
								e.printStackTrace();
							}
						}, () -> {
							throw new RuntimeException("Event " + ((String) method[0]) + " not found");
						});
				}
				
				clearEvents();
				setUpdateFrame();
			}
		} catch (RuntimeException e) {
			for (Object[] method : pendingEvents) {
				System.err.println(Arrays.toString(method));
			}
			e.printStackTrace();
			clearEvents();
			println("[ERROR] " + e.getMessage());
			setUpdateFrame();
		}
	}
	
	private void update() {
		invokeReflMethod();
		if (waitInput) {
			consumeInput();
		}
	}
	
	/************************* RENDER SECTION *************************/
	public void asyncPrint(String msg, boolean before) {
		if (syncBuffer == null) {
			clearSyncBuffer();
		}
		
		if (before) {
			buffer.insert(0, msg + syncBuffer);
		} else {
			buffer.append(msg);
		}
	}
	
	public void asyncPrintln(boolean before) {
		asyncPrint("\n", before);
	}
	
	public void asyncPrintln(String msg, boolean before) {
		asyncPrint(msg + "\n", before);
	}
	
	public void print(String msg) {
		try {
			if (buffer == null) {
				clearBuffer();
			}
			if (syncBuffer == null) {
				clearSyncBuffer();
			}
			
			syncBuffer.append(msg);
			buffer.append(msg);
		} catch (Exception e) {}
	}
	
	public void println() {
		print("\n");
	}
	
	public void println(String msg) {
		print(msg + "\n");
	}
	
	private void clearBuffer() {
		buffer = new StringBuilder();
	}
	
	public void clearSyncBuffer() {
		syncBuffer = new StringBuilder();
	}
	
	private void clearConsole() {
		clearer.clear();
	}
	
	private void render() {
		if (refresh) {
			renderFrame = buffer.toString();
			clearConsole();
			gui.threadWrite(renderFrame);
			refresh = false;
		}
	}
	
	/************************* GUI INPUT FUNCTIONS *************************/
	public void getInput(InputType inType, Object... params) {
		callMethod(inType, params);
		setWaitInput();
	}
}
