/** @copyright 2025 Sean Kasun */

#include "tiles.h"
#include <bit>

int Tile::load(std::shared_ptr<Handle> handle, const std::vector<bool> &extra) {
  TileFlags1 flags1 = std::bit_cast<TileFlags1>(handle->r8());
  TileFlags2 flags2 = std::bit_cast<TileFlags2>(flags1.hasFlags2 ? handle->r8() : static_cast<uint8_t>(0));
  TileFlags3 flags3 = std::bit_cast<TileFlags3>(flags2.hasFlags3 ? handle->r8() : static_cast<uint8_t>(0));
  TileFlags4 flags4 = std::bit_cast<TileFlags4>(flags3.hasFlags4 ? handle->r8() : static_cast<uint8_t>(0));

  if (flags1.active) {
    is = IsActive;
    type = handle->r8();
    if (flags1.tile16) {
      type |= handle->r8() << 8;
    }
    if (extra[type]) {
      u = handle->r16();
      v = handle->r16();
    } else {
      u = v = -1;
    }
    if (flags3.paint) {
      paint = handle->r8();
    }
  }

  if (flags1.wall) {
    wall = handle->r8();
    // 16-bit wall is set below
    if (flags3.wallPaint) {
      wallPaint = handle->r8();
    }
    wallu = wallv = -1;
  }

  if (flags1.water || flags1.lava) {
    liquid = handle->r8();
    if (flags1.water && flags1.lava) {
      is |= IsHoney;
    } else if (flags1.lava) {
      is |= IsLava;
    }
    if (flags3.shimmer) {
      is |= IsShimmer;
    }
  }

  if (flags2.redWire) {
    is |= IsRedWire;
  }
  if (flags2.blueWire) {
    is |= IsBlueWire;
  }
  if (flags2.greenWire) {
    is |= IsGreenWire;
  }
  if (flags3.yellowWire) {
    is |= IsYellowWire;
  }
  if (flags2.slope > 1) {
    slope = flags2.slope - 1;
  } else if (flags2.slope == 1) {
    is |= IsHalf;
  }

  if (flags3.actuator) {
    is |= IsActuator;
  }
  if (flags3.inactive) {
    is |= IsInactive;
  }
  if (flags3.wall16) {
    // has to be after liquid since we read a byte
    wall |= handle->r8() << 8;
  }

  switch (flags1.rle) {
    case 1:
      return handle->r8();
    case 2:
      return handle->r16();
  }
  return 0;
}

void Tile::setSeen(bool seen) {
  if (seen) {
    is |= IsSeen;
  } else {
    is &= ~IsSeen;
  }
}

uint16_t Tile::Is() const {
  return is;
}

bool Tile::active() const {
  return is & IsActive;
}

bool Tile::inactive() const {
  return is & IsInactive;
}

bool Tile::half() const {
  return is & IsHalf;
}

bool Tile::lava() const {
  return is & IsLava;
}

bool Tile::honey() const {
  return is & IsHoney;
}

bool Tile::shimmer() const {
  return is & IsShimmer;
}

bool Tile::actuator() const {
  return is & IsActuator;
}
