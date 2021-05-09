#include "stdafx.h"
#include "mapfile.h"

#include "colorcycle.h"
#include "gameobjectarray.h"

MapFileI::MapFileI(const std::filesystem::path& path): file_ {} {
    file_.open(path, std::ios::in | std::ios::binary);
}

MapFileI::~MapFileI() {
    file_.close();
}


void MapFileI::read(unsigned char* b, int n) {
    file_.read((char *)b, n);
}

unsigned char MapFileI::read_byte() {
    unsigned char b;
    file_.read((char *)&b, 1);
    return b;
}

unsigned int MapFileI::read_uint32() {
	unsigned char b[4];
	file_.read((char *)b, 4);
	return b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
}

int MapFileI::read_int32() {
	unsigned char b[4];
	file_.read((char*)b, 4);
    unsigned int u = b[0] + (b[1] << 8) + (b[2] << 16) + (b[3] << 24);
	return *(int*)(&u);
}

Point3 MapFileI::read_point3() {
    unsigned char b[3];
    read(b, 3);
    return {b[0], b[1], b[2]};
}

Point3 MapFileI::read_spoint3() {
    Point3_S16 p;
    *this >> p;
    return Point3{ p };
}

std::string MapFileI::read_str() {
    unsigned char n;
    char b[256] = "";
    read(&n, 1);
    file_.read(b, n);
    return std::string(b, n);
}

std::string MapFileI::read_long_str() {
	unsigned char n[2];
	read(n, 2);
	unsigned int len = n[0] + (n[1] << 8);
	char b[MAX_LONG_STRING_SIZE] = "";
	file_.read(b, len);
	return std::string(b, len);
}

MapFileI& operator>>(MapFileI& f, int& v) {
    unsigned char b;
    f.read(&b, 1);
    v = b;
    return f;
}

MapFileI& operator>>(MapFileI& f, bool& v) {
	unsigned char b;
	f.read(&b, 1);
	v = b;
	return f;
}

MapFileI& operator>>(MapFileI& f, double& v) {
	unsigned char b[3];
	f.read(b, 3);
	v = (b[0] * (1 << 4) - (1 << 11)) +
		(b[1] / (double)(1 << 4)) +
		(b[2] / (double)(1 << 12));
	return f;
}

MapFileI& operator>>(MapFileI& f, Point2& v) {
    unsigned char b[2];
    f.read(b, 2);
    v = {b[0], b[1]};
    return f;
}

MapFileI& operator>>(MapFileI& f, Point3_S16& v) {
    unsigned char b[6];
    f.read(b, 6);
    v.x = b[0] + (b[1] << 8) - (1 << 15);
    v.y = b[2] + (b[3] << 8) - (1 << 15);
    v.z = b[4] + (b[5] << 8) - (1 << 15);
    return f;
}

MapFileI& operator>>(MapFileI& f, Point3& v) {
    unsigned char b[3];
    f.read(b, 3);
    v = {b[0], b[1], b[2]};
    return f;
}

MapFileI& operator>>(MapFileI& f, FPoint3& v) {
    f >> v.x >> v.y >> v.z;
    return f;
}

MapFileI& operator>>(MapFileI& f, IntRect& r) {
	f >> r.xa >> r.ya >> r.xb >> r.yb;
	return f;
}

MapFileI& operator>>(MapFileI& f, FloatRect& r) {
	f >> r.xa >> r.ya >> r.xb >> r.yb;
	return f;
}

MapFileI& operator>>(MapFileI& f, ColorCycle& v) {
    unsigned char b[2 + MAX_COLOR_CYCLE];
    f.read(b, 2);
    f.read(b+2, b[0]);
    v = ColorCycle{b};
    return f;
}


MapFileIwithObjs::MapFileIwithObjs(const std::filesystem::path& path, GameObjectArray* arr) :
	MapFileI(path), arr_{ arr } {}

MapFileIwithObjs::~MapFileIwithObjs() {}

FrozenObject MapFileIwithObjs::read_frozen_obj() {
	auto ref = static_cast<ObjRefCode>(read_byte());
	auto inacc_id = read_uint32();
    Point3 pos;
	switch (ref) {
    case ObjRefCode::Null:
	case ObjRefCode::Inaccessible:
        break;
	default:
        pos = read_point3();
        break;
	}
    return FrozenObject(pos, ref, inacc_id);
}


