use crate::{color, dialogs::confirm, icons};
use const_format::formatcp;
use egui::{Layout, Sense};
use egui_extras::{Column, TableBuilder};
use serde::{Deserialize, Serialize};
use std::{
    error::Error,
    fs::File,
    io::{BufReader, BufWriter},
    path::PathBuf,
};

static FILENAME: &str = "tiles.json";

#[derive(PartialEq)]
enum TileAction {
    Nothing,
    Close,
    Edit(usize),
    Delete(usize),
}

#[derive(Deserialize, Default, Serialize)]
struct Tile {
    id: i32,
    #[serde(rename = "ref", skip_serializing_if = "Option::is_none")]
    reference: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    color: Option<String>,
    #[serde(default, skip_serializing_if = "is_default")]
    flags: u16,
    #[serde(default, skip_serializing_if = "is_default")]
    merge: String,
    #[serde(default, skip_serializing_if = "is_default")]
    blend: String,
    #[serde(default, skip_serializing_if = "is_default")]
    skipy: u32,
    #[serde(default, skip_serializing_if = "is_default")]
    toppad: i32,
    #[serde(
        default = "default_dimension",
        skip_serializing_if = "is_default_dimension"
    )]
    w: u32,
    #[serde(
        default = "default_dimension",
        skip_serializing_if = "is_default_dimension"
    )]
    h: u32,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    var: Vec<TileVariant>,
}

#[derive(Deserialize, Default, Serialize)]
struct TileVariant {
    #[serde(skip_serializing_if = "Option::is_none")]
    x: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    minx: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    maxx: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    y: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    miny: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    maxy: Option<i32>,
    #[serde(rename = "ref", skip_serializing_if = "Option::is_none")]
    reference: Option<i32>,
    #[serde(skip_serializing_if = "Option::is_none")]
    name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    color: Option<String>,
    #[serde(default, skip_serializing_if = "is_default")]
    toppad: i32,
    #[serde(
        default = "default_dimension",
        skip_serializing_if = "is_default_dimension"
    )]
    w: u32,
    #[serde(
        default = "default_dimension",
        skip_serializing_if = "is_default_dimension"
    )]
    h: u32,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    var: Vec<TileVariant>,
}

fn is_default<T: Default + PartialEq>(t: &T) -> bool {
    *t == Default::default()
}

fn is_default_dimension(d: &u32) -> bool {
    *d == 18
}

fn default_dimension() -> u32 {
    18
}

mod tile_is {
    pub const SOLID: u16 = 1;
    pub const TRANSPARENT: u16 = 2;
    pub const DIRT: u16 = 4;
    pub const STONE: u16 = 8;
    pub const GRASS: u16 = 0x10;
    pub const PILE: u16 = 0x20;
    pub const FLIP: u16 = 0x40;
    pub const BRICK: u16 = 0x80;
    pub const MOSS: u16 = 0x100;
    pub const MERGE: u16 = 0x200;
    pub const LARGE: u16 = 0x400;
}

#[derive(Default)]
pub struct Tiles {
    entries: Vec<Tile>,
    to_delete: Option<usize>,
    to_edit: Vec<usize>,
}

impl Tiles {
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
                .button(formatcp!("{} Add new tile", icons::ADD))
                .clicked()
            {
                self.entries.push(Tile::default());
                scroll_to = Some(self.entries.len() - 1);
            }
        });
        let mut table = TableBuilder::new(ui)
            .column(Column::initial(50.0))
            .column(Column::initial(200.0))
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
                    ui.strong("Edit");
                });
                header.col(|ui| {
                    ui.strong("Delete");
                });
            })
            .body(|body| {
                body.rows(18.0, self.entries.len(), |mut row| {
                    let row_index = row.index();
                    row.col(|ui| {
                        ui.label(format!(
                            "{}{}",
                            self.entries[row_index].id,
                            if self.entries[row_index].var.len() > 0 {
                                "*"
                            } else {
                                ""
                            }
                        ));
                    });
                    row.col(|ui| {
                        if let Some(r) = self.entries[row_index].reference {
                            ui.label(format!("{}", r));
                        } else if let Some(n) = self.entries[row_index].name.as_ref() {
                            ui.label(n);
                        } else {
                            ui.label("[Unknown]");
                        }
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::EDIT).clicked() {
                            self.to_edit.push(row_index);
                        }
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::DELETE).clicked() {
                            self.to_delete = Some(row_index);
                        }
                    });
                });
            });
        if self.to_edit.len() > 0 {
            let tilepos = *self.to_edit.first().unwrap();
            match edit_tile(ctx, &mut self.entries[tilepos], &self.to_edit) {
                TileAction::Edit(pos) => self.to_edit.push(pos),
                TileAction::Delete(pos) => self.to_delete = Some(pos),
                TileAction::Close => {
                    self.to_edit.pop();
                }
                TileAction::Nothing => {}
            }
            if let Some(varpos) = self.to_delete
                && let Some(okay) = confirm(ctx, "Are you sure?")
            {
                if okay {
                    self.entries[tilepos].var.remove(varpos);
                }
                self.to_delete = None;
            }
        } else if let Some(pos) = self.to_delete
            && let Some(okay) = confirm(ctx, "Are you sure?")
        {
            if okay {
                self.entries.remove(pos);
            }
            self.to_delete = None;
        }
    }
}

