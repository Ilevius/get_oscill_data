#include "stdafx.h"
#include <iostream>
#include "oscill.h"
#include <cstring>
#include <iomanip>
#include <string>

using namespace std;
// пойдет
void OscilloscopeRigol_DS1054Z::connect() {
	cout << "Started OWON6102A connection" << endl; // OWON6102A connection has been started
															// Адрес прибора
	const char* resource = "USB0::0x5345::0x1235::2222099::INSTR";

	DEVICE = VI_NULL;
	RESOURCE_MANAGER = VI_NULL;
	ViStatus status;
	// Открываем Resource Manager
	status = viOpenDefaultRM(&RESOURCE_MANAGER);
	if (status != VI_SUCCESS)
	{
		throw std::exception("Resource manager error!");
	}
	//Открываем прибор
	status = viOpen(RESOURCE_MANAGER, resource, VI_NULL, VI_NULL, &DEVICE);
	if (status != VI_SUCCESS)
	{
		cout << "Device not found!\n";
		//throw "Device not found!\n";
	}
	else {
		printf("OWON6102A connected succesfully\n"); //Oscilloscope has been connected
		OscilloscopeRigol_DS1054Z::connection = true;

	}
}
//
//// пойдет
void OscilloscopeRigol_DS1054Z::disconnect() {
	cout << "Запущено рассоединение с OWON6102A" << endl; //OWON6102A disconnection has been started
	viClose(DEVICE);
	viClose(RESOURCE_MANAGER);
	printf(" Осцилограф был успешно отсоединён\n"); // Oscilloscope has been disconnected
	OscilloscopeRigol_DS1054Z::connection = false;
}

void OscilloscopeRigol_DS1054Z::setup() {
	cout << "Started setup OWON6102A" << endl; // OWON6102A setup has been started


	CURR_DEPMEM = find_depMem(int64_t(100000));
	CURR_VOLT_SCALE = find_chanScale(int64_t(10));
	CURR_HORSCALE = find_horScale(int64_t(200));

	string setup_commands[] = {
		":HORIzontal:SCALe " + horScale_str(CURR_HORSCALE) + "\n",  // horisontal scale
		":ACQuire:PRECision 14\n",// precision
		":CH1:DISPlay ON\n",			// set chanell to display
		":CH1:OFFSet 1.2\n",
		":CH1:COUP AC\n",
		":TRIG:SING:MODE SLOPE\n",
		":TRIG:SING:EDGE:COUP AC\n",
		":TRIG:SING:EDGE:SOUR CH2\n",
		":TRIG:SING:EDGE:LEV 3\n",
		":ACQ:DEPMEM " + depMem_str(CURR_DEPMEM) + "\n",
		":CH1:SCAL " + chanDivScale_str(CURR_VOLT_SCALE) + "\n"
	};

	//											Начинаем настройку осцилографа
	for (string command : setup_commands) {
		viPrintf(DEVICE, command.c_str());
		Sleep(100);
	}

	//											Передача стартовых команд
	cout << "Inquire" << "                                 " << '|' << "  " << "Answer\n";
	cout << "-----------------------------------------------------\n";
	ask_and_print_answer(":HORIzontal:SCALe?\n");
	ask_and_print_answer(":ACQuire:PRECision?\n");
	ask_and_print_answer(":CH1:DISPlay?\n");
	ask_and_print_answer(":CH1:COUP?\n");
	ask_and_print_answer(":CH1:OFFSet?\n");
	ask_and_print_answer(":TRIG:SING:MODE?\n");
	ask_and_print_answer(":TRIG:SING:EDGE:COUP?\n");
	ask_and_print_answer(":TRIG:SING:EDGE:SOUR?\n");
	ask_and_print_answer(":TRIGger:SINGle:EDGE:LEVel?\n");
	ask_and_print_answer(":ACQ:DEPMEM?\n");
	ask_and_print_answer(":CH1:SCAL?\n");
}

// пойдет
int OscilloscopeRigol_DS1054Z::ask_and_print_answer(ViConstString inquire)
{
	char buffer[256];
	ViUInt32 bytes_read;
	ViStatus status;

	viPrintf(DEVICE, inquire);
	status = viRead(DEVICE, (ViBuf)buffer, 255, &bytes_read);

	if (status < VI_SUCCESS) {
		printf("Reading error: 0x%08X\n", status);
		buffer[bytes_read] = 0;
		cout << inquire << setw(40) << '|' << "  " << "Reading error: 0x%08X\n" << ' ' << status;
		return -1;
	}
	else {
		buffer[bytes_read] = 0;
		cout << inquire << setw(40) << '|' << "  " << buffer;
		// cout << "The ask is: " << inquire;
		// printf(" The answer is: %s\n", buffer);
		return 0;
	}
}

