package org.seabattles.gui.jfx.controllers;

import java.util.List;
import java.util.Map;

import org.seabattles.gui.jfx.EventGUI;
import org.seabattles.gui.jfx.FxGUI;
import org.seabattles.gui.jfx.FxGUIController;
import org.seabattles.src.Board;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Ship;
import org.seabattles.src.ShipConfig;

import javafx.application.Platform;
import javafx.beans.binding.Bindings;
import javafx.beans.binding.DoubleBinding;
import javafx.collections.ListChangeListener;
import javafx.fxml.FXML;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Control;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollBar;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.ScrollPane.ScrollBarPolicy;
import javafx.scene.control.TextField;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.ClipboardContent;
import javafx.scene.input.Dragboard;
import javafx.scene.input.KeyCode;
import javafx.scene.input.TransferMode;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.RowConstraints;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;

public class PlaceShipsController extends FxGUIController {
	
	@FXML private VBox boardVBox;
	private GridPane board;
	
	@FXML private Button quitBtn;
	@FXML private Button sendBtn;
	
	@FXML private ScrollPane shipsDisplayWrap;
	@FXML private VBox shipsDisplay;
	
	@FXML private TextField textSend;
	@FXML private Button sendChat;
	@FXML private ScrollPane chatScrollPane;
	@FXML private VBox chat;
	
	@FXML private BorderPane footer;
	
	private Board boardData;
	private GameConfig conf; 
	private Map<Integer, Image> textures;
	private Map<Integer, Image[][]> splitted;
	private double maxHeight;
	private Object dragStartSource;
	
	private Ship selected;
	
	@Override
	protected void applyParams() {}
	
	@Override
	protected void onSceneReady() {
		scene.setOnKeyPressed(e -> {
			if (e.getCode() == KeyCode.R) {
				if (selected != null) {
					rotateShip();
				}
			}
			e.consume();
		});
	}
	
	@FXML
	@Override
	protected void initialize() {
		super.initialize();
		
		quitBtn.setOnAction(e -> {
			EventGUI.returnObj(null);
		});
		
		sendBtn.setOnAction(e -> {
			if (boardData.checkAndPlace()) {
				tryBoard();
			}
		});
		
		chatScrollPane.setStyle("-fx-border-color: black");
		chat.heightProperty().addListener((obs, oldVal, newVal) -> {
			chatScrollPane.setVvalue(1);
        });
		
		createBoard();
		createShipsDisplay();
		
		if (!FxGUI.getGame().isMultiplayer()) {
			disableMultiplayer();
		}
	}
	
	private void disableMultiplayer() {
		chatScrollPane.setVisible(false);
		chatScrollPane.setManaged(false);
		
		((BorderPane) textSend.getParent()).setVisible(false);
		((BorderPane) textSend.getParent()).setManaged(false);
		
		footer.setPrefHeight(Control.USE_COMPUTED_SIZE);
		footer.setPrefWidth(Control.USE_COMPUTED_SIZE);
		BorderPane.setMargin(sendBtn.getParent(), new Insets(5));
	}
	
	private void tryBoard() {
		EventGUI.returnObj(boardData);
	}
	
	private void createBoard() {
		boardData = new Board(FxGUI.getGame().getGameConfig().getShipsConfig());
		
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
	            StackPane cell = new StackPane(img);
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
		VBox shipsParent = (VBox) shipsDisplayWrap.getParent();
		
		shipsDisplay.setFillWidth(true);
		
		shipsDisplayWrap.prefWidthProperty().bind(board.prefHeightProperty());
		shipsDisplayWrap.setFitToWidth(true);
		shipsDisplayWrap.setHbarPolicy(ScrollBarPolicy.NEVER);
		shipsDisplayWrap.setStyle("-fx-border-color: black");
		
		HBox megaParent = (HBox) boardVBox.getParent();
		HBox.setHgrow(boardVBox, Priority.ALWAYS);
		HBox.setHgrow(shipsParent, Priority.ALWAYS);
		
		boardVBox.prefWidthProperty().bind(megaParent.widthProperty().multiply(2.0 / 3.0));
		shipsParent.prefWidthProperty().bind(megaParent.widthProperty().multiply(1.0 / 3.0));
		
		Label[] help = {
				new Label("* Drag your ships on the board and drop them where you want!"),
				new Label("* You can reposition by dragging ships on your field"),
				new Label("* Press R while holding to rotate"),
				new Label("* You can drop back ships to unplace them"),
		};
		VBox labels = new VBox();
		for (Label l : help) {
			l.setFont(new Font("System italic", 14));
			labels.getChildren().add(l);
		}
		shipsParent.getChildren().add(labels);
	}
	
