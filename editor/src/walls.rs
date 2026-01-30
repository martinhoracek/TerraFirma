use crate::{color, dialogs::confirm, icons};
use const_format::formatcp;
use egui::Sense;
use egui_extras::{Column, TableBuilder};
use serde::{Deserialize, Serialize};
use std::{
    error::Error,
    fs::File,
    io::{BufReader, BufWriter},
    path::PathBuf,
};

static FILENAME: &str = "walls.json";

#[derive(Deserialize, Default, Serialize)]
struct Wall {
    id: i32,
    #[serde(rename = "ref", skip_serializing_if = "Option::is_none")]
    reference: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    name: Option<String>,
    color: String,
    #[serde(default, skip_serializing_if = "is_default")]
    blend: i32,
    #[serde(default, skip_serializing_if = "is_default")]
    large: i32,
}

fn is_default<T: Default + PartialEq>(t: &T) -> bool {
    *t == Default::default()
}

#[derive(Default)]
pub struct Walls {
    entries: Vec<Wall>,
    to_delete: Option<usize>,
}

impl Walls {
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
        ui.horizontal(|ui| {
            if ui
                .button(formatcp!("{} Add new wall", icons::ADD))
                .clicked()
            {
                self.entries.push(Wall::default());
                scroll_to = Some(self.entries.len() - 1);
            }
        });
        let mut table = TableBuilder::new(ui)
            .column(Column::initial(50.0))
            .column(Column::initial(200.0))
            .column(Column::initial(50.0))
            .column(Column::initial(70.0))
            .column(Column::initial(50.0))
            .column(Column::initial(50.0))
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
                    ui.strong("Color");
                });
                header.col(|ui| {
                    ui.strong("");
                });
                header.col(|ui| {
                    ui.strong("Blend");
                });
                header.col(|ui| {
                    ui.strong("Large");
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
                            egui::DragValue::new(&mut self.entries[row_index].id).range(0..=500), // general boundaries for wall ids
                        );
                    });
                    row.col(|ui| {
                        let mut name = String::new();
                        if let Some(r) = self.entries[row_index].reference {
                            name = r.to_string();
                        } else if let Some(n) = self.entries[row_index].name.as_ref() {
                            name = n.to_owned();
                        }
                        ui.text_edit_singleline(&mut name);
                        match name.parse::<i32>() {
                            Ok(r) => {
                                self.entries[row_index].reference = Some(r);
                                self.entries[row_index].name = None;
                            }
                            Err(_) => {
                                self.entries[row_index].reference = None;
                                self.entries[row_index].name = (!name.is_empty()).then_some(name);
                            }
                        }
                    });
                    row.col(|ui| {
                        let (rect, _) =
                            ui.allocate_exact_size(ui.spacing().interact_size, Sense::hover());
                        ui.painter_at(rect).rect_filled(
                            rect,
                            0,
                            color::string_to_color32(self.entries[row_index].color.as_str()),
                        );
                    });
                    row.col(|ui| {
                        ui.text_edit_singleline(&mut self.entries[row_index].color);
                    });
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].blend).range(0..=500), // boundaries for wall ids
                        );
                    });
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].large).range(0..=2), // boundaries for large
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