fn edit_tile(ctx: &egui::Context, tile: &mut Tile, edit: &Vec<usize>) -> TileAction {
    let mut action = TileAction::Nothing;
    if let Some(varid) = edit.get(1) {
        let var = tile.var.get_mut(*varid).expect("Invalid variant index");
        action = edit_var(ctx, var, edit)
    } else {
        egui::Modal::new(egui::Id::new("edit_tile")).show(ctx, |ui| {
            ui.with_layout(Layout::right_to_left(egui::Align::LEFT), |ui| {
                if ui.small_button("X").clicked() {
                    action = TileAction::Close;
                }
            });
            ui.vertical(|ui| {
                let table = TableBuilder::new(ui)
                    .column(Column::initial(100.0))
                    .column(Column::remainder());
                table.body(|mut body| {
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("ID");
                        });
                        row.col(|ui| {
                            ui.add(egui::DragValue::new(&mut tile.id).range(0..=5000));
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Name or Item ID");
                        });
                        row.col(|ui| {
                            let mut name = String::new();
                            if let Some(r) = tile.reference {
                                name = r.to_string();
                            } else if let Some(n) = tile.name.as_ref() {
                                name = n.to_owned();
                            }
                            ui.text_edit_singleline(&mut name);
                            match name.parse::<i32>() {
                                Ok(r) => {
                                    tile.reference = Some(r);
                                    tile.name = None;
                                }
                                Err(_) => {
                                    tile.reference = None;
                                    tile.name = (!name.is_empty()).then_some(name);
                                }
                            }
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Color");
                        });
                        row.col(|ui| {
                            ui.horizontal(|ui| {
                                let mut present = tile.color.is_some();
                                ui.checkbox(&mut present, "");
                                if present {
                                    let mut c = if let Some(c) = tile.color.as_ref() {
                                        color::string_to_color(c)
                                    } else {
                                        [0.0, 0.0, 0.0]
                                    };
                                    ui.color_edit_button_rgb(&mut c);
                                    tile.color = Some(color::color_to_string(c));
                                } else {
                                    tile.color = None;
                                }
                            });
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Flags");
                        });
                        row.col(|ui| {
                            ui.horizontal_wrapped(|ui| {
                                let mut solid = tile.flags & tile_is::SOLID != 0;
                                ui.checkbox(&mut solid, "Solid");
                                let mut transparent = tile.flags & tile_is::TRANSPARENT != 0;
                                ui.checkbox(&mut transparent, "Transparent");
                                let mut dirt = tile.flags & tile_is::DIRT != 0;
                                ui.checkbox(&mut dirt, "Dirt");
                                let mut stone = tile.flags & tile_is::STONE != 0;
                                ui.checkbox(&mut stone, "Stone");
                                let mut grass = tile.flags & tile_is::GRASS != 0;
                                ui.checkbox(&mut grass, "Grass");
                                let mut pile = tile.flags & tile_is::PILE != 0;
                                ui.checkbox(&mut pile, "Pile");
                                let mut flip = tile.flags & tile_is::FLIP != 0;
                                ui.checkbox(&mut flip, "Flip");
                                let mut brick = tile.flags & tile_is::BRICK != 0;
                                ui.checkbox(&mut brick, "Brick");
                                let mut moss = tile.flags & tile_is::MOSS != 0;
                                ui.checkbox(&mut moss, "Moss");
                                let mut merge = tile.flags & tile_is::MERGE != 0;
                                ui.checkbox(&mut merge, "Merge");
                                let mut large = tile.flags & tile_is::LARGE != 0;
                                ui.checkbox(&mut large, "Large");

                                tile.flags = (solid as u16 * tile_is::SOLID)
                                    | (transparent as u16 * tile_is::TRANSPARENT)
                                    | (dirt as u16 * tile_is::DIRT)
                                    | (stone as u16 * tile_is::STONE)
                                    | (grass as u16 * tile_is::GRASS)
                                    | (pile as u16 * tile_is::PILE)
                                    | (flip as u16 * tile_is::FLIP)
                                    | (brick as u16 * tile_is::BRICK)
                                    | (moss as u16 * tile_is::MOSS)
                                    | (merge as u16 * tile_is::MERGE)
                                    | (large as u16 * tile_is::LARGE);
                            });
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Merge");
                        });
                        row.col(|ui| {
                            ui.text_edit_singleline(&mut tile.merge);
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Blend");
                        });
                        row.col(|ui| {
                            ui.text_edit_singleline(&mut tile.blend);
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Width");
                        });
                        row.col(|ui| {
                            ui.add(egui::DragValue::new(&mut tile.w).range(18..=56));
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Height");
                        });
                        row.col(|ui| {
                            ui.add(egui::DragValue::new(&mut tile.h).range(18..=56));
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Skip Y");
                        });
                        row.col(|ui| {
                            ui.add(egui::DragValue::new(&mut tile.skipy).range(0..=4));
                        });
                    });
                    body.row(18.0, |mut row| {
                        row.col(|ui| {
                            ui.label("Top Pad");
                        });
                        row.col(|ui| {
                            ui.add(egui::DragValue::new(&mut tile.toppad).range(-32..=32));
                        });
                    });
                });

                if action == TileAction::Nothing {
                    let mut scroll_to = None;
                    ui.horizontal(|ui| {
                        if ui
                            .button(formatcp!("{} Add new variant", icons::ADD))
                            .clicked()
                        {
                            tile.var.push(TileVariant::default());
                            scroll_to = Some(tile.var.len() - 1);
                        }
                    });
                    action = show_vars(ui, &tile.var, scroll_to);
                }
            });
        });
    }
    action
}

