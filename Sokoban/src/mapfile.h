#ifndef MAPFILE_H
#define MAPFILE_H

#include <fstream>
#include <filesystem>

#include "point.h"

class ColorCycle;

enum class MapCode;
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