	private VBox getShipBox(ShipConfig s, ImageView image) {
		VBox v = new VBox();
		v.getChildren().add(new Label("Length: " + s.getLength()));
		v.getChildren().add(new Label("Width: " + s.getWidth()));
		HBox h = new HBox(image);
		h.setAlignment(Pos.CENTER);
		h.setPadding(new Insets(5,0,5,0));
		v.getChildren().add(h);
		return v;
	}
	
	private void addImageViews() {
		Map<Integer, ShipConfig> ships = conf.getShipsConfig();
		
		for (ShipConfig s : ships.values()) {
			ImageView image = new ImageView(textures.get(s.getID()));
			
			shipsDisplayWrap.viewportBoundsProperty().addListener((obs, oldB, newB) -> {
				ScrollBar vBar = (ScrollBar) shipsDisplayWrap.lookup(".scroll-bar:vertical");
				double scrollbarWidth = 0;
			    if (vBar != null && vBar.isVisible()) {
			        scrollbarWidth = vBar.getWidth();
			    }
			    double safeWidth = newB.getWidth() - scrollbarWidth;
			    image.setFitWidth(Math.max(0, safeWidth));
			});
			
			image.setId(String.valueOf(s.getID()));
			image.setFitHeight(maxHeight);
			image.setPreserveRatio(true);
			
			StackPane p = new StackPane(getShipBox(s, image));
			p.setPadding(new Insets(5, 5, 5, 5));
			p.setStyle("-fx-border-color: black");
			p.setId(String.valueOf(s.getID()));
			
			shipsDisplay.getChildren().add(p);
		}
	}
	
