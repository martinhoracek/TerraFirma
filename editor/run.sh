#!/bin/sh

unset WAYLAND_DISPLAY
export XDG_SESSION_TYPE=x11
cargo run
