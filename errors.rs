#[derive(Clone, Debug)]
#[repr(u8)]
pub enum Error {
	InvalidCoord,
	CoordNotEmpty,
	RecIsEmpty,
	RecIsFull,
	ItemNotExist,
	TransformFailed
}

impl std::fmt::Display for Error {
	fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
		// TODO: better output format
		write!(f, "{:?}", self)
	}
}
