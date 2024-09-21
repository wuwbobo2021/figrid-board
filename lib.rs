mod coord;
mod rec;
mod rec_base;

pub use coord::*;
pub use rec::*;
pub use rec_base::*;

/// Possible errors returned by `figrid_board`.
#[derive(Clone, Debug, PartialEq)]
#[repr(u8)]
pub enum Error {
	ParseError,
	InvalidCoord,
	CoordNotEmpty,
	RecIsEmpty,
	RecIsFull,
	ItemNotExist,
	TransformFailed
}
