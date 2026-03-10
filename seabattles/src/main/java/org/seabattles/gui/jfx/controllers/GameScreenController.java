package org.seabattles.gui.jfx.controllers;

import java.util.List;
import java.util.Map;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.interfaces.GUI.GuiAction;
import org.seabattles.main.Main;
import org.seabattles.src.Board;
import org.seabattles.src.Game.GameStatus;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Ship;

import javafx.beans.binding.Bindings;
import javafx.beans.binding.DoubleBinding;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.collections.ListChangeListener;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.CacheHint;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.RowConstraints;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;

public class GameScreenController extends FxGUIController {
	
	@FXML private VBox boardVBox;
	private GridPane board;
	
	@FXML private VBox chatParent;
	@FXML private TextField textSend;
	@FXML private Button sendChat;
	@FXML private ScrollPane chatScrollPane;
	@FXML private VBox chat;
	
	@FXML private StackPane turnDisplay;
	@FXML private VBox turnSelector;
	@FXML private Button viewBtn;
	@FXML private Button attackBtn;
	@FXML private Button leaveBtn;
	
	private Board boardData;
	private GameConfig conf; 
	private Map<Integer, Image> textures;
	private Map<Integer, Image[][]> splitted;
	private double maxHeight;
	
	private boolean waitingTurn;
	private SimpleBooleanProperty turnChanged;
	
	@Override
	protected void applyParams() {
		waitingTurn = (boolean) params[0];
		setBehaviour();
	}
	
	@Override
	protected void onSceneReady() {}
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		conf = FxGUI.getGame().getGameConfig();
		boardData = FxGUI.getGame().getOwnPlayer().getBoard();
		
		chatScrollPane.setStyle("-fx-border-color: black");
		chat.heightProperty().addListener((obs, oldVal, newVal) -> {
			chatScrollPane.setVvalue(1);
        });
		
		turnChanged = FxGUI.getTurnChanged();
		turnChanged.addListener((obs, oldVal, newVal) -> {
			updateTurn();
			
			if (FxGUI.getGame().getOwnID().equals(FxGUI.getGame().getTurn())) {
				if (FxGUI.getGame().getStatus() == GameStatus.RUNNING) {
					EventGUI.releaseGUI();
				}
			}
		});
		
		FxGUI.getUpdateGameStatus().addListener((obs, oldVal, newVal) -> {
			if (FxGUI.getGame().getStatus() != GameStatus.RUNNING) {
				while (EventGUI.permits() != 0) {
					Main.sleep(10);
				}
				
				if (EventGUI.getCurrentScene() instanceof GameScreenController) {
					EventGUI.releaseGUI();
				}
			}
		});
		
		isReady = true;
		
