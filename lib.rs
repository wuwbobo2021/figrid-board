mod coord;
mod evaluator;
mod rec;
mod rec_base;
mod rec_checker;
mod row;
mod rule;
mod tree;

pub use coord::*;
pub use evaluator::*;
pub use rec::*;
pub use rec_base::*;
pub use rec_checker::*;
pub use row::*;
pub use rule::*;
pub use tree::*;

/// Possible errors returned by `figrid_board`.
#[derive(Clone, Debug, PartialEq)]
#[repr(u8)]
pub enum Error {
    ParseError,
    InvalidCoord,
    CoordNotEmpty,
    RecIsEmpty,
    RecIsFull,
    RecIsFinished,
    ItemNotExist,
    TransformFailed,
    CursorAtEnd,
}