// нормально
bool OscilloscopeRigol_DS1054Z::trigger() {
	char status[16];
	viQueryf(DEVICE, ":TRIGger:STATus?\n", "%s", status);
	if (strstr(status, "TRIG")) {
		return true;
	}
	else {
		return false;
	}
}


vector<uint16_t> OscilloscopeRigol_DS1054Z::getRaw16BitSignal(const uint16_t& EMPTY_TICKS, const uint32_t& TICKS) {
	vector<uint16_t> result(TICKS, 0);

	uint32_t startRead = uint32_t(uint32_t(CURR_DEPMEM) / 2) - uint32_t(EMPTY_TICKS);
	const string WAV_RANGE = ":WAV:RANG " + to_string(startRead) + "," + to_string(TICKS) + "\n";

	ViUInt32 bytes_read;
	unsigned char read_buf[301000];

	// 1. Начать чтение
	viPrintf(DEVICE, ":WAV:BEG CH1\n");
	// 2. Задать offset и size  
	viPrintf(DEVICE, WAV_RANGE.c_str());
	// 3. Читать данные
	viPrintf(DEVICE, ":WAV:FETC?\n");

	viRead(DEVICE, read_buf, sizeof(read_buf) - 1, &bytes_read);
	read_buf[bytes_read] = '\0';

	// 4. Парсинг TMC + int16... (без изменений)

	if (bytes_read < 2 || read_buf[0] != '#') throw "Wrong format data packet from oscill!"; // if answer contains less then 2 bytes or has no header
	int n_digits = read_buf[1] - '0';						// amount of digits in data bytes number
	if (bytes_read < 2 + n_digits) throw "Empty data packet from oscill!";			// if there is no data after header

	std::string len_str((char*)read_buf + 2, n_digits);		// the string of data bytes number
	size_t data_len = std::stoul(len_str);					// integer data bytes number

															//						Проверка длины пакета данных и ее четности
	if (data_len & 1) {
		throw "Wrong format data packet from oscill!";
	}
	else {
		if (uint32_t(data_len / 2) > TICKS)
			throw "Data packet longer than demanded ticks";
	}
	// Извлечение int16 из байтов (big-endian)
	const unsigned char* data_ptr = read_buf + 2 + n_digits;

	for (size_t i = 0; i < data_len; i += 2) {
		uint16_t raw16 = (data_ptr[i + 1] << 8) | (data_ptr[i]);  // 16-бит слово
		result[i / 2] = (uint16_t)(raw16 & 0x3FFF);
	}
	viPrintf(DEVICE, ":WAV:END\n");

	return result;
}

double OscilloscopeRigol_DS1054Z::rawTickToVolts(double signal_tick) {
	return (signal_tick - 8192) / 6400;
}

std::string OscilloscopeRigol_DS1054Z::horScale_str(horScales_ns horScale) {
	switch (horScale)
	{
	case horScales_ns::ns1: return "1.0ns";
	case horScales_ns::ns2:  return "2.0ns";
	case horScales_ns::ns5:  return "5.0ns";
	case horScales_ns::ns10: return "10ns";
	case horScales_ns::ns20: return "20ns";
	case horScales_ns::ns50: return "50ns";
	case horScales_ns::ns100: return "100ns";
	case horScales_ns::ns200: return "200ns";
	case horScales_ns::ns500: return "500ns";
	case horScales_ns::us1:  return "1.0us";
	case horScales_ns::us2:  return "2.0us";
	case horScales_ns::us5:  return "5.0us";
	case horScales_ns::us10: return "10us";
	case horScales_ns::us20: return "20us";
	case horScales_ns::us50: return "50us";
	case horScales_ns::us100: return "100us";
	case horScales_ns::us200: return "200us";
	case horScales_ns::us500: return "500us";
	case horScales_ns::ms1:  return "1.0ms";
	case horScales_ns::ms2:  return "2.0ms";
	case horScales_ns::ms5:  return "5.0ms";
	case horScales_ns::ms10: return "10ms";
	case horScales_ns::ms20: return "20ms";
	case horScales_ns::ms50: return "50ms";
	case horScales_ns::ms100: return "100ms";
	case horScales_ns::ms200: return "200ms";
	case horScales_ns::ms500: return "500ms";
	case horScales_ns::s1:   return "1.0s";
	default:
		return "unknown";
		break;
	}

	
}