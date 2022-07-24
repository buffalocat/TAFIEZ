#ifndef COMMON_ENUMS_H
#define COMMON_ENUMS_H

enum class Direction {
	NONE = 0,
	Left = 1,
	Far = 2,
	Right = 3,
	Close = 4,
	Up = 5,
	Down = 6,
};

Direction point_to_dir(Point3 p);

Point3 dir_to_point(Direction dir);

enum class ObjCode {
    NONE = 0,
    PushBlock = 1,
    SnakeBlock = 2,
    Wall = 3,
    Player = 4,
    GateBody = 5,
	ArtWall = 6,
	Null = 255,
};

enum class ModCode {
	NONE = 0,
	Door = 1,
	Car = 2,
	PressSwitch = 3,
	Gate = 4,
	AutoBlock = 5,
	PuppetBlock = 6,
	ClearFlag = 7,
	// 8 Unused
	PermanentSwitch = 9,
	FloorSign = 10,
	Incinerator = 11,
	FlagGate = 12,
	FlagSwitch = 13,
	MapDisplay = 14,
	AutosavePanel = 15,
};

// This is useless for now, but we'll keep it anyway
enum class CameraCode {
    NONE = 0,
    General = 1,
	// Radial?
};

enum class Sticky {
    None = 0,
	// Stickiness checks
	WeakStick = 1 | 2,
	StrongStick = 4,
	Weak = 1 | 2 | 8,
	Strong = 4 | 16,
	// Block levels
    WeakBlock = 1,
	SemiBlock = 1 | 2,
    StrongBlock = 2 | 4,
	// Snakes
	WeakSnake = 8,
	StrongSnake = 8 | 16,
};

enum class CauseOfDeath {
	None = 0,
	Fallen = 1,
	Split = 2,
	Incinerated = 3,
	Voided = 4,
	Collided = 5,
};

Sticky operator &(Sticky a, Sticky b);

#endif // COMMON_ENUMS_H
