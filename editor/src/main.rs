use crate::{
    globals::Globals, header::Header, items::Items, npcs::NPCs, prefixes::Prefixes, tiles::Tiles,
    walls::Walls,
};
use eframe::egui;
use egui_file_dialog::FileDialog;
use std::{env, error::Error, path::PathBuf};

mod color;
mod dialogs;
mod globals;
mod header;
mod icons;
mod items;
mod npcs;
mod prefixes;
mod tiles;
mod walls;

#[derive(PartialEq)]
enum Tabs {
    Globals,
    Header,
    Items,
    NPCs,
    Prefixes,
    Tiles,
    Walls,
}

enum For {
    Loading,
    Saving,
}

struct Editor {
    loaded: bool,
    pick_for: For,
    file_dialog: FileDialog,
    globals: Globals,
    header: Header,
    items: Items,
    npcs: NPCs,
    prefixes: Prefixes,
    tiles: Tiles,
    walls: Walls,
    selected_tab: Tabs,
}

impl Editor {
    fn load_all(&mut self, path: PathBuf) -> Result<(), Box<dyn Error>> {
        self.globals.load(&path)?;
        self.header.load(&path)?;
        self.items.load(&path)?;
        self.npcs.load(&path)?;
        self.prefixes.load(&path)?;
        self.tiles.load(&path)?;
        self.walls.load(&path)?;
        self.loaded = true;
        Ok(())
    }
    fn save_all(&self, path: PathBuf) -> Result<(), Box<dyn Error>> {
        self.globals.save(&path)?;
        self.header.save(&path)?;
        self.items.save(&path)?;
        self.npcs.save(&path)?;
        self.prefixes.save(&path)?;
        self.tiles.save(&path)?;
        self.walls.save(&path)?;
        Ok(())
    }
}

impl Default for Editor {
    fn default() -> Self {
        let file_dialog = FileDialog::default().initial_directory(
            env::current_dir()
                .unwrap()
                .parent()
                .unwrap()
                .join("assets/jsons"),
        );
        Self {
            loaded: false,
            file_dialog,
            selected_tab: Tabs::Globals,
            pick_for: For::Loading,
            globals: Globals::default(),
            header: Header::default(),
            items: Items::default(),
            npcs: NPCs::default(),
            prefixes: Prefixes::default(),
            tiles: Tiles::default(),
            walls: Walls::default(),
        }
    }
}

impl eframe::App for Editor {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        let open_shortcut = egui::KeyboardShortcut::new(egui::Modifiers::COMMAND, egui::Key::O);
        let save_shortcut = egui::KeyboardShortcut::new(egui::Modifiers::COMMAND, egui::Key::S);

        egui::TopBottomPanel::top("top_panel").show(ctx, |ui| {
            egui::MenuBar::new().ui(ui, |ui| {
                ui.menu_button("File", |ui| {
                    if ui
                        .add(
                            egui::Button::new("Open...")
                                .shortcut_text(ui.ctx().format_shortcut(&open_shortcut)),
                        )
                        .clicked()
                    {
                        self.pick_for = For::Loading;
                        self.file_dialog.pick_directory();
                    }
                    if ui
                        .add_enabled(
                            self.loaded,
                            egui::Button::new("Save...")
                                .shortcut_text(ui.ctx().format_shortcut(&save_shortcut)),
                        )
                        .clicked()
                    {
                        self.pick_for = For::Saving;
                        self.file_dialog.pick_directory();
                    }
                    ui.separator();
                    if ui.button("Quit").clicked() {
                        std::process::exit(0);
                    }
                })
            })
        });
        egui::CentralPanel::default().show(ctx, |ui| {
            if !self.loaded {
                ui.label("Open folder with json files...");
            } else {
                ui.horizontal(|ui| {
                    ui.with_layout(egui::Layout::left_to_right(egui::Align::TOP), |ui| {
                        ui.selectable_value(&mut self.selected_tab, Tabs::Globals, "Globals");
                        ui.selectable_value(&mut self.selected_tab, Tabs::Header, "Header");
                        ui.selectable_value(&mut self.selected_tab, Tabs::Items, "Items");
                        ui.selectable_value(&mut self.selected_tab, Tabs::NPCs, "NPCs");
                        ui.selectable_value(&mut self.selected_tab, Tabs::Prefixes, "Prefixes");
                        ui.selectable_value(&mut self.selected_tab, Tabs::Tiles, "Tiles");
                        ui.selectable_value(&mut self.selected_tab, Tabs::Walls, "Walls");
                    });
                });
                match self.selected_tab {
                    Tabs::Globals => self.globals.view(ctx, ui),
                    Tabs::Header => self.header.view(ctx, ui),
                    Tabs::Items => self.items.view(ctx, ui),
                    Tabs::NPCs => self.npcs.view(ctx, ui),
                    Tabs::Prefixes => self.prefixes.view(ctx, ui),
                    Tabs::Tiles => self.tiles.view(ctx, ui),
                    Tabs::Walls => self.walls.view(ctx, ui),
                };
            }
        });
        self.file_dialog.update(ctx);
        if ctx.input_mut(|i| i.consume_shortcut(&open_shortcut)) {
            self.pick_for = For::Loading;
            self.file_dialog.pick_directory();
        }
        if ctx.input_mut(|i| i.consume_shortcut(&save_shortcut)) {
            self.pick_for = For::Saving;
            self.file_dialog.pick_directory();
        }
        if let Some(path) = self.file_dialog.take_picked() {
            match self.pick_for {
                For::Loading => self.load_all(path).expect("Failed to load json files"),
                For::Saving => self.save_all(path).expect("Failed to save"),
            }
        }
    }
}

fn main() -> eframe::Result<()> {
    let native_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default().with_inner_size((800.0, 600.0)),
        ..Default::default()
    };

    eframe::run_native(
        "Terrafirma Editor",
        native_options,
        Box::new(|_| Ok(Box::<Editor>::default())),
    )
}
