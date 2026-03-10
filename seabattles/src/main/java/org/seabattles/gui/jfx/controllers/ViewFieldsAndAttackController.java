package org.seabattles.gui.jfx.controllers;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.interfaces.GUI;
import org.seabattles.main.Main;
import org.seabattles.src.Board;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Player;
import org.seabattles.src.Ship;

import javafx.animation.FillTransition;
import javafx.beans.binding.Bindings;
import javafx.beans.binding.DoubleBinding;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.collections.ListChangeListener;
import javafx.collections.ObservableList;
import javafx.collections.SetChangeListener;
import javafx.concurrent.Task;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.CacheHint;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;
import javafx.scene.control.TextField;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.KeyCode;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.RowConstraints;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.util.Duration;

public class ViewFieldsAndAttackController extends FxGUIController {
	
	@FXML private TabPane boardsView;
	
	@FXML private VBox chatParent;
	@FXML private TextField textSend;
	@FXML private Button sendChat;
	@FXML private ScrollPane chatScrollPane;
	@FXML private VBox chat;
	
	@FXML private StackPane turnDisplay;
	@FXML private VBox turnSelector;
	@FXML private Button backBtn;
	
	private GameConfig conf; 
	private Map<Integer, Image> textures;
	private Map<Integer, Image[][]> splitted;
	private double maxHeight;
	
	private Map<UUID, Player> players;
	private SimpleBooleanProperty turnChanged;
	private boolean isAttacking;
	private Object[] atk;
	
	@Override
	protected void applyParams() {
		players = (Map<UUID, Player>) params[0];
		
		createBoards();
		bindDimensions();
		styleOthers();
		addFunctionality();
		setBehaviour();
		updateTurn();
		updateBoards();
	}
	
	@Override
	protected void onSceneReady() {}
	
	public void setPlayers(Map<UUID, Player> p) {
		players = p;
		updateTurn();
		updateBoards();
	}
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		conf = FxGUI.getGame().getGameConfig();
		
		chatScrollPane.setStyle("-fx-border-color: black");
		chat.heightProperty().addListener((obs, oldVal, newVal) -> {
			chatScrollPane.setVvalue(1);
        });
		
		turnChanged = FxGUI.getTurnChanged();
		turnChanged.addListener((pbs, oldVal, newVal) -> {
			updateTurn();
		});
		