MapFileO::MapFileO(const std::filesystem::path& path): file_ {} {
    file_.open(path, std::ios::out | std::ios::binary);
}

MapFileO::~MapFileO() {
    file_.close();
}

void MapFileO::write_uint32(unsigned int n) {
	file_ << (unsigned char)n << (unsigned char)(n >> 8) << (unsigned char)(n >> 16) << (unsigned char)(n >> 24);
}

void MapFileO::write_int32(int n) {
    write_uint32(*(unsigned int*)(&n));
}

void MapFileO::write_spoint3(Point3 p) {
    *this << Point3_S16{ p };
}

void MapFileO::write_long_str(std::string str) {
	unsigned int len = (unsigned int)str.size();
	file_ << (unsigned char)(len) << (unsigned char)(len >> 8);
	file_.write(str.c_str(), len);
}

MapFileO& MapFileO::operator<<(unsigned char n) {
    file_ << n;
    return *this;
}

MapFileO& MapFileO::operator<<(int n) {
    file_ << (unsigned char) n;
    return *this;
}

MapFileO& MapFileO::operator<<(unsigned int n) {
    file_ << (unsigned char) n;
    return *this;
}

// Serializes floats in the range [-2^11, 2^11) with 24 bits of precision
MapFileO& MapFileO::operator<<(double f) {
	file_ << (unsigned char)(unsigned int)((f + (1 << 11)) / (1 << 4));
    file_ << (unsigned char)(unsigned int)((1 << 4) * f);
    file_ << (unsigned char)(unsigned int)((1 << 12) * f);
    return *this;
}

MapFileO& MapFileO::operator<<(bool b) {
    file_ << (unsigned char) b;
    return *this;
}

MapFileO& MapFileO::operator<<(Point2 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) pos.y;
    return *this;
}

MapFileO& MapFileO::operator<<(Point3_S16 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) ((pos.x + (1 << 15)) >> 8);
    file_ << (unsigned char) pos.y;
    file_ << (unsigned char) ((pos.y + (1 << 15)) >> 8);
    file_ << (unsigned char) pos.z;
    file_ << (unsigned char) ((pos.z + (1 << 15)) >> 8);
    return *this;
}

// NOTE: all Point3's that get serialized are actually in the range [0,255]
MapFileO& MapFileO::operator<<(Point3 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) pos.y;
    file_ << (unsigned char) pos.z;
    return *this;
}

MapFileO& MapFileO::operator<<(FPoint3 pos) {
	*this << pos.x;
    *this << pos.y;
    *this << pos.z;
    return *this;
}

MapFileO& MapFileO::operator<<(IntRect r) {
	*this << r.xa << r.ya << r.xb << r.yb;
	return *this;
}

MapFileO& MapFileO::operator<<(FloatRect r) {
	*this << r.xa << r.ya << r.xb << r.yb;
	return *this;
}

MapFileO& MapFileO::operator<<(std::string str) {
    file_ << (unsigned char) str.size();
    file_.write(str.c_str(), str.size());
    return *this;
}

// TODO: consider replacing these with a template!

MapFileO& MapFileO::operator<<(MapCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(ObjCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(ModCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(DeltaCode code) {
	file_ << (unsigned char)code;
	return *this;
}

MapFileO& MapFileO::operator<<(CameraCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(ObjRefCode code) {
	file_ << (unsigned char)code;
	return *this;
}

MapFileO& MapFileO::operator<<(Sticky sticky) {
    file_ << (unsigned char) sticky;
    return *this;
}

MapFileO& MapFileO::operator<<(PlayerState state) {
    file_ << (unsigned char) state;
    return *this;
}

MapFileO& MapFileO::operator<<(CarType type) {
	file_ << (unsigned char)type;
	return *this;
}

MapFileO& MapFileO::operator<<(AnimationSignal signal) {
	file_ << (unsigned char)signal;
	return *this;
}

MapFileO& MapFileO::operator<<(CauseOfDeath value) {
	file_ << (unsigned char)value;
	return *this;
}

MapFileO& MapFileO::operator<<(const ColorCycle& color) {
    file_ << (unsigned char) color.size_;
    file_ << (unsigned char) color.index_;
    for (int i = 0; i < color.size_; ++i) {
        file_ << (unsigned char) color.colors_[i];
    }
    return *this;
}

