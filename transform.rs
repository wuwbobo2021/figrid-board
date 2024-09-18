extern crate num_enum;

use crate::Coord;

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
#[derive(num_enum::IntoPrimitive, num_enum::TryFromPrimitive)]
#[repr(u8)]
pub enum Rotation {
	Original,             // 0 00, don't rotate
	Clockwise,            // 0 01, rotate 90 degrees
	CentralSymmetric,     // 0 10, equals rotate 180 degrees clockwise
	Counterclockwise,     // 0 11, equals rotate 270 degrees clockwise
	FlipHorizontal,       // 1 00, reflect by vertical line in the middle
	FlipLeftDiagonal,     // 1 01, equals FlipHorizontal | Clockwise
	FlipVertical,         // 1 10, equals FlipHorizontal | CentralSymmetric
	FlipRightDiagonal     // 1 11, equals FlipHorizontal | Counterclockwise
}

impl Rotation {
	#[inline(always)]
	pub fn rotate_coord<const SZ: usize>(&self, coord: Coord<SZ>) -> Coord<SZ> {
		if coord.is_null() { return coord; }
		
		let bnd = SZ as u8 - 1u8;
		let (ro, fl) = self.ro_fl();
		let (mut x, y) = (coord.x(), coord.y());
		// flip before rotate
		if fl == 1_u8 {
			x = (SZ as u8 - 1u8) - x;
		}
		let (x, y) = match ro {
			0b01_u8 => (y,       bnd - x),
			0b10_u8 => (bnd - y, x      ),
			0b11_u8 => (bnd - x, bnd - y),
			_ => (x, y)
		};
		// SAFETY: assumes the algorithm is correct
		//         and null coord is checked by Rec::transform()
		unsafe { Coord::<SZ>::build_unchecked(x, y) }
	}
	
	#[inline]
	pub fn add(&self, later: Self) -> Self {
		// r1 + r2 = (fl1 + ro1) + (fl2 + ro2) = fl1 + (ro1 + fl2) + ro2
		// fl2 = 0: ro1 + fl2 = fl2 + ro1
		// fl2 = 1: ro1 + fl2 = fl2 + (0b100 - ro1)
		let (ro1, fl1) = self.ro_fl();
		let (ro2, fl2) = later.ro_fl();
		
		let fl = (fl1 + fl2) & 0b1_u8;
		let ro = if fl2 == 1 {
			0b100_u8 - ro1 + ro2
		} else {
			ro1 + ro2
		};
		let ro = ro & 0b11_u8;
		Self::from_ro_fl(ro, fl)
	}

	#[inline]
	pub fn reverse(&self) -> Self {
		let (mut ro, fl) = self.ro_fl();
		if fl == 0_u8 {
			ro = (0b100_u8 - ro) & 0b11_u8;
		}
		Self::from_ro_fl(ro, fl)
	}
	
	#[inline(always)]
	fn ro_fl(&self) -> (u8, u8) {
		let r: u8 = (*self).into();
		(
			(r >> 2_u8) & 0b1_u8,
			r & 0b11_u8
		)
	}
	
	fn from_ro_fl(ro: u8, fl: u8) -> Self {
		use std::convert::TryFrom;
		Rotation::try_from((fl << 2_u8) | ro).unwrap()
	}
}
