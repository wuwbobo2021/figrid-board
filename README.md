# figrid-board

A library for the Five-in-a-Row (Gomoku) game.

This crate is looking for new owners interested in this game.

Notes:

- The initial Rust version suffered in critical bugs and got the lowest ranking in Gomocup 2025. Some bug fixes were made since then.

- To build the engine for the current CPU, use `RUSTFLAGS="-C target-cpu=native" cargo build --release`. Currently, advanced CPU features are not used explicitly in the code.

- In case of someone still needs the previous `figrid-board` as an Linux alternative of Renlib, download the [tag v0.20 in the Github repository](https://github.com/wuwbobo2021/figrid-board/releases/tag/v0.20).
