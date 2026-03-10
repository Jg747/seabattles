package org.seabattles.gui.jfx;

import java.time.Duration;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

import org.seabattles.gui.jfx.controllers.GameScreenController;
import org.seabattles.gui.jfx.controllers.PlaceShipsController;
import org.seabattles.gui.jfx.controllers.ViewFieldsAndAttackController;
import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;
import org.seabattles.src.Board;
import org.seabattles.src.Game;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.Player;
import org.seabattles.src.Player.PlayerStatus;

import javafx.application.Platform;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.collections.ObservableSet;
import javafx.scene.paint.Color;
import javafx.scene.paint.Paint;

public class FxGUI implements GUI {
	public static final int WIDTH = 1280;
	public static final int HEIGHT = 720;
	
	public static final int DEF_SHIP_TEXTURE_ROTATION = 180;
	
	private static Game game;
	private static EventGUI controller;
	private static ObservableList<String> chat;
	private static ObservableSet<UUID> playerList;
	
	private static Map<UUID, Player> players;
	private static SimpleBooleanProperty updateTurn;
	private static SimpleBooleanProperty updateBoard;
	private static SimpleBooleanProperty updateGameStatus;
	private static boolean endGameReceived;
	
	public static ObservableList<String> getChat() {
		return chat;
	}
	
	public static ObservableSet<UUID> getPlayerList() {
		return playerList;
	}
	
    public FxGUI() {}
    
    public static SimpleBooleanProperty getUpdateBoard() {
    	return updateBoard;
    }
    
    public static SimpleBooleanProperty getTurnChanged() {
    	return updateTurn;
    }
    
    public static SimpleBooleanProperty getUpdateGameStatus() {
    	return updateGameStatus;
    }
    
    @Override
    public void setGame(Game g) {
    	game = g;
    	initGUI();
    }
    
    public static Game getGame() {
    	return game;
    }
    
    public static Paint getCellColor(byte value) {
		switch (value) {
			case Board.HIT_FLAG:
				return Color.rgb(216, 24, 24, 0.5);
			case Board.SHIP_HIT_FLAG:
				return Color.rgb(184, 216, 24, 0.5);
			case Board.SHIP_SUNK_FLAG:
				return Color.rgb(46, 216, 24, 0.5);
			default:
				return Color.rgb(0, 0, 0, 0);
		}
	}

	@Override
	public void startGUI() {
		try {
			EventGUI.launchApp();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void stopGUI() {
		Main.stopPgm();
	}

	private void updateBoard() {
		Platform.runLater(() -> {
			updateBoard.setValue(!updateBoard.getValue());
		});
	}
	
	private void updatePlayerList(boolean add) {
		Platform.runLater(() -> {
			if (add) {
				playerList.addAll(game.getPlayers());
			} else {
				playerList.removeIf(e -> !game.getPlayers().contains(e));
			}
		});
	}
	
	private void updateGameStatus() {
		Platform.runLater(() -> {
			updateGameStatus.setValue(!updateGameStatus.getValue());
		});
	}
	
	private void updatePlayerTurn() {
		while (!EventGUI.isEverythingReady()) {
			Main.sleep(10);
		}
		
		Platform.runLater(() -> {
			updateTurn.setValue(!updateTurn.getValue());
		});
	}
	
	@Override
	public void raiseAsyncEvent(String event, Object... params) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
			
		if ("userList".equals(event)) {
			updatePlayerList(true);
		}
		
		if ("playerQuit".equals(event)) {
			updatePlayerList(false);
		}
		
		if ("playerElimination".equals(event)) {
			updatePlayerTurn();
		}
		
		if ("gotAttacked".equals(event)) {
			updateBoard();
		}
		
		if ("gameEnd".equals(event)) {
			endGameReceived = true;
			updateGameStatus();
		}
		
		if ("playerTurn".equals(event)) {
			updatePlayerTurn();
			releaseFromInput();
		}
		
		if ("test".equals(event)) {
			testEventHook();
		}
	}

	@Override
	public void releaseFromInput() {
		while (EventGUI.permits() != 0);
		EventGUI.releaseGUI();
	}

	@Override
	public void initGUI() {
		updateTurn = new SimpleBooleanProperty(false);
    	updateBoard = new SimpleBooleanProperty(false);
    	updateGameStatus = new SimpleBooleanProperty(false);
    	
    	chat = FXCollections.observableArrayList();
    	playerList = FXCollections.observableSet();
    	
    	endGameReceived = false;
	}

	@Override
	public int getMode() {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		controller.changeScene("mainMenu");
		return controller.getInt();
	}

	@Override
	public GameConfig configGame() throws Exception {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		controller.changeScene("configGame");
		return (GameConfig) controller.getObject();
	}

	@Override
	public String[] getMultiplayerMode() {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		controller.changeScene("getMultiplayerMode");
		return (String[]) controller.getObject();
	}

	@Override
	public boolean waitStartingGame(boolean isHost) throws Exception {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		controller.changeScene("waitStartingGame", isHost);
		if (!EventGUI.isRetObjSet()) {
			return true;
		}
		
		return controller.getBoolean();
	}

	@Override
	public void executeMod(boolean ban) {
		// UNUSED
	}

	@Override
	public void waitGameStartAfterPlacement() {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		if (controller.getCurrentScene() instanceof PlaceShipsController) {
			PlaceShipsController c = (PlaceShipsController) controller.getCurrentScene();
			c.disableControls();
		}
		
		game.getClient().acquire(Game.sems_names[1]);
	}

	@Override
	public String sendChat() {
		// UNUSED
		return null;
	}

	@Override
	public void addChatMessage(UUID from, String msg) {
		Platform.runLater(() -> {
			chat.add(game.getPlayer(from).get().getUsername() + ": " + msg);
		});
	}
	
	@Override
	public void setWaitingTurn(boolean state) {
		// UNUSED
	}

	@Override
	public Board placeShips() {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		if (Main.DEBUG_TESTING_DATA) {
			Board b = new Board(game.getGameConfig().getShipsConfig());
			Main.placeTestShips(b);
			return b;
		}
		
		controller.changeScene("placeShips");
		return (Board) controller.getObject();
	}
	
	@Override
	public GuiAction getAction(boolean waitingForTurn) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		if (endGameReceived) {
			return GuiAction.NONE;
		}
		
		controller.changeScene("gameScreen", waitingForTurn);
		return (GuiAction) controller.getObject();
	}

