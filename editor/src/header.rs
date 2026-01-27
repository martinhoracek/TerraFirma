use crate::{dialogs::confirm, icons};
use const_format::formatcp;
use egui_extras::{Column, TableBuilder};
use serde::{Deserialize, Serialize};
use std::{
    error::Error,
    fs::File,
    io::{BufReader, BufWriter},
    mem,
    path::PathBuf,
};

static FILENAME: &str = "header.json";

#[derive(Deserialize, Default, Serialize)]
struct HeaderEntry {
    name: String,
    #[serde(rename = "type")]
    entry_type: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    num: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    relnum: Option<String>,
    #[serde(default, skip_serializing_if = "is_default")]
    min: i32,
}

fn is_default<T: Default + PartialEq>(t: &T) -> bool {
    *t == Default::default()
}

#[derive(Default)]
pub struct Header {
    entries: Vec<HeaderEntry>,
    to_delete: Option<usize>,
}

impl Header {
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
                .button(formatcp!("{} Add new header", icons::ADD))
                .clicked()
            {
                self.entries.push(HeaderEntry::default());
                scroll_to = Some(self.entries.len() - 1);
            }
        });
        let mut table = TableBuilder::new(ui)
            .column(Column::initial(200.0))
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
                    ui.strong("Name");
                });
                header.col(|ui| {
                    ui.strong("Type");
                });
                header.col(|ui| {
                    ui.strong("Array");
                });
                header.col(|ui| {
                    ui.strong("Min Version");
                });
                header.col(|ui| {
                    ui.strong("Reorder");
                });
                header.col(|ui| {
                    ui.strong("Delete");
                });
            })
            .body(|body| {
                let mut swap = None;
                let last = self.entries.len();
                body.rows(18.0, self.entries.len(), |mut row| {
                    let row_index = row.index();
                    row.col(|ui| {
                        ui.text_edit_singleline(&mut self.entries[row_index].name);
                    });
                    row.col(|ui| {
                        let mut sel = self.entries[row_index].entry_type.as_ref();
                        egui::ComboBox::from_label("")
                            .selected_text(&self.entries[row_index].entry_type)
                            .show_ui(ui, |ui| {
                                ui.selectable_value(&mut sel, "b", "Boolean");
                                ui.selectable_value(&mut sel, "s", "String");
                                ui.selectable_value(&mut sel, "u8", "8-bit unsigned");
                                ui.selectable_value(&mut sel, "i16", "16-bit signed");
                                ui.selectable_value(&mut sel, "i32", "32-bit signed");
                                ui.selectable_value(&mut sel, "i64", "64-bit signed");
                                ui.selectable_value(&mut sel, "f32", "32-bit float");
                                ui.selectable_value(&mut sel, "f64", "64-bit float");
                            });
                        self.entries[row_index].entry_type = sel.to_owned();
                    });
                    row.col(|ui| {
                        let mut arr = String::new();
                        if let Some(a) = self.entries[row_index].num {
                            arr = a.to_string();
                        } else if let Some(a) = self.entries[row_index].relnum.as_ref() {
                            arr = a.to_owned();
                        }
                        ui.text_edit_singleline(&mut arr);
                        // is it an integer?
                        match arr.parse::<i32>() {
                            Ok(a) => {
                                self.entries[row_index].num = Some(a);
                                self.entries[row_index].relnum = None;
                            }
                            Err(_) => {
                                self.entries[row_index].num = None;
                                self.entries[row_index].relnum = (!arr.is_empty()).then_some(arr);
                            }
                        }
                    });
                    row.col(|ui| {
                        ui.add(
                            egui::DragValue::new(&mut self.entries[row_index].min).range(0..=500), // version range
                        );
                    });
                    row.col(|ui| {
                        if row_index != last - 1 {
                            if ui.small_button(icons::MOVEDOWN).clicked() {
                                swap = Some(row_index);
                            }
                        }
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::DELETE).clicked() {
                            self.to_delete = Some(row_index);
                        }
                    });
                });
                if let Some(p) = swap {
                    let t = mem::replace(&mut self.entries[p], HeaderEntry::default());
                    self.entries[p] = mem::replace(&mut self.entries[p + 1], t);
                }
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
