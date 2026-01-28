use crate::{dialogs::confirm, icons};
use const_format::formatcp;
use egui_extras::{Column, TableBuilder};
use serde::{Deserialize, Serialize};
use std::{
    error::Error,
    fs::File,
    io::{BufReader, BufWriter},
    path::PathBuf,
};

static FILENAME: &str = "npcs.json";

#[derive(Deserialize, Default, Serialize)]
struct NPC {
    id: i32,
    name: String,
    #[serde(default, skip_serializing_if = "is_default")]
    head: i32,
    #[serde(default, skip_serializing_if = "is_default")]
    banner: i32,
}

fn is_default<T: Default + PartialEq>(t: &T) -> bool {
    *t == Default::default()
}

#[derive(Default)]
pub struct NPCs {
    entries: Vec<NPC>,
    to_delete: Option<usize>,
}

impl NPCs {
    pub fn load(&mut self, base: &PathBuf) -> Result<(), Box<dyn Error>> {
        let file = File::open(base.join(FILENAME))?;
        let reader = BufReader::new(file);
        self.entries = serde_json::from_reader(reader)?;
        Ok(())
    }

    pub fn save(&self, base: &PathBuf) -> Result<(), Box<dyn Error>> {
        let file = File::create(base.join(FILENAME))?;
        let writer = BufWriter::new(file);
        serde_json::to_writer(writer, &self.entries)?;
        Ok(())
    }

    pub fn view(&mut self, ctx: &egui::Context, ui: &mut egui::Ui) {
        let mut scroll_to = None;
        let add_shortcut = egui::KeyboardShortcut::new(egui::Modifiers::COMMAND, egui::Key::A);
        ui.horizontal(|ui| {
            if ui.button(formatcp!("{} Add new NPC", icons::ADD)).clicked()
                || ctx.input_mut(|i| i.consume_shortcut(&add_shortcut))
            {
                self.entries.push(NPC::default());
                let lastindex = self.entries.len() - 1;
                self.entries[lastindex].id = self.entries[lastindex - 1].id + 1;
                scroll_to = Some(lastindex);
            }
        });

        let mut table = TableBuilder::new(ui)
            .column(Column::auto())
            .column(Column::initial(200.0))
            .column(Column::auto())
            .column(Column::auto())
            .column(Column::auto());
        if let Some(to) = scroll_to {
            table = table.scroll_to_row(to, None);
        }
        table
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.strong("ID");
                });
                header.col(|ui| {
                    ui.strong("Name");
                });
                header.col(|ui| {
                    ui.strong("Head");
                });
                header.col(|ui| {
                    ui.strong("Banner");
                });
                header.col(|ui| {
                    ui.strong("Delete");
                });
            })
            .body(|body| {
                body.rows(18.0, self.entries.len(), |mut row| {
                    let row_index = row.index();
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].id).range(-70..=1000), // general boundaries for npc ids
                        );
                    });
                    row.col(|ui| {
                        ui.text_edit_singleline(&mut self.entries[row_index].name);
                    });
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].head).range(0..=300), // id boundaries for npc heads
                        );
                    });
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].banner)
                                .range(0..=500), // id boundaries for banners
                        );
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::DELETE).clicked() {
                            self.to_delete = Some(row_index);
                        }
                    });
                });
            });
        if let Some(pos) = self.to_delete
            && let Some(okay) = confirm(ctx, "Are you sure?")
        {
            if okay {
                self.entries.remove(pos);
            }
            self.to_delete = None;
        }
    }
}
