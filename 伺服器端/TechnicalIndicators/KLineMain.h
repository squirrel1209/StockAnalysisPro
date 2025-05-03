#ifndef KLINE_MAIN_H
#define KLINE_MAIN_H
#include <vector>
#include <string>
#include "KLineRecord.h"

double getValue(const std::vector<KLineRecord> &records, int index, const std::string &field);
std::string getValueAsString(const std::vector<KLineRecord> &records, int index, const std::string &field);

std::vector<double> getAllOpenValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllHighValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllLowValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllCloseValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllVolumeValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllMA5Values(const std::vector<KLineRecord> &records);
std::vector<double> getAllMA10Values(const std::vector<KLineRecord> &records);
std::vector<double> getAllMA20Values(const std::vector<KLineRecord> &records);
std::vector<double> getAllKValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllDValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllRSIValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllMACDLineValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllSignalLineValues(const std::vector<KLineRecord> &records);
std::vector<double> getAllHistogramValues(const std::vector<KLineRecord> &records);
std::vector<std::string> getAllDateValues(const std::vector<KLineRecord> &records);
std::vector<std::string> getAllSignalValues(const std::vector<KLineRecord> &records);
std::vector<std::string> getAllStrengthValues(const std::vector<KLineRecord> &records);

#endif