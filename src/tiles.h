/** @copyright 2025 Sean Kasun */

#pragma once

#include "handle.h"
#include <memory>
#include <vector>

enum TileType {
  TileBlend = -2,
  TileAir = -1,
  TileDirt = 0,
  TileStone = 1,
  TileGrass = 2,
  TileTorches = 4,
  TileTrees = 5,
  TilePlatforms = 19,
  TileCorruptGrass = 23,
  TileChandeliers = 34,
  TileLamps = 42,
  TileCobweb = 51,
  TileSand = 53,
  TileGlass = 54,
  TileJungleGrass = 60,
  TileMushroomGrass = 70,
  TileMushroom = 72,
  TileCactus = 80,
  TileBanners = 91,
  TileChineseLantern = 95,
  TileStatues = 105,
  TileHallowGrass = 109,
  TileEbonSand = 112,
  TileTinker = 114,
  TilePearlSand = 116,
  TileDiscoBall = 126,
  TileCrystals = 129,
  TileSwitches = 136,
  TileSnow = 147,
  TileXmasTree = 171,
  TileMoss = 184,
  TileChunks = 185,
  TileCrimsonGrass = 199,
  TileRope = 213,
  TileFlower = 227,
  TileCrimSand = 234,
  TileLizhardAltar = 237,
  TileFirefly = 270,
  TileLightningBug = 271,
  TilePlating = 272,
  TileTrack = 314,
  TilePalm = 323,
  TileWeaponRack = 334,
  TileSmoothMarble = 357,
  TileWaterDrop = 373,
  TileLavaDrop = 374,
  TileHoneyDrop = 375,
  TilePlanters = 380,
  TileTrapDoor = 386,
  TileTrapDoorClose = 387,
  TileItemFrame = 395,
  TileManipulator = 412,
  TileConveyorR = 421,
  TileConveyorL = 422,
  TileJunction = 424,
  TilePixel = 445,
  TileTealPressure = 442,
  TileBeehive = 444,
  TilePigronata = 454,
  TileSandDrop = 461,
  TileWarBanner = 465,
  TileMannequin = 470,
  TileMowed = 477,
  TileHallowMowed = 492,
  TileSoulBottle = 572,
  TileLavafly = 581,
  TileTopazTree = 583,
  TileAmethystTree = 584,
  TileSapphireTree = 585,
  TileEmeraldTree = 586,
  TileRubyTree = 587,
  TileDiamondTree = 588,
  TileAmberTree = 589,
  TileHangingPots = 591,
  TileHangingBrazier = 592,
  TileSakuraTree = 596,
  TilePylon = 597,
  TileWillowTree = 616,
  TileMasterTrophies = 617,
  TileAshTree = 634,
  TileFaeling = 660,
  TileCorruptJungle = 661,
  TileCrimsonJungle = 662,
};

enum Is : uint16_t {
  IsAir        = 0x0000,
  IsActive     = 0x0001,
  IsLava       = 0x0002,
  IsHoney      = 0x0004,
  IsShimmer    = 0x0008,
  IsRedWire    = 0x0010,
  IsBlueWire   = 0x0020,
  IsGreenWire  = 0x0040,
  IsYellowWire = 0x0080,
  IsActuator   = 0x0100,
  IsInactive   = 0x0200,
  IsHalf       = 0x1000,
  IsSeen       = 0x8000,
};

struct TileFlags1 {
  bool hasFlags2 : 1;  // 01
  bool active : 1;  // 02
  bool wall : 1;  // 04
  bool water : 1;  // 08
  bool lava : 1;  // 10  (water & lava = honey)
  bool tile16 : 1;  // 20
  uint8_t rle : 2; // c0
};
struct TileFlags2 {
  bool hasFlags3 : 1;  // 01
  bool redWire : 1;  // 02
  bool blueWire : 1;  // 04
  bool greenWire : 1;  // 08
  uint8_t slope : 4;  // f0
};
struct TileFlags3 {
  bool hasFlags4 : 1;  // 01
  bool actuator : 1;  // 02
  bool inactive : 1;  // 04
  bool paint : 1;  // 08
  bool wallPaint : 1;  // 10
  bool yellowWire : 1;  // 20
  bool wall16 : 1;  // 40
  bool shimmer : 1;  // 80
};
struct TileFlags4 {
  bool unused : 1;  // 01 (will probably be hasFlags5)
  bool transparent : 1;  // 02 (lets light through)
  bool transparentWall : 1;  // 04
  bool glowing : 1;  // 08 (emits light)
  bool glowingWall : 1;  // 10
};

class Tile {
  public:
    int16_t u, v, wallu, wallv, type, wall;
    uint8_t liquid, paint, wallPaint, slope;
    int load(std::shared_ptr<Handle> handle, const std::vector<bool> &extra);
    uint16_t Is() const;
    bool active() const;
    bool lava() const;
    bool honey() const;
    bool shimmer() const;
    bool seen() const;
    void setSeen(bool seen);
    bool half() const;
    bool actuator() const;
    bool inactive() const;

  private:
    uint16_t is;
};