	private void updateShipImage(boolean unplace) {
		highlightSelectedPanes(selected, unplace);
		
		int x = selected.getX();
		int y = selected.getY();
		int r = selected.getRotation();
		
		if (x == -1 || y == -1) {
			return;
		}
		
		StackPane pane;
		ImageView view;
		Image img;
		
		int width = r % 2 == 0 ? selected.getWidth() : selected.getLength();
		int len = r % 2 == 0 ? selected.getLength() : selected.getWidth();
		
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < len; j++) {
				pane = (StackPane) board.getChildren().get(1 + ((y + i) * Board.getWidth() + (x + j)));
				if (unplace) {
					img = null;
				} else {
					switch (r) {
						case 0:
							img = splitted.get(selected.getID())[i][j];
							break;
						case 1:
							img = splitted.get(selected.getID())[selected.getWidth() - 1 - j][i];
							break;
						case 2:
							img = splitted.get(selected.getID())[selected.getWidth() - 1 - i][selected.getLength() - 1 - j];
							break;
						case 3:
							img = splitted.get(selected.getID())[j][selected.getLength() - 1 - i];
							break;
						default:
							img = getNoTexture();
							break;
					}
				}
				view = (ImageView) pane.getChildren().get(0);
				view.setRotate((r * 90) + FxGUI.DEF_SHIP_TEXTURE_ROTATION);
				view.setImage(img);
			}
		}
	}
	
	private void updateShipOnList() {
		shipsDisplay.getChildren().stream().filter(e -> e.getId().equals(String.valueOf(selected.getID()))).findFirst().ifPresent(e -> {
			if (selected.isPlaced()) {
				e.setVisible(false);
				e.setManaged(false);
			} else {
				e.setVisible(true);
				e.setManaged(true);
			}
		});
	}
	
	private void updateShipPos(Node pane) {
		try {
			if (selected.isPlaced()) {
				updateShipImage(true);
			}
			
			int y = GridPane.getRowIndex(pane);
			int x = GridPane.getColumnIndex(pane);
			
			try {
				selected.setX(x);
				selected.setY(y);
				if (!boardData.placeShip(selected)) {
					repositionShip(selected.getX(), selected.getY());
				}
			} catch (IllegalArgumentException e) {
				repositionShip(x, y);
			}
			
			if (selected != null) {
				updateShipImage(!selected.isPlaced());
				updateShipOnList();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private void styleBorder(Ship selection, int i, int j, StackPane pane) {
		int r = selection.getRotation();
		int width, len;
		boolean paintTop = true, paintBottom = true, paintLeft = true, paintRight = true;
		
		if (r % 2 == 0) {
			width = selection.getWidth();
			len = selection.getLength();
		} else {
			width = selection.getLength();
			len = selection.getWidth();
		}
		
		if (i + 1 < width) {
			// NO PAINT BASSO
			paintBottom = false;
		}
		if (i - 1 >= 0) {
			// NO PAINT ALTO
			paintTop = false;
		}
		if (j + 1 < len) {
			// NO PAINT DESTRO
			paintRight = false;
		}
		if (j - 1 >= 0) {
			// NO PAINT SINISTRO
			paintLeft = false;
		}
		
		pane.setBorder(new Border(new BorderStroke(
			paintTop ? Color.BLACK : null,
			paintRight ? Color.BLACK : null,
			paintBottom ? Color.BLACK : null,
			paintLeft ? Color.BLACK : null,
			paintTop ? BorderStrokeStyle.SOLID : BorderStrokeStyle.NONE,
			paintRight ? BorderStrokeStyle.SOLID : BorderStrokeStyle.NONE,
			paintBottom ? BorderStrokeStyle.SOLID : BorderStrokeStyle.NONE,
			paintLeft ? BorderStrokeStyle.SOLID : BorderStrokeStyle.NONE,
			CornerRadii.EMPTY,
			new BorderWidths(2),
			Insets.EMPTY
		)));
	}
	
	private void clearBorderStyles() {
		board.getChildren().forEach(e -> {
			if (!(e instanceof StackPane)) {
				return;
			}
			
			((StackPane) e).setBorder(null);
		});
	}
	
	private void highlightSelectedPanes(Ship selection, boolean unplace) {
		clearBorderStyles();
		
		boolean wasNull = false;
		if (selection == null) {
			wasNull = true;
			selection = selected;
		}
		
		if (selected == null) {
			return;
		}
		
		int x = selection.getX();
		int y = selection.getY();
		int r = selection.getRotation();
		
		if (x == -1 || y == -1) {
			return;
		}
		
		int width = r % 2 == 0 ? selection.getWidth() : selection.getLength();
		int len = r % 2 == 0 ? selection.getLength() : selection.getWidth();
		
		StackPane pane;
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < len; j++) {
				pane = (StackPane) board.getChildren().get(1 + ((y + i) * Board.getWidth() + (x + j)));
				if (wasNull || unplace) {
					pane.setBorder(null);
				} else {
					styleBorder(selection, i, j, pane);
				}
			}
		}
	}
	
	private void rotateShip() {
		try {
			if (selected.isPlaced()) {
				updateShipImage(true);
			}
			
			try {
				selected.rotate();
				if (!boardData.placeShip(selected)) {
					repositionShip(selected.getX(), selected.getY());
				}
			} catch (IllegalArgumentException e) {
				repositionShip(selected.getX(), selected.getY());
			}
			
			updateShipImage(!selected.isPlaced());
			updateShipOnList();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private void addFunctionality() {
		shipsDisplay.getChildren().forEach(elem -> {
			StackPane pane = (StackPane) elem;
			ImageView img = (ImageView) ((HBox) ((VBox) pane.getChildren().get(0)).getChildren().get(2)).getChildren().get(0);
			
			pane.setOnDragDetected(e -> {
				dragStartSource = pane;
				
				selected = boardData.getShips().get(Integer.parseInt(img.getId()));
				
				Dragboard db = pane.startDragAndDrop(TransferMode.MOVE);
				ClipboardContent content = new ClipboardContent();
				content.putImage(img.getImage());
				db.setContent(content);
				e.consume();
			});
		});
		
		shipsDisplay.setOnDragOver(e -> {
		    if (e.getGestureSource() == dragStartSource && e.getDragboard().hasImage()) {
		        e.acceptTransferModes(TransferMode.MOVE);
		    }
		    e.consume();
		});
		
		shipsDisplay.setOnDragDropped(e -> {
			try {
				updateShipImage(true);
				boardData.unplaceShip(selected);
				updateShipOnList();
				
				e.setDropCompleted(true);
			    e.consume();
			    
			    selected = null;
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		});
		
		board.getChildren().forEach(elem -> {
			if (!(elem instanceof StackPane)) {
				return;
			}
			
			StackPane p = (StackPane) elem;
			
			p.setOnDragOver(e -> {
			    if (e.getGestureSource() == dragStartSource && e.getDragboard().hasImage()) {
			        e.acceptTransferModes(TransferMode.MOVE);
			    }
			    e.consume();
			});
			
			p.setOnDragDropped(e -> {
				try {
					Dragboard db = e.getDragboard();
				    if (db.hasImage()) {
				        updateShipPos(p);
				    }
				    e.setDropCompleted(true);
				    e.consume();
				} catch (Exception ex) {
					ex.printStackTrace();
				}
			});
			
			p.setOnDragDetected(e -> {
				dragStartSource = p;
				
				selected = searchSelectedShip(p);
				if (selected == null) {
					e.consume();
					return;
				}
				
				Dragboard db = p.startDragAndDrop(TransferMode.MOVE);
				ClipboardContent content = new ClipboardContent();
				content.putImage(textures.get(selected.getID()));
				db.setContent(content);
				e.consume();
			});
			
			p.setOnMouseClicked(e -> {
				Ship selection = searchSelectedShip(p);
				highlightSelectedPanes(selection, false);
				if (selection != null) {
					selected = selection;
				}
			});
		});
		
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
	
	private Ship searchSelectedShip(StackPane p) {
		int y = GridPane.getRowIndex(p);
		int x = GridPane.getColumnIndex(p);
		
		for (Ship s : boardData.getShips().values()) {
			if (s.occupiesCoords(x, y)) {
				return s;
			}
		}
		return null;
	}
	
	private void createShipsDisplay() {
		conf = FxGUI.getGame().getGameConfig();
		
		fillTextures();
		
		addImageViews();
		bindDimensions();
		
		addFunctionality();
	}
	
	private void repositionShip(int x, int y) {
		int r = selected.getRotation();
		int width = r % 2 == 0 ? selected.getLength() : selected.getWidth();
		int heigth = r % 2 == 0 ? selected.getWidth() : selected.getLength();
		
		int max_x = Board.getWidth() - width;
		int max_y = Board.getHeigth() - heigth;
		
		int direction = 0; // v
		if (r % 2 == 1) {
			direction = 1; // >
		}
		
		if (y > max_y) {
			selected.setY(max_y);
		}
		if (x > max_x) {
			selected.setX(max_x);
		}
		
		if (boardData.placeShip(selected)) {
			return;
		}
		
		do {
			x = selected.getX();
			y = selected.getY();
			
			switch (direction) {
				case 0:
					if (y + 1 < max_y) {
						selected.setY(y + 1);
					} else {
						direction++;
					}
					break;
				case 1:
					if (x + 1 < max_x) {
						selected.setX(x + 1);
					} else {
						direction++;
					}
					break;
				case 2:
					if (y - 1 >= 0) {
						selected.setY(y - 1);
					} else {
						direction++;
					}
					break;
				case 3:
					if (x - 1 >= 0) {
						selected.setX(x - 1);
					} else {
						direction++;
					}
					break;
				default:
					boardData.unplaceShip(selected);
					throw new RuntimeException("Can't place ship [" + selected.getID() + "]: no board space left");
			}
		} while (!boardData.placeShip(selected));
	}

	private void updateChat(List<? extends String> added) {
		for (String s : added) {
			Label l = new Label(s);
			l.setFont(new Font("System", 18));
			l.setWrapText(true);
			chat.getChildren().add(l);
		}
		
		chatScrollPane.layout();
		chatScrollPane.setVvalue(1);
	}
	
	private void chatSend() {
		String msg = textSend.getText();
		
		if (msg.isEmpty() || msg.isBlank()) {
			return;
		}
		
		textSend.setText("");
		FxGUI.getGame().getClient().sendChatMessage(msg);
	}

	public void disableControls() {
		highlightSelectedPanes(selected, true);
		
		scene.setOnKeyPressed(null);
		
		shipsDisplay.getChildren().forEach(elem -> {
			StackPane pane = (StackPane) elem;
			pane.setOnDragDetected(null);
		});

		shipsDisplay.setOnDragOver(null);
		shipsDisplay.setOnDragDropped(null);
		
		board.getChildren().forEach(elem -> {
			if (!(elem instanceof StackPane)) {
				return;
			}
			
			StackPane p = (StackPane) elem;
			
			p.setOnDragOver(null);
			p.setOnDragDropped(null);
			p.setOnDragDetected(null);
			p.setOnMouseClicked(null);
		});
		
		Platform.runLater(() -> {
			changeLayout();
		});
	}
	
	private void changeLayout() {
		footer.setVisible(false);
		footer.setManaged(false);
		
		VBox shipsParent = (VBox) shipsDisplayWrap.getParent();
		shipsParent.getChildren().forEach(e -> {
			e.setVisible(false);
			e.setManaged(false);
		});
		
		Label l = new Label("Chat");
		l.setFont(new Font("System Bold", 24));
		shipsParent.getChildren().add(l);
		
		Node bottomChat = footer.getChildren().get(0);
		footer.getChildren().remove(0);
		footer.getChildren().remove(chatScrollPane);
		BorderPane.setAlignment(footer.getChildren().get(0), Pos.BOTTOM_CENTER);
		
		shipsParent.getChildren().addAll(chatScrollPane, bottomChat);
		
		chatScrollPane.prefWidthProperty().bind(board.prefHeightProperty());
		chatScrollPane.setFitToWidth(true);
		chatScrollPane.prefHeightProperty().bind(board.prefHeightProperty());
		
		BorderPane.setMargin(shipsParent.getParent(), new Insets(0, 5, 0, 0));
		
		boardVBox.getChildren().stream().filter(e -> e instanceof Label).findFirst().ifPresent(e -> ((Label) e).setText("YOUR BOARD"));
	}

}