fn show_vars(ui: &mut egui::Ui, vars: &Vec<TileVariant>, scroll_to: Option<usize>) -> TileAction {
    let mut action = TileAction::Nothing;
    if vars.len() > 0 {
        let mut vartable = TableBuilder::new(ui)
            .id_salt(egui::Id::new("variant"))
            .column(Column::initial(50.0))
            .column(Column::initial(50.0))
            .column(Column::initial(200.0))
            .column(Column::initial(50.0))
            .column(Column::auto())
            .column(Column::auto());
        if let Some(to) = scroll_to {
            vartable = vartable.scroll_to_row(to, None);
        }
        vartable
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.strong("X");
                });
                header.col(|ui| {
                    ui.strong("Y");
                });
                header.col(|ui| {
                    ui.strong("Name");
                });
                header.col(|ui| {
                    ui.strong("Color");
                });
                header.col(|ui| {
                    ui.strong("Edit");
                });
                header.col(|ui| {
                    ui.strong("Delete");
                });
            })
            .body(|body| {
                body.rows(18.0, vars.len(), |mut row| {
                    let row_index = row.index();
                    row.col(|ui| {
                        let star = if vars[row_index].var.len() > 0 {
                            "*"
                        } else {
                            ""
                        };
                        if let Some(x) = vars[row_index].x {
                            ui.label(format!("{}{}", star, x));
                        } else if let Some(x) = vars[row_index].minx {
                            ui.label(format!("{}>{}", star, x));
                        } else if let Some(x) = vars[row_index].maxx {
                            ui.label(format!("{}<{}", star, x));
                        } else {
                            ui.label(format!("{}-", star));
                        }
                    });
                    row.col(|ui| {
                        if let Some(y) = vars[row_index].y {
                            ui.label(y.to_string());
                        } else if let Some(y) = vars[row_index].miny {
                            ui.label(format!(">{}", y));
                        } else if let Some(y) = vars[row_index].maxy {
                            ui.label(format!("<{}", y));
                        } else {
                            ui.label("-");
                        }
                    });
                    row.col(|ui| {
                        if let Some(r) = vars[row_index].reference {
                            ui.label(format!("{}", r));
                        } else if let Some(n) = vars[row_index].name.as_ref() {
                            ui.label(n);
                        } else {
                            ui.label("-");
                        }
                    });
                    row.col(|ui| {
                        if let Some(c) = vars[row_index].color.as_ref() {
                            let c = color::string_to_color32(c);
                            let (rect, _) =
                                ui.allocate_exact_size(ui.spacing().interact_size, Sense::hover());
                            ui.painter_at(rect).rect_filled(rect, 0, c);
                        } else {
                            ui.label("-");
                        }
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::EDIT).clicked() {
                            action = TileAction::Edit(row_index);
                        }
                    });
                    row.col(|ui| {
                        if ui.small_button(icons::DELETE).clicked() {
                            action = TileAction::Delete(row_index);
                        }
                    });
                });
            });
    }
    action
}