		createBoard();
		fillTextures();
		bindDimensions();
		styleOthers();
		addFunctionality();
		placeShips();
		updateTurn();
	}
	
	private void styleOthers() {
		turnSelector.setAlignment(Pos.CENTER);
	}
	
	private void createBoard() {
		board = new GridPane();
	    
		board.setGridLinesVisible(true);
	    board.setAlignment(Pos.CENTER);

	    int rows = Board.getHeigth();
	    int cols = Board.getWidth();

	    StackPane wrap = new StackPane(board);
	    wrap.setPadding(new Insets(5, 5, 5, 5));
	    wrap.setAlignment(Pos.CENTER);
	    VBox.setVgrow(wrap, Priority.ALWAYS);
	    
	    DoubleBinding cellSize = Bindings.createDoubleBinding(
	    	    () -> Math.min(
	    	        (wrap.getWidth() - wrap.getPadding().getLeft() - wrap.getPadding().getRight()) / cols,
	    	        (wrap.getHeight() - wrap.getPadding().getTop() - wrap.getPadding().getBottom()) / rows
	    	    ),
	    	    wrap.widthProperty(),
	    	    wrap.heightProperty()
	    	);
	    
	    for (int r = 0; r < rows; r++) {
	        for (int c = 0; c < cols; c++) {
	        	ImageView img = new ImageView();
	        	img.setRotate(FxGUI.DEF_SHIP_TEXTURE_ROTATION);
	        	img.fitHeightProperty().bind(cellSize);
	        	img.fitWidthProperty().bind(cellSize);
	        	img.setPreserveRatio(true);
	        	img.setCache(true);
	        	img.setCacheHint(CacheHint.SPEED);
	        	
	        	Rectangle overlay = new Rectangle();
	        	overlay.widthProperty().bind(img.fitWidthProperty());
	        	overlay.heightProperty().bind(img.fitHeightProperty());
	        	overlay.setFill(FxGUI.getCellColor(boardData.getHits()[r][c]));
	            
	        	StackPane cell = new StackPane(img, overlay);
	            board.add(cell, c, r);
	        }
	    }
	    
	    for (int c = 0; c < cols; c++) {
	        ColumnConstraints cc = new ColumnConstraints();
	        cc.setMinWidth(0);
	        cc.prefWidthProperty().bind(cellSize);
	        cc.setMaxWidth(Double.MAX_VALUE);
	        board.getColumnConstraints().add(cc);
	    }

	    for (int r = 0; r < rows; r++) {
	        RowConstraints rc = new RowConstraints();
	        rc.setMinHeight(0);
	        rc.prefHeightProperty().bind(cellSize);
	        rc.setMaxHeight(Double.MAX_VALUE);
	        board.getRowConstraints().add(rc);
	    }
	    
	    board.prefWidthProperty().bind(cellSize.multiply(cols));
	    board.prefHeightProperty().bind(cellSize.multiply(rows));
	    
	    boardVBox.getChildren().add(wrap);
	}
	
	private void fillTextures() {
		Object[] ret = getTextures(conf);
		textures = (Map<Integer, Image>) ret[0];
		maxHeight = (double) ret[1];
		splitted = getSplittedTextures(textures, conf.getShipsConfig());
	}
	
	private void bindDimensions() {
		HBox megaParent = (HBox) boardVBox.getParent();
		HBox.setHgrow(boardVBox, Priority.ALWAYS);
		HBox.setHgrow(turnDisplay, Priority.ALWAYS);
		HBox.setHgrow(chatParent, Priority.ALWAYS);
		
		boardVBox.prefWidthProperty().bind(megaParent.widthProperty().multiply(2.5 / 4.0));
		turnDisplay.prefWidthProperty().bind(megaParent.widthProperty().multiply(0.5 / 4.0));
		chatParent.prefWidthProperty().bind(megaParent.widthProperty().multiply(1.0 / 4.0));
		
		chatScrollPane.prefWidthProperty().bind(board.prefHeightProperty());
		chatScrollPane.setFitToWidth(true);
		chatScrollPane.prefHeightProperty().bind(board.prefHeightProperty());
		
		BorderPane.setMargin(chatScrollPane.getParent().getParent(), new Insets(0, 5, 0, 0));
		
		VBox.setMargin(attackBtn.getParent(), new Insets(5));
		viewBtn.prefWidthProperty().bind(turnSelector.widthProperty().multiply(0.8));
		attackBtn.prefWidthProperty().bind(turnSelector.widthProperty().multiply(0.8));
		leaveBtn.prefWidthProperty().bind(turnSelector.widthProperty().multiply(0.8));

		turnSelector.prefHeightProperty().bind(board.prefHeightProperty().multiply(0.5));
		turnSelector.prefWidthProperty().bind(turnDisplay.widthProperty());
		VBox.setMargin(turnSelector, new Insets(5));
		
		turnSelector.setStyle("-fx-border-color: black");
	}
	
	private void setBehaviour() {
		if (FxGUI.getGame().isMultiplayer()) {
			updateChat(FxGUI.getChat());
		}
		
		if (!FxGUI.getGame().isMultiplayer()) {
			chatParent.setVisible(false);
			chatParent.setManaged(false);
			boardVBox.prefWidthProperty().bind(((HBox) boardVBox.getParent()).widthProperty().multiply(3.0 / 4.0));
			turnDisplay.prefWidthProperty().bind(((HBox) boardVBox.getParent()).widthProperty().multiply(1.0 / 4.0));
		}
		
		if (waitingTurn) {
			attackBtn.setVisible(false);
			attackBtn.setManaged(false);
		}
	}
	
	public boolean isWaitingTurn() {
		return waitingTurn;
	}

	private void addChatFunc() {
		FxGUI.getChat().addListener((ListChangeListener.Change<? extends String> c) -> {
			if (c.next()) {
				updateChat(c.getAddedSubList());
			}
		});
		
		textSend.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.ENTER) {
				chatSend();
			}
		});
		
		sendChat.setOnAction(e -> {
			chatSend();
		});
	}
	
	private void addFunctionality() {
		addChatFunc();
		
		viewBtn.setOnAction(e -> {
			EventGUI.returnObj(GuiAction.SHOW_FIELD);
		});
		
		attackBtn.setOnAction(e -> {
			EventGUI.returnObj(GuiAction.ATTACK);
		});
		
		leaveBtn.setOnAction(e -> {
			EventGUI.returnObj(GuiAction.QUIT);
		});
		
		FxGUI.getUpdateBoard().addListener((obs, oldValue, newValue) -> {
			updateBoard();
		});
	}
	
	private void updateChat(List<? extends String> added) {
		for (String s : added) {
			Label l = new Label(s);
			l.setFont(new Font("System", 18));
			l.setWrapText(true);
			chat.getChildren().add(l);
		}
	}
	
	private void chatSend() {
		String msg = textSend.getText();
		
		if (msg.isEmpty() || msg.isBlank()) {
			return;
		}
		
		textSend.setText("");
		FxGUI.getGame().getClient().sendChatMessage(msg);
	}

	private void updateTurn() {
		turnSelector.getChildren().clear();
		
		FxGUI.getPlayerList().stream().filter(id -> !FxGUI.getGame().getPlayer(id).get().isDead() && !FxGUI.getGame().getPlayer(id).get().didQuit()).forEachOrdered(e -> {
			Label l = new Label("> " + FxGUI.getGame().getPlayer(e).get().getUsername());
			if (FxGUI.getGame().getOwnID().equals(e)) {
				l.setText(l.getText() + " - YOU");
			}
			
			l.setWrapText(true);
			l.setFont(new Font("System Regular", 18));
			if (FxGUI.getGame().getTurn().equals(e)) {
				l.setStyle("-fx-background-color: rgba(43, 251, 103, 0.5)");
			}
			l.prefWidthProperty().bind(turnSelector.widthProperty());
			l.setPadding(new Insets(2));
			turnSelector.getChildren().add(l);
		});
	}
	
	private void placeShip(Ship s) {
		int x = s.getX();
		int y = s.getY();
		int r = s.getRotation();
		
		if (x == -1 || y == -1) {
			throw new RuntimeException("Why is this (" + s.getID() + ") unplaced mid game :cry:");
		}
		
		StackPane pane;
		ImageView view;
		Image img;
		
		int width = r % 2 == 0 ? s.getWidth() : s.getLength();
		int len = r % 2 == 0 ? s.getLength() : s.getWidth();
		
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < len; j++) {
				pane = (StackPane) board.getChildren().get(1 + ((y + i) * Board.getWidth() + (x + j)));
				switch (r) {
					case 0:
						img = splitted.get(s.getID())[i][j];
						break;
					case 1:
						img = splitted.get(s.getID())[s.getWidth() - 1 - j][i];
						break;
					case 2:
						img = splitted.get(s.getID())[s.getWidth() - 1 - i][s.getLength() - 1 - j];
						break;
					case 3:
						img = splitted.get(s.getID())[j][s.getLength() - 1 - i];
						break;
					default:
						img = getNoTexture();
						break;
				}
				view = (ImageView) pane.getChildren().get(0);
				view.setRotate((r * 90) + FxGUI.DEF_SHIP_TEXTURE_ROTATION);
				view.setImage(img);
			}
		}
	}
	
	private void placeShips() {
		boardData.getShips().values().forEach(e -> placeShip(e));
	}
	
	private void updateBoard() {
		byte[][] matrix = boardData.getHits();
		for (int i = 0; i < matrix.length; i++) {
			for (int j = 0; j < matrix[i].length; j++) {
				Rectangle r = (Rectangle) ((StackPane) board.getChildren().get(1 + (i * matrix.length) + j)).getChildren().get(1);
				r.setFill(FxGUI.getCellColor(matrix[i][j]));
			}
		}
	}

}
