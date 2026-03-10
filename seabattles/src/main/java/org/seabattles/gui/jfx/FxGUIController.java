package org.seabattles.gui.jfx;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.seabattles.main.Main;
import org.seabattles.src.GameConfig;
import org.seabattles.src.Logger;
import org.seabattles.src.ShipConfig;

import javafx.fxml.FXML;
import javafx.scene.Scene;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.image.PixelReader;
import javafx.scene.image.WritableImage;

public abstract class FxGUIController {
	@FXML private Label title;
	
	protected Object[] params;
	protected Scene scene;
	protected boolean isReady;
	
	protected void initialize() {
		title.setText(Main.name + " - v" + Main.version);
		
		Scene s = title.getScene();
		if (s != null) {
			scene = s;
		} else {
			title.sceneProperty().addListener((obs, oldVal, newVal) -> {
				if (newVal != null) {
					scene = newVal;
					onSceneReady();
				}
			});
		}
	}
	
	public boolean isSceneReady() {
		return scene != null;
	}
	
	public boolean isEverythingReady() {
		return isReady;
	}
	
	public void setParams(Object[] params) {
		this.params = params;
		try {
			applyParams();
		} catch (Exception e) {
			Logger.logException(e);
		}
		params = null;
	}
	
	protected static Image getNoTexture() {
		return new Image(FxGUI.class.getResourceAsStream("sprites/notexture.png"));
	}
	
	protected static Image getUnknownTexture() {
		return new Image(FxGUI.class.getResourceAsStream("sprites/unknown.png"));
	}
	
	protected Object[] getTextures(GameConfig conf) {
		Object[] ret = new Object[2];
		
		Map<Integer, Image> textures = new HashMap<>();
		Map<Integer, ShipConfig> ships = conf.getShipsConfig();
		double maxHeight = 0;
		
		for (ShipConfig s : ships.values()) {
			Image texture;
			if (s.getSpritePath() == null || s.getSpritePath().isEmpty() || s.getSpritePath().isBlank() || s.getSpritePath().equalsIgnoreCase("null")) {
				try (InputStream is = FxGUI.class.getResourceAsStream("sprites/" + s.getID() + ".png")) {
					texture = new Image(is);
				} catch (IOException | NullPointerException e) {
					texture = new Image(FxGUI.class.getResourceAsStream("sprites/notexture.png"));
				}
			} else {
				try (InputStream is = new FileInputStream(s.getSpritePath())) {
					texture = new Image(is);
				} catch (IOException | NullPointerException e) {
					texture = new Image(FxGUI.class.getResourceAsStream("sprites/notexture.png"));
				}
			}
			maxHeight = Math.max(maxHeight, texture.getHeight());
			textures.put(s.getID(), texture);
		}
		
		ret[0] = textures;
		ret[1] = maxHeight;
		
		return ret;
	}
	
	private static WritableImage[][] splitImage(Image originalImage, int rows, int cols) {
		int originalWidth = (int) Math.ceil(originalImage.getWidth());
        int originalHeight = (int) Math.ceil(originalImage.getHeight());
        int chunkWidth = (int) Math.ceil((double) originalWidth / cols);
        int chunkHeight = (int) Math.ceil((double) originalHeight / rows);

        PixelReader reader = originalImage.getPixelReader();
        WritableImage[][] chunks = new WritableImage[rows][cols];

        for (int row = 0; row < rows; row++) {
            for (int col = 0; col < cols; col++) {
                int x = col * chunkWidth;
                int y = row * chunkHeight;

                int width = Math.min(chunkWidth, originalWidth - x);
                int height = Math.min(chunkHeight, originalHeight - y);

                if (width > 0 && height > 0) {
                    WritableImage img = new WritableImage(reader, x, y, width, height);
                    chunks[rows - 1 - row][cols - 1 - col] = img;
                }
            }
        }
        return chunks;
    }
	
	protected Map<Integer, Image[][]> getSplittedTextures(Map<Integer, Image> textures, Map<Integer, ShipConfig> ships) {
		Map<Integer, Image[][]> splitted = new HashMap<>();
		for (ShipConfig s : ships.values()) {
			Image[][] ret = splitImage(textures.get(s.getID()), s.getWidth(), s.getLength());
			splitted.put(s.getID(), ret);
		}
		return splitted;
	}
	
	protected abstract void applyParams();
	protected abstract void onSceneReady();
}
