use std::{
	ops::Index,
	str::FromStr,
	fmt::Display
};

use crate::{
	Coord,
	CoordState,
	Rec, //trait
	Error
};

/// Basic record structure without any evaluator or rule checker.
/// `SZ` is the board size, `LEN` is the maximum capacity (it should be more than `SZ*SZ`).
#[derive(Clone, PartialEq, Eq, Debug)]
pub struct BaseRec<const SZ: usize, const LEN: usize> {
	len: usize,
	cnt_pass: u8,
	seq: [Coord<SZ>; LEN],
	grid: [[CoordState; SZ]; SZ], //array of vertical colomns, use it with [x][y]
}

pub type BaseRec15 = BaseRec<15, {15*15 + 16}>; //maximum 16 pass moves
pub type BaseRec19 = BaseRec<19, {19*19 + 16}>;

impl<const SZ: usize, const LEN: usize> BaseRec<SZ, LEN> {
	const SZ_X_SZ: usize = SZ*SZ;
	
	/// Returns an empty record.
	#[inline]
	pub fn new() -> Self {
		Self {
			len: 0,
			cnt_pass: 0u8,
			seq: [Coord::new(); LEN],
			grid: [[CoordState::Empty; SZ]; SZ],
		}
	}
}

impl<const SZ: usize, const LEN: usize> Rec<SZ> for BaseRec<SZ, LEN> {
	#[inline(always)]
	fn as_slice(&self) -> &[Coord<SZ>] {
		&self.seq[..self.len]
	}
	
	#[inline(always)]
	fn coord_state(&self, coord: Coord<SZ>) -> CoordState {
		if coord.is_real() {
			// SAFETY: assumes coord is valid
			unsafe { self.coord_state_unchecked(coord) }
		} else {
			CoordState::Empty
		}
	}
	
	#[inline(always)]
	unsafe fn coord_state_unchecked(&self, coord: Coord<SZ>) -> CoordState {
		// SAFETY: assumes coord is NOT null
		unsafe {
			let (x, y) = coord.get_unchecked();
			*((self.grid.get_unchecked(x as usize) as &[CoordState; SZ])
				.get_unchecked(y as usize))
		}
	}
	
	#[inline(always)]
	fn len(&self) -> usize {
		self.len
	}

	#[inline(always)]
	fn len_max() -> usize {
		LEN
	}
	
	fn stones_count(&self) -> usize {
		self.len - self.cnt_pass as usize
	}
	
	#[inline(always)]
	fn is_full(&self) -> bool {
		// `>` should be impossible
		self.len >= LEN || self.stones_count() >= Self::SZ_X_SZ
	}
	
	#[inline(always)]
	fn add(&mut self, coord: Coord<SZ>) -> Result<(), Error> {
		if self.len >= LEN {
			return Err(Error::RecIsFull);
		}
		if self.coord_state(coord) != CoordState::Empty {
			return Err(Error::CoordNotEmpty);
		}
		
		if coord.is_real() {
			*(self.coord_state_mut(coord)?) = self.color_next();
		} else {
			self.cnt_pass += 1;
		}
		// SAFETY: assumes self.len is valid
		* unsafe {
			self.seq.get_unchecked_mut(self.len)
		} = coord;
		self.len += 1;
		Ok(())
	}
	
	#[inline(always)]
	fn undo(&mut self) -> Result<Coord<SZ>, Error> {
		let c = self.last_coord().ok_or(Error::RecIsEmpty)?;
		self.len -= 1;
		if let Ok(mut_stat) = self.coord_state_mut(c) {
			*mut_stat = CoordState::Empty;
		} else {
			self.cnt_pass -= 1;
		}
		Ok(c)
	}
	
	#[inline(always)]
	fn clear(&mut self) {
		self.len = 0;
		self.cnt_pass = 0;
		self.grid = [[CoordState::Empty; SZ]; SZ];
	}
}

impl<const SZ: usize, const LEN: usize> BaseRec<SZ, LEN> {
	#[inline(always)]
	fn coord_state_mut(&mut self, coord: Coord<SZ>) -> Result<&mut CoordState, Error> {
		if coord.is_real() {
			// SAFETY: assumes coord is valid
			Ok(unsafe { self.coord_state_mut_unchecked(coord) })
		} else {
			Err(Error::InvalidCoord)
		}
	}
	
	#[inline(always)]
	unsafe fn coord_state_mut_unchecked(&mut self, coord: Coord<SZ>) -> &mut CoordState {
		// SAFETY: assumes coord is NOT null
		unsafe {
			let (x, y) = coord.get_unchecked();
			(self.grid.get_unchecked_mut(x as usize) as &mut [CoordState; SZ])
				.get_unchecked_mut(y as usize)
		}
	}
}

// Note: these traits can't be implemented automatically for all types with trait Record<SZ>

impl<const SZ: usize, const LEN: usize> FromStr for BaseRec<SZ, LEN> {
	type Err = Error;
	fn from_str(s: &str) -> Result<Self, Self::Err> {
		let mut rec = Self::new();
		rec.append_str(&s).map_err(|_| Error::ParseError)?;
		Ok(rec)
	}
}

impl<const SZ: usize, const LEN: usize> Index<usize> for BaseRec<SZ, LEN> {
    type Output = Coord<SZ>;

	#[inline(always)]
    fn index(&self, i: usize) -> &Coord<SZ> {
        &self.as_slice()[i]
    }
}

impl<const SZ: usize, const LEN: usize> Display for BaseRec<SZ, LEN> {
	#[inline(always)]
	fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
		self.print_str(f, " ", true)
	}
}
