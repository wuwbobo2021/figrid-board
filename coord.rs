use std::{
	fmt::Display
};

use crate::Error;

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
#[repr(u8)]
pub enum CoordState {
	Empty, Black, White
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct Coord<const SZ: usize> {
	x: u8, y: u8
}

pub type Coord15 = Coord<15>;
pub type Coord19 = Coord<19>;

impl<const SZ: usize> Coord<SZ> {
	const SIZE: u8 = if SZ >= 5 && SZ <= 26 {
		SZ as u8
	} else {
		panic!()
	};
	pub const COORD_NULL: Self = Self { x: 0xff, y: 0xff };
	
	#[inline(always)]
	pub fn new() -> Self {
		Self::COORD_NULL
	}
	
	#[inline(always)]
	pub fn from(x: u8, y: u8) -> Self {
		if x < Self::SIZE && y < Self::SIZE {
			Self {x, y}
		} else {
			Self::COORD_NULL
		}
	}
	
	#[inline(always)]
	pub(crate) unsafe fn build_unchecked(x: u8, y: u8) -> Self {
		Self {x, y}
	}
	
	#[inline(always)] pub fn is_null(&self) -> bool {
		*self == Self::COORD_NULL
	}
	#[inline(always)] pub fn is_real(&self) -> bool {
		! self.is_null()
	}
	#[inline(always)] pub fn x(&self) -> u8 { self.x }
	#[inline(always)] pub fn y(&self) -> u8 { self.y }
	
	#[inline(always)]
	pub fn set(&mut self, x: u8, y: u8) -> Result<(), Error> {
		if x < Self::SIZE && y < Self::SIZE {
			self.x = x; self.y = y;
			Ok(())
		} else {
			Err(Error::InvalidCoord)
		}
	}
	
	#[inline(always)]
	pub fn set_null(&mut self) {
		*self = Self::COORD_NULL;
	}
	
	#[inline(always)]
	#[allow(dead_code)]
	pub(crate) unsafe fn set_unchecked(&mut self, x: u8, y: u8) {
		self.x = x; self.y = y;
	}
}

impl<const SZ: usize> Display for Coord<SZ> {
	#[inline(always)]
	fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
		if self.is_real() {
			write!(f, "{}{}", coord_x_letter(self.x), self.y)
		} else {
			write!(f, "-")
		}
	}
}

pub(crate) fn coord_x_letter(x: u8) -> char {
	if x < 26 {
		// SAFETY: x < 26
		unsafe { char::from_u32_unchecked(('a' as u32) + x as u32) }
	} else {
		'?'
	}
}
