use std::{
	slice,
	ops::Index,
	fmt::{self, Display}
};

use crate::{
	Coord,
	CoordState,
	Rotation,
	Error
};

#[derive(Clone, PartialEq, Eq, Debug)]
pub struct Rec<const SZ: usize, const LEN: usize> {
	len: usize,
	cnt_pass: u8,
	seq: [Coord<SZ>; LEN],
	grid: [[CoordState; SZ]; SZ], //array of vertical colomns, use it with [x][y]
	out_fmt: (String, bool) //divider, print_no
}

pub type Rec15 = Rec<15, {15*15 + 16}>; //maximum 16 pass moves
pub type Rec19 = Rec<19, {19*19 + 16}>;

impl<const SZ: usize, const LEN: usize> Rec<SZ, LEN> {
	#[inline]
	pub fn new() -> Self {
		Self {
			len: 0,
			cnt_pass: 0u8,
			seq: [Coord::new(); LEN],
			grid: [[CoordState::Empty; SZ]; SZ],
			out_fmt: (" ".to_string(), true)
		}
	}
	
	#[inline(always)]
	pub fn as_slice(&self) -> &[Coord<SZ>] {
		&self.seq[..self.len]
	}
	
	#[inline(always)]
	pub fn iter(&self) -> slice::Iter<'_, Coord<SZ>> {
		self.as_slice().iter()
	}
	
	#[inline(always)]
	pub fn coord_state(&self, coord: Coord<SZ>) -> CoordState {
		if coord.is_real() {
			// SAFETY: assumes coord is valid
			unsafe { self.coord_state_unchecked(coord) }
		} else {
			CoordState::Empty
		}
	}
}

impl<const SZ: usize, const LEN: usize> Index<usize> for Rec<SZ, LEN> {
    type Output = Coord<SZ>;

	#[inline(always)]
    fn index(&self, i: usize) -> &Coord<SZ> {
        &self.as_slice()[i]
    }
}

impl<const SZ: usize, const LEN: usize> Rec<SZ, LEN> {
	const SZ_X_SZ: usize = SZ*SZ;
	
	#[inline(always)]
	pub fn len(&self) -> usize {
		self.len
	}
	
	pub fn stones_count(&self) -> usize {
		self.len - self.cnt_pass as usize
	}
	
	#[inline(always)]
	pub fn is_empty(&self) -> bool {
		self.len == 0
	}
	
	#[inline(always)]
	pub fn is_full(&self) -> bool {
		// `>` should be impossible
		self.len >= LEN || self.stones_count() >= Self::SZ_X_SZ
	}
	
	#[inline(always)]
	pub fn last_coord(&self) -> Option<Coord<SZ>> {
		if ! self.is_empty() {
			// SAFETY: assumes self.len is valid
			Some(* unsafe { self.seq.get_unchecked(self.len - 1) })
		} else {
			None
		}
	}
	
	#[inline(always)]
	pub fn color_next(&self) -> CoordState {
		if self.len % 2 == 0 {
			CoordState::Black
		} else {
			CoordState::White
		}
	}
}