		fillTextures();
	}
	
	private void styleOthers() {
		turnSelector.setAlignment(Pos.CENTER);
	}
	
	private void createBoards() {
		for (Player p : players.values()) {
			VBox playerBoard = createBoard(p.getUsername());
			Tab t = new Tab(p.getUsername(), new StackPane(playerBoard));
			t.setId(p.getID().toString());
			boardsView.getTabs().add(t);
		}
	}
	
	private VBox createBoard(String name) {
		VBox boardVBox = new VBox();
		Label l = new Label(name + "'s board");
		l.setFont(new Font("System Bold", 36));
		boardVBox.getChildren().add(l);
		boardVBox.setAlignment(Pos.TOP_CENTER);
		
		GridPane board = new GridPane();
		board.setId("board");
	    
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
	        	
	        	Rectangle overlay = new Rectangle();
	        	overlay.widthProperty().bind(img.fitWidthProperty());
	        	overlay.heightProperty().bind(img.fitHeightProperty());
	        	overlay.setFill(Color.rgb(0, 0, 0, 0));
	            
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
	    
	    return boardVBox;
	}
	
	private void fillTextures() {
		Object[] ret = getTextures(conf);
		textures = (Map<Integer, Image>) ret[0];
		maxHeight = (double) ret[1];
		splitted = getSplittedTextures(textures, conf.getShipsConfig());
	}
	
	private void bindDimensions() {
		HBox megaParent = (HBox) boardsView.getParent();
		HBox.setHgrow(boardsView, Priority.ALWAYS);
		HBox.setHgrow(turnDisplay, Priority.ALWAYS);
		HBox.setHgrow(chatParent, Priority.ALWAYS);
		
		boardsView.prefWidthProperty().bind(megaParent.widthProperty().multiply(2.5 / 4.0));
		turnDisplay.prefWidthProperty().bind(megaParent.widthProperty().multiply(0.5 / 4.0));
		chatParent.prefWidthProperty().bind(megaParent.widthProperty().multiply(1.0 / 4.0));
		
		chatScrollPane.prefWidthProperty().bind(boardsView.prefHeightProperty());
		chatScrollPane.setFitToWidth(true);
		chatScrollPane.prefHeightProperty().bind(boardsView.heightProperty());
		
		BorderPane.setMargin(chatScrollPane.getParent().getParent(), new Insets(0, 5, 0, 0));
		
		VBox.setMargin(backBtn.getParent(), new Insets(5));
		backBtn.prefWidthProperty().bind(turnSelector.widthProperty().multiply(0.8));

		turnSelector.prefHeightProperty().bind(boardsView.heightProperty().multiply(0.5));
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
			boardsView.prefWidthProperty().bind(((HBox) boardsView.getParent()).widthProperty().multiply(3.0 / 4.0));
			turnDisplay.prefWidthProperty().bind(((HBox) boardsView.getParent()).widthProperty().multiply(1.0 / 4.0));
		}
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
		addAttackFunctionality();
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
	
	private void updateBoards() {
		Tab t = boardsView.getSelectionModel().getSelectedItem();
		if (t != null) {
			updateSelectedBoard(UUID.fromString(t.getId()));
		}
		players.values().forEach(p -> updateBoard(p));
	}
	
	private void updateSelectedBoard(UUID id) {
		if (!isAttacking && FxGUI.getGame().isMultiplayer()) {
			FxGUI.getGame().queryBoard(Optional.ofNullable(players.get(id)));
		}
	}
	
	private void boardOverlay(GridPane pain, Board b) {
		byte[][] board = (!FxGUI.getGame().isMultiplayer() && !Main.DEBUG_STRING.equals(FxGUI.getGame().getGameConfig().getDebugString())) ? b.getObfuscatedHits() : b.getHits();
		ObservableList<Node> panels = pain.getChildren();
		for (int i = 0; i < board.length; i++) {
			for (int j = 0; j < board[i].length; j++) {
				Rectangle overlay = (Rectangle) ((StackPane) panels.get(1 + (i * board[i].length) + j)).getChildren().get(1);
				overlay.setFill(FxGUI.getCellColor(board[i][j]));
			}
		}
	}
	
	private void updateBoard(Player p) {
		Tab t = boardsView.getTabs().stream().filter(e -> UUID.fromString(e.getId()).equals(p.getID())).findFirst().get();
		if (t != null) {
			GridPane boardPane = (GridPane) (((StackPane) (((VBox) (((StackPane) t.getContent()).getChildren().get(0))).getChildren().get(1))).getChildren().get(0));
			boardOverlay(boardPane, p.getBoard());
			placeKnownShipsOrHits(boardPane, p.getBoard());
		}
	}
	
	private Image getCorrectTexture(Ship s, int x, int y) {
		int x_index = x - s.getX();
		int y_index = y - s.getY();
		
		Image ret;
		switch (s.getRotation()) {
			case 0:
				ret = splitted.get(s.getID())[y_index][x_index];
				break;
			case 1:
				ret = splitted.get(s.getID())[s.getWidth() - 1 - x_index][y_index];
				break;
			case 2:
				ret = splitted.get(s.getID())[s.getWidth() - 1 - y_index][s.getLength() - 1 - x_index];
				break;
			case 3:
				ret = splitted.get(s.getID())[x_index][s.getLength() - 1 - y_index];
				break;
			default:
				ret = getNoTexture();
				break;
		}
		
		return ret;
	}
	
	private void placeTexture(ImageView img, Board b, int i, int j) {
		boolean imgSet = false;
		
		for (Ship s : b.getShips().values()) {
			if (s.occupiesCoords(j, i)) {
				img.setImage(getCorrectTexture(s, j, i));
				img.setRotate((s.getRotation() * 90) + FxGUI.DEF_SHIP_TEXTURE_ROTATION);
				imgSet = true;
				break;
			}
		}
		
		if (!imgSet) {
			img.setImage(getUnknownTexture());
			img.setRotate(0);
		}
	}
	
	private void placeKnownShipsOrHits(GridPane pain, Board b) {
		byte[][] matrix = (!FxGUI.getGame().isMultiplayer() && !Main.DEBUG_STRING.equals(FxGUI.getGame().getGameConfig().getDebugString())) ? b.getObfuscatedHits() : b.getHits();
		ImageView img;
		for (int i = 0; i < matrix.length; i++) {
			for (int j = 0; j < matrix[i].length; j++) {
				switch (matrix[i][j]) {
					case Board.SHIP_HIT_FLAG:
					case Board.SHIP_FLAG:
						img = (ImageView) ((StackPane) pain.getChildren().get(1 + (i * matrix[i].length) + j)).getChildren().get(0);
						img.setImage(getUnknownTexture());
						img.setRotate(0);
						break;
					case Board.SHIP_SUNK_FLAG:
						img = (ImageView) ((StackPane) pain.getChildren().get(1 + (i * matrix[i].length) + j)).getChildren().get(0);
						for (Ship s : b.getShips().values()) {
							if (s.occupiesCoords(j, i)) {
								img.setImage(getCorrectTexture(s, j, i));
								img.setRotate((s.getRotation() * 90) + FxGUI.DEF_SHIP_TEXTURE_ROTATION);
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	
	public void attackErr() {
		GridPane g = (GridPane) (((StackPane) (((VBox) (((StackPane) boardsView.getSelectionModel().getSelectedItem().getContent()).getChildren().get(0))).getChildren().get(1))).getChildren().get(0));
		StackPane cell = (StackPane) g.getChildren().get(1 + ((Integer) atk[2] * Board.getWidth()) + ((Integer) atk[1]));
		Rectangle r = (Rectangle) cell.getChildren().get(1);

		FillTransition animation = new FillTransition(Duration.millis(GUI.INVALID_ATTACK_ANIMATION_TIMER), r, (Color) r.getFill(), Color.rgb(255, 0, 0, 1));
		
		EventHandler<MouseEvent> enter = (EventHandler<MouseEvent>) cell.getOnMouseEntered();
		EventHandler<MouseEvent> exit = (EventHandler<MouseEvent>) cell.getOnMouseExited();
		EventHandler<MouseEvent> click = (EventHandler<MouseEvent>) cell.getOnMouseClicked();
		
		cell.setOnMouseEntered(null);
		cell.setOnMouseExited(null);
		cell.setOnMouseClicked(null);
		
		animation.setOnFinished(e -> {
			cell.setOnMouseEntered(enter);
			cell.setOnMouseExited(exit);
			cell.setOnMouseClicked(click);
			EventGUI.releaseGUI();
		});
	    
		animation.play();
	}
	
	public void enableAttack() {
		isAttacking = true;
	}
	
	
	private void disableAttack() {
		isAttacking = false;
	}
	
	
	private void addAttackFunctionality() {
		backBtn.setOnAction(e -> {
			if (isAttacking) {
				Object[] ret = new Object[1];
				ret[0] = null;
				
				disableAttack();
				EventGUI.returnObj(ret);
			} else {
				EventGUI.returnObj(Optional.<Player>empty());
			}
		});
		
		boardsView.getSelectionModel().selectedItemProperty().addListener((obs, oldTab, newTab) -> {
			if (newTab != null) {
				if (!isAttacking) {
					updateBoards();
				}
			}
		});
		
		boardsView.getTabs().forEach(t -> {
			GridPane boardPane = (GridPane) (((StackPane) (((VBox) (((StackPane) t.getContent()).getChildren().get(0))).getChildren().get(1))).getChildren().get(0));
			boardPane.getChildren().forEach(e -> {
				if (!(e instanceof StackPane)) {
					return;
				}
				
				StackPane p = (StackPane) e;
				Rectangle r = (Rectangle) p.getChildren().get(1);
				
				
				p.setOnMouseEntered(event -> {
					if (isAttacking) {
						r.setFill(Color.rgb(216, 24, 194, 0.5));
					}
				});
				
				p.setOnMouseExited(event -> {
					if (isAttacking) {
						Optional<Player> player = FxGUI.getGame().getPlayer(UUID.fromString(t.getId()));
						if (player.isEmpty()) {
							return;
						}
						Board b = player.get().getBoard();
						byte[][] matrix = (!FxGUI.getGame().isMultiplayer() && !Main.DEBUG_STRING.equals(FxGUI.getGame().getGameConfig().getDebugString())) ? b.getObfuscatedHits() : b.getHits();
						r.setFill(FxGUI.getCellColor(matrix[GridPane.getRowIndex(p)][GridPane.getColumnIndex(p)]));
					}
				});
				
				p.setOnMouseClicked(event -> {
					if (isAttacking) {
						if (atk == null) {
							atk = new Object[3];
						}
						atk[0] = UUID.fromString(t.getId());
						atk[1] = GridPane.getColumnIndex(p);
						atk[2] = GridPane.getRowIndex(p);
						event.consume();
						EventGUI.returnObj(atk);
					}
				});
			});
		});
	}
}