	@Override
	public Optional<Player> getWhoPlayer(String msg, Map<UUID, Player> players, Set<UUID> ignore) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		if (players == null || ignore == null) {
			return Optional.empty();
		}
		
		// USED TO UPDATE AVAILABLE PLAYERS LIST
		this.players = players.entrySet().stream().filter(e -> {
			return !ignore.contains(e.getKey());
		}).collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
		return Optional.empty();
	}
	
	@Override
	public void showField(Optional<Player> who, boolean own, boolean waitInput) {
		if (own) {
			return;
		}
		
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
	
		controller.changeScene("viewFieldsAndAttack", players);
		controller.consumeReturn();
	}

	@Override
	public Object[] attack(Optional<Player> who) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}

		if (!(controller.getCurrentScene() instanceof GameScreenController) && !(controller.getCurrentScene() instanceof ViewFieldsAndAttackController)) {
			Object[] ret = new Object[1];
			ret[0] = null;
			return ret;
		}
		
		controller.changeSceneUnblocking("viewFieldsAndAttack", players);
		((ViewFieldsAndAttackController) controller.getCurrentScene()).enableAttack();
		EventGUI.waitGUI();
		return (Object[]) controller.getObject();
	}

	@Override
	public void invalidAttack(String msg) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		if (!(controller.getCurrentScene() instanceof ViewFieldsAndAttackController) || !msg.contains("coordinates")) {
			return;
		}
		
		ViewFieldsAndAttackController c = (ViewFieldsAndAttackController) controller.getCurrentScene();
		Platform.runLater(() -> {
			c.attackErr();
		});
		Main.sleep(GUI.INVALID_ATTACK_ANIMATION_TIMER);
	}

	@Override
	public void endScreen(UUID myID, PlayerStatus myStatus, Duration duration, Map<UUID, Object[]> playerStats) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}
		
		controller.changeScene("endScreen", myID, myStatus, duration, playerStats);
	}

	@Override
	public void errorScreen(String msg) {
		if (controller == null) {
			controller = EventGUI.getInstance();
		}

		EventGUI.errorDialog(msg);
	}

	@Override
	public void threadWrite(String msg) {
		Logger.write(msg);
	}

	@Override
	public void threadWriteDebug(String msg) {
		if (controller == null) {
			return;
		}
		
		Platform.runLater(() -> {
			controller.writeDebug(msg);
		});
	}
	
	private void testEventHook() {
		// Executed when "test" event is raised
		releaseFromInput();
	}
}