impl<const SZ: usize, const LEN: usize> Rec<SZ, LEN> {
	#[inline(always)]
	pub fn add(&mut self, coord: Coord<SZ>) -> Result<(), Error> {
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
	pub fn undo(&mut self) -> Result<Coord<SZ>, Error> {
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
	pub fn back_to(&mut self, coord: Coord<SZ>) -> Result<usize, Error> {
		if let Some(i) = self.iter().rposition(|&c| c == coord) {
			let steps = (self.len - 1) - i;
			for _ in 0..steps {
				let _ = self.undo();
			}
			Ok(steps)
		} else {
			Err(Error::ItemNotExist)
		}
	}
	
	#[inline(always)]
	pub fn clear(&mut self) {
		self.len = 0;
		self.cnt_pass = 0;
		self.grid = [[CoordState::Empty; SZ]; SZ];
	}
	
	#[inline(always)]
	pub fn rotate(&mut self, rotation: Rotation) {
		self.transform(|c| rotation.rotate_coord(c))
	}
	
	pub fn transform(&mut self, f: impl Fn(Coord<SZ>) -> Coord<SZ>) {
		// Note: Vec is used
		let seq = self.iter().map(
			|&c| if c.is_real() { f(c) } else { c }
		).collect::<Vec<_>>();
		
		self.clear();
		for c in seq {
			let _ = self.add(c);
		}
	}
	
	pub fn transform_checked<F>(&mut self, f: F) -> Result<(), Error>
		where F: Fn(Coord<SZ>) -> Option<Coord<SZ>>
	{
		let seq = self.iter().map(
			|&c| if c.is_real() { f(c) } else { Some(c) }
		).collect::<Vec<_>>();
		
		for c in &seq {
			if c.is_none() {
				return Err(Error::TransformFailed);
			}
		}
		self.clear();
		for c in seq {
			self.add(c.unwrap())
				.map_err(|_| Error::TransformFailed)?;
		}
		Ok(())
	}
}

impl<const SZ: usize, const LEN: usize> Rec<SZ, LEN> {
	fn write(&self, f: &mut impl fmt::Write) -> std::fmt::Result {
		let (ref divider, print_no) = self.out_fmt;
		for (i, c) in self.iter().enumerate() {
			if print_no && i % 2 == 0 {
				write!(f, "{}. ", i % 2)?;
			}
			write!(f, "{}{}", c, divider)?;
		}
		Ok(())
	}
	
	pub fn print_board(&self, f: &mut impl fmt::Write, dots: &[Coord<SZ>], full_char: bool)
		-> std::fmt::Result
	{
		#[allow(non_snake_case)] let SIZE: u8 = SZ as u8;
		let ( //actually string literals, not chars
			ch_black, ch_black_last, ch_white, ch_white_last, ch_emp, ch_dot,
			ch_b, ch_u, ch_l, ch_r, ch_bl, ch_br, ch_ul, ch_ur
		) = if full_char {
			("●", "◆", "○", "⊙", "┼", "·",
			 "┴", "┬", "├", "┤", "└", "┘", "┌", "┐")
		} else {
			(" X", " #", " O", " Q", " .", " *",
			 " .", " .", " .", " .", " .", " .", " .", " .")
		};
		
		for y in (0..SIZE).rev() {
			write!(f, " {:2}", y + 1)?;
			for x in 0..SIZE {
				let coord = Coord::from(x, y);
				let st = unsafe {
					// SAFETY: ranges of x and y is correct
					self.coord_state_unchecked(coord)
				};
				let ch = match st {
					CoordState::Empty => {
						if dots.iter().find(|&&c| coord == c)
							.is_some()                         { ch_dot }
						else if x == 0        && y == 0        { ch_bl  }
						else if x == SIZE - 1 && y == 0        { ch_br  }
						else if x == 0        && y == SIZE - 1 { ch_ul  }
						else if x == SIZE - 1 && y == SIZE - 1 { ch_ur  }
						else if x == 0                         { ch_l   }
						else if x == SIZE - 1                  { ch_r   }
						else if                  y == 0        { ch_b   }
						else if                  y == SIZE - 1 { ch_u   }
						else                                   { ch_emp }
					},
					CoordState::Black => {
						if coord != self.last_coord().unwrap_or(Coord::COORD_NULL) {
							ch_black
						} else {
							ch_black_last
						}
					},
					CoordState::White => {
						if coord != self.last_coord().unwrap_or(Coord::COORD_NULL) {
							ch_white
						} else {
							ch_white_last
						}
					}
				};
				write!(f, "{ch}")?;
			}
			write!(f, "\n")?;
		}
		write!(f, "    ")?;
		for h in 0..SIZE {
			write!(f, "{} ", crate::coord_x_letter(h))?;
		}
		write!(f, "\n")?;
		Ok(())
	}
}

impl<const SZ: usize, const LEN: usize> Display for Rec<SZ, LEN> {
	#[inline(always)]
	fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
		self.write(f)
	}
}

impl<const SZ: usize, const LEN: usize> Rec<SZ, LEN> {
	#[inline(always)]
	unsafe fn coord_state_unchecked(&self, coord: Coord<SZ>) -> CoordState {
		unsafe { *(
			(self.grid.get_unchecked(coord.x() as usize) as &[CoordState; SZ])
				.get_unchecked(coord.y() as usize)
		)}
	}
	
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
		// SAFETY: coord is NOT null
		unsafe {
			(self.grid.get_unchecked_mut(coord.x() as usize) as &mut [CoordState; SZ])
				.get_unchecked_mut(coord.y() as usize)
		}
	}
}
