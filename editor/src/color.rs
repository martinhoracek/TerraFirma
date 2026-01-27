use std::i32;

use egui::Color32;

pub fn string_to_color(s: &str) -> [f32; 3] {
    match i32::from_str_radix(s.trim_start_matches("#"), 16) {
        Ok(c) => {
            let r = ((c >> 16) & 0xff) as f32 / 255.0;
            let g = ((c >> 8) & 0xff) as f32 / 255.0;
            let b = (c & 0xff) as f32 / 255.0;
            [r, g, b]
        }
        Err(_) => [0.0, 0.0, 0.0],
    }
}

pub fn string_to_color32(s: &str) -> Color32 {
    match i32::from_str_radix(s.trim_start_matches("#"), 16) {
        Ok(c) => {
            let r = ((c >> 16) & 0xff) as u8;
            let g = ((c >> 8) & 0xff) as u8;
            let b = (c & 0xff) as u8;
            Color32::from_rgb(r, g, b)
        }
        Err(_) => Color32::BLACK,
    }
}

pub fn color_to_string(c: [f32; 3]) -> String {
    format!(
        "#{:02x}{:02x}{:02x}",
        (c[0] * 255.0) as u32,
        (c[1] * 255.0) as u32,
        (c[2] * 255.0) as u32
    )
}
