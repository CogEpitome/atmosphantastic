#pragma once

#include "data.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>

void setupHistory();
void pushHistory(HistoricalDataPoint *data);
int getHistoryCount();
HistoricalDataPoint getHistoricalDataPoint(int offset);