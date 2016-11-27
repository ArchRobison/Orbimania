#include "File.h"
#include "Universe.h"
#include "View.h"
#include <cstdio>
#include <cstdint>
#include <cerrno>

static const int32_t FileFormatVersion = 1;

void ReportFileError(const std::string& filename, const char* problem) {
	// FIXME - send message to user
}

class IO {
protected:
	FILE* f;
	IO(const std::string& filename, const char* mode) : f(nullptr) {
		f = std::fopen(filename.c_str(), mode);
		if(!f)
			throw(errno);
	}
public:
	~IO();
};

IO::~IO() {
	if(f)
		fclose(f);
}

class Writer : public IO {
public:
	Writer(std::string filename) : IO(filename, "w") {}
	template<typename T>
	void transmit(const T& x) {
		if(fwrite(&x, sizeof(T), 1, f)!=1)
			throw errno;
	}
};

class Reader : public IO {
public:
	Reader(std::string filename) : IO(filename, "r") {}
	template<typename T>
	void transmit(T& x) {
		if(fread(&x, sizeof(T), 1, f)!=1)
			throw errno;
	}
};

template<typename RW>
static inline void TransmitVal(RW& rw) {
}

template<typename RW, typename T, typename... U>
static inline void TransmitVal(RW& rw, T& x, U&... rest) {
	rw.transmit(x);
	TransmitVal(rw, rest...);
}

// Reads as "Orbi" on little-endian machines
static uint32_t FileHeader = 'i'<<24 | 'b'<<16 | 'r'<<8 | 'O';

template<typename RW>
static void Transmit(RW& rw) {
	using namespace Universe;
	TransmitVal(rw, ViewScale, ViewOffsetX, ViewOffsetY);
	TransmitVal(rw, ViewVelocityScale, ViewMassScale, ViewChargeScale);
	int32_t z = ZoomLevel;
	TransmitVal(z);
	ZoomLevel = z;
	int32_t n = int32_t(NParticle);
	TransmitVal(rw, n);
	if(n>N_PARTICLE_MAX)
		n = N_PARTICLE_MAX;
	NParticle = n;
	for(size_t k=0; k<n; ++k) {
		TransmitVal(rw, Mass[k], Charge[k], Sx[k], Sy[k], Vx[k], Vy[k]);
	}
}

void WriteUniverseToFile(const std::string& filename) {
	using namespace Universe;
	try {
		Writer w(filename);
	    w.transmit(FileHeader);
	    w.transmit(FileFormatVersion);
	    Transmit(w);
	}
	catch(int error) {
		ReportFileError(filename, strerror(error));
	}
}

void ReadUniverseFromFile(const std::string& filename) {
	try {
		Reader r(filename);
		uint32_t header;
		int32_t fileFormat;
		r.transmit(header);
		r.transmit(fileFormat);
		if(header!=FileHeader) {
			ReportFileError(filename, "not an Orbimania file");
			return;
		}
		if(fileFormat!=1) {
			ReportFileError(filename, "unrecognized file-format version");
			return;
		}
		Transmit(r);
	}
	catch(int error) {
		ReportFileError(filename, strerror(error));
	}
}
