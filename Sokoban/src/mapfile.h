#ifndef MAPFILE_H
#define MAPFILE_H

#include "point.h"

enum class MapCode {
	Dimensions = 1, // The dimensions of the room as 1 byte integers
	FullLayer = 2, // Create a new full layer
	SparseLayer = 3, // Create a new sparse layer
	WallRuns = 4, // A (relatively) efficient encoding of walls
	OffsetPos = 5, // The position that (0,0) was at when the room was created
	Objects = 6, // Read in all map objects
	CameraRects = 7, // Get a camera context rectangle
	SnakeLink = 8, // Link two snakes (1 = Right, 2 = Down)
	DoorDest = 9, // Give a door a destination Map + Pos
	Signaler = 10, // List of Switches and Switchables linked to a Signaler
	UNUSED_11 = 11, // Formerly for walls, now followed by 0
	UNUSED_12 = 12, // Formerly for the player, now nothing
	GateBodyLocation = 13, // Indicates that a GateBody needs to be paired with its parent
	WallPositions = 14, // Initiates list of Wall Positions
	End = 255,
};

class ColorCycle;

enum class ObjCode;
enum class ModCode;
enum class CameraCode;
enum class Sticky;
enum class RidingState;

class MapFileI {
public:
    MapFileI(const std::filesystem::path& path);
    ~MapFileI();
    void read(unsigned char* b, int n);

    unsigned char read_byte();
	unsigned int read_uint32();
    Point3 read_point3();
    std::string read_str();

private:
    std::ifstream file_;
};

MapFileI& operator>>(MapFileI& f, int& v);
MapFileI& operator>>(MapFileI& f, bool& v);
MapFileI& operator>>(MapFileI& f, double& v);

MapFileI& operator>>(MapFileI& f, Point2& v);
MapFileI& operator>>(MapFileI& f, Point3& v);
MapFileI& operator>>(MapFileI& f, Point3_S16& v);
MapFileI& operator>>(MapFileI& f, FPoint3& v);
MapFileI& operator>>(MapFileI& f, IntRect& v);
MapFileI& operator>>(MapFileI& f, FloatRect& v);

MapFileI& operator>>(MapFileI& f, ColorCycle& v);



class MapFileO {
public:
    MapFileO(const std::filesystem::path& path);
    ~MapFileO();

	void write_uint32(unsigned int);

    MapFileO& operator<<(unsigned char);
    MapFileO& operator<<(int);
    MapFileO& operator<<(unsigned int);
    MapFileO& operator<<(bool);
    MapFileO& operator<<(double);

    MapFileO& operator<<(Point2);
    MapFileO& operator<<(Point3);
    MapFileO& operator<<(Point3_S16);
    MapFileO& operator<<(FPoint3);
	MapFileO& operator<<(IntRect);
	MapFileO& operator<<(FloatRect);

    MapFileO& operator<<(const std::string&);
    MapFileO& operator<<(const ColorCycle&);

    MapFileO& operator<<(MapCode);
    MapFileO& operator<<(ObjCode);
    MapFileO& operator<<(ModCode);
    MapFileO& operator<<(CameraCode);
    MapFileO& operator<<(Sticky);
    MapFileO& operator<<(RidingState);

private:
    std::ofstream file_;
};

#endif // MAPFILE_H
