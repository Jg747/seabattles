package org.seabattles.gui.jfx.controllers;

import java.io.IOException;

import org.seabattles.gui.jfx.FxGUI;

import javafx.fxml.FXML;

public class PrimaryController {

    @FXML
    private void switchToSecondary() throws IOException {
        FxGUI.setRoot("secondary");
    }
}
