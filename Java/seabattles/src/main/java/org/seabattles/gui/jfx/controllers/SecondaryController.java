package org.seabattles.gui.jfx.controllers;

import java.io.IOException;

import org.seabattles.gui.jfx.FxGUI;

import javafx.fxml.FXML;

public class SecondaryController {

    @FXML
    private void switchToPrimary() throws IOException {
        FxGUI.setRoot("primary");
    }
}