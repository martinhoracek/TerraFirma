use egui::Id;

pub fn confirm(ctx: &egui::Context, msg: &str) -> Option<bool> {
    let mut resp = None;
    egui::Modal::new(Id::new("confirmation")).show(ctx, |ui| {
        ui.vertical(|ui| {
            ui.label(msg);
            ui.horizontal(|ui| {
                if ui.button("No").clicked() {
                    resp = Some(false);
                }
                if ui.button("Yes").clicked() {
                    resp = Some(true);
                }
            })
        })
    });
    if ctx.input_mut(|i| i.consume_key(egui::Modifiers::NONE, egui::Key::Escape)) {
        resp = Some(false);
    }
    if ctx.input_mut(|i| i.consume_key(egui::Modifiers::NONE, egui::Key::Enter)) {
        resp = Some(true);
    }

    resp
}
