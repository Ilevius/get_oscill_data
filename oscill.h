#include <vector>
#include <Windows.h>
#include "include/visa.h"

#pragma comment(lib, "include/visa64.lib")

#pragma once
using namespace std; // 125 567


class OscilloscopeRigol_DS1054Z  {
public:
	~OscilloscopeRigol_DS1054Z() = default;
	void connect();
	void disconnect();
	void setup();
	bool trigger();
	vector<uint16_t> getRaw16BitSignal(const uint16_t& EMPTY_TICKS, const uint32_t& TICKS);
	double rawTickToVolts(double signal_tick);

private:
	ViSession DEVICE;
	ViSession RESOURCE_MANAGER;
	int ask_and_print_answer(ViConstString inquiry);

	bool connection;

	enum class horScales_ns : uint64_t
	{
		ns1 = 1, ns2 = 2, ns5 = 5, ns10 = 10, ns20 = 20, ns50 = 50, ns100 = 100, ns200 = 200, ns500 = 500, us1 = 1000, us2 = 2000,
		us5 = 5000, us10 = 10000, us20 = 20000, us50 = 50000, us100 = 100000, us200 = 200000, us500 = 500000, ms1 = 1000000,
		ms2 = 2000000, ms5 = 5000000, ms10 = 10000000, ms20 = 20000000, ms50 = 50000000, ms100 = 100000000, ms200 = 200000000,
		ms500 = 500000000, s1 = 1000000000
	};

	enum class depMem :uint64_t
	{
		k1 = 1000, k10 = 10000, k100 = 100000, m1 = 1000000, m10 = 10000000
	};

	enum class chanDivScale_mv :uint64_t {
		mv2 = 2, mv5 = 5, mv10 = 10, mv20 = 20, mv50 = 50, mv100 = 100, mv200 = 200, mv500 = 500, v1 = 1000, v2 = 2000, v5 = 5000
	};

	std::string horScale_str(horScales_ns horScale);

	std::string depMem_str(depMem dpm) {
		switch (dpm)
		{
		case depMem::k1: return "1k";
			break;
		case depMem::k10: return "10k";
			break;
		case depMem::k100: return "100k";
			break;
		case depMem::m1: return "1M";
			break;
		case depMem::m10: return "10M";
			break;
		default: throw "Invalid DEPMEM value!";
			break;
		}
	}

	std::string chanDivScale_str(chanDivScale_mv chanScale) {
		switch (chanScale) {
		case chanDivScale_mv::mv2:   return "2mV";
		case chanDivScale_mv::mv5:   return "5mV";
		case chanDivScale_mv::mv10:  return "10mV";
		case chanDivScale_mv::mv20:  return "20mV";
		case chanDivScale_mv::mv50:  return "50mV";
		case chanDivScale_mv::mv100: return "100mV";
		case chanDivScale_mv::mv200: return "200mV";
		case chanDivScale_mv::mv500: return "500mV";
		case chanDivScale_mv::v1:    return "1V";
		case chanDivScale_mv::v2:    return "2V";
		case chanDivScale_mv::v5:    return "5V";
		default: return "unknown";
		}
	}

	horScales_ns find_horScale(uint64_t ns_value) {
		switch (ns_value) {
		case 1:         return horScales_ns::ns1;
		case 2:         return horScales_ns::ns2;
		case 5:         return horScales_ns::ns5;
		case 10:        return horScales_ns::ns10;
		case 20:        return horScales_ns::ns20;
		case 50:        return horScales_ns::ns50;
		case 100:       return horScales_ns::ns100;
		case 200:       return horScales_ns::ns200;
		case 500:       return horScales_ns::ns500;
		case 1000:      return horScales_ns::us1;
		case 2000:      return horScales_ns::us2;
		case 5000:      return horScales_ns::us5;
		case 10000:     return horScales_ns::us10;
		case 20000:     return horScales_ns::us20;
		case 50000:     return horScales_ns::us50;
		case 100000:    return horScales_ns::us100;
		case 200000:    return horScales_ns::us200;
		case 500000:    return horScales_ns::us500;
		case 1000000:   return horScales_ns::ms1;
		case 2000000:   return horScales_ns::ms2;
		case 5000000:   return horScales_ns::ms5;
		case 10000000:  return horScales_ns::ms10;
		case 20000000:  return horScales_ns::ms20;
		case 50000000:  return horScales_ns::ms50;
		case 100000000: return horScales_ns::ms100;
		case 200000000: return horScales_ns::ms200;
		case 500000000: return horScales_ns::ms500;
		case 1000000000ULL: return horScales_ns::s1;
		default:        return horScales_ns::us200;  // 10 нс Ч разумный default
		}
	}

	depMem find_depMem(uint64_t points) {
		switch (points) {
		case 1000:      return depMem::k1;
		case 10000:     return depMem::k10;
		case 100000:    return depMem::k100;
		case 1000000:   return depMem::m1;
		case 10000000:  return depMem::m10;
		default:        throw "incorrect DEPMEM VALUE!";  // 10k Ч разумный default
		}
	}

	chanDivScale_mv find_chanScale(uint64_t mv_value) {
		switch (mv_value) {
		case 2:     return chanDivScale_mv::mv2;
		case 5:     return chanDivScale_mv::mv5;
		case 10:    return chanDivScale_mv::mv10;
		case 20:    return chanDivScale_mv::mv20;
		case 50:    return chanDivScale_mv::mv50;
		case 100:   return chanDivScale_mv::mv100;
		case 200:   return chanDivScale_mv::mv200;
		case 500:   return chanDivScale_mv::mv500;
		case 1000:  return chanDivScale_mv::v1;
		case 2000:  return chanDivScale_mv::v2;
		case 5000:  return chanDivScale_mv::v5;
		default:    return chanDivScale_mv::mv10;  // 10mV Ч разумный default
		}
	}


	depMem CURR_DEPMEM = depMem::k100;
	chanDivScale_mv CURR_VOLT_SCALE = chanDivScale_mv::v5;
	horScales_ns CURR_HORSCALE = horScales_ns::us200;
};