fn edit_var(ctx: &egui::Context, mut var: &mut TileVariant, edit: &Vec<usize>) -> TileAction {
    let mut action = TileAction::Nothing;

    for id in edit.iter().skip(2) {
        var = var.var.get_mut(*id).expect("Invalid variant index");
    }

    egui::Modal::new(egui::Id::new("edit_var")).show(ctx, |ui| {
        ui.vertical(|ui| {
            ui.with_layout(Layout::right_to_left(egui::Align::LEFT), |ui| {
                if ui.small_button("X").clicked() {
                    action = TileAction::Close;
                }
            });
            let table = TableBuilder::new(ui)
                .column(Column::initial(100.0))
                .column(Column::remainder());
            table.body(|mut body| {
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("X");
                    });
                    row.col(|ui| {
                        let mut xtxt = String::new();
                        if let Some(x) = var.x {
                            xtxt = x.to_string();
                        } else if let Some(x) = var.minx {
                            xtxt = format!(">{}", x)
                        } else if let Some(x) = var.maxx {
                            xtxt = format!("<{}", x)
                        }
                        ui.text_edit_singleline(&mut xtxt);
                        if xtxt.starts_with("<") {
                            var.x = None;
                            var.minx = None;
                            xtxt.remove(0);
                            var.maxx = xtxt.parse::<i32>().ok();
                        } else if xtxt.starts_with(">") {
                            var.x = None;
                            var.maxx = None;
                            xtxt.remove(0);
                            var.minx = xtxt.parse::<i32>().ok();
                        } else {
                            var.minx = None;
                            var.maxx = None;
                            var.x = xtxt.parse::<i32>().ok();
                        }
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Y");
                    });
                    row.col(|ui| {
                        let mut ytxt = String::new();
                        if let Some(y) = var.y {
                            ytxt = y.to_string();
                        } else if let Some(y) = var.miny {
                            ytxt = format!(">{}", y)
                        } else if let Some(y) = var.maxy {
                            ytxt = format!("<{}", y)
                        }
                        ui.text_edit_singleline(&mut ytxt);
                        if ytxt.starts_with("<") {
                            var.y = None;
                            var.miny = None;
                            ytxt.remove(0);
                            var.maxy = ytxt.parse::<i32>().ok();
                        } else if ytxt.starts_with(">") {
                            var.y = None;
                            var.maxy = None;
                            ytxt.remove(0);
                            var.miny = ytxt.parse::<i32>().ok();
                        } else {
                            var.miny = None;
                            var.maxy = None;
                            var.y = ytxt.parse::<i32>().ok();
                        }
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Name or Item ID");
                    });
                    row.col(|ui| {
                        let mut name = String::new();
                        if let Some(r) = var.reference {
                            name = r.to_string();
                        } else if let Some(n) = var.name.as_ref() {
                            name = n.to_owned();
                        }
                        ui.text_edit_singleline(&mut name);
                        match name.parse::<i32>() {
                            Ok(r) => {
                                var.reference = Some(r);
                                var.name = None;
                            }
                            Err(_) => {
                                var.reference = None;
                                var.name = (!name.is_empty()).then_some(name);
                            }
                        }
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Color");
                    });
                    row.col(|ui| {
                        ui.horizontal(|ui| {
                            let mut present = var.color.is_some();
                            ui.checkbox(&mut present, "");
                            if present {
                                let mut c = if let Some(c) = var.color.as_ref() {
                                    color::string_to_color(c)
                                } else {
                                    [0.0, 0.0, 0.0]
                                };
                                ui.color_edit_button_rgb(&mut c);
                                var.color = Some(color::color_to_string(c));
                            } else {
                                var.color = None;
                            }
                        });
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Width");
                    });
                    row.col(|ui| {
                        ui.add(egui::DragValue::new(&mut var.w).range(18..=56));
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Height");
                    });
                    row.col(|ui| {
                        ui.add(egui::DragValue::new(&mut var.h).range(18..=56));
                    });
                });
                body.row(18.0, |mut row| {
                    row.col(|ui| {
                        ui.label("Top Pad");
                    });
                    row.col(|ui| {
                        ui.add(egui::DragValue::new(&mut var.toppad).range(-32..=32));
                    });
                });
            });

            if action == TileAction::Nothing {
                let mut scroll_to = None;
                ui.horizontal(|ui| {
                    if ui
                        .button(formatcp!("{} Add new variant", icons::ADD))
                        .clicked()
                    {
                        var.var.push(TileVariant::default());
                        scroll_to = Some(var.var.len() - 1);
                    }
                });
                action = show_vars(ui, &var.var, scroll_to);
            }
        });
    });
    action
}
