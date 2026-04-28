/* 
// Keeps a circular buffer of the past HISTORY_COUNT measurements.
// Saves and loads buffer to permanent memory on new measurements.
*/
#include "history.h"

const char* STORAGE_KEY = "history";
const int HISTORY_COUNT = 168; // One week of hourly measurements (24 * 7)

struct HistoryStorage {
    int32_t count;
    HistoricalDataPoint data[HISTORY_COUNT];
};

HistoricalDataPoint history[HISTORY_COUNT];
int history_count = 0;
int history_head = 0;

void saveDataPoint(HistoricalDataPoint *data, int count);
int loadHistory(HistoricalDataPoint *history);

int getHistoryCount() { return history_count; }

// Initialize flash memory and load existing history
void setupHistory() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      nvs_flash_erase();
      nvs_flash_init();
  }
  history_count = loadHistory(history);
  history_head = history_count % HISTORY_COUNT;
}

// Add data point and save to flash.
void pushHistory(HistoricalDataPoint *data) {
  history[history_head] = *data;
  history_head = (history_head + 1) % HISTORY_COUNT;
  if (history_count < HISTORY_COUNT) history_count++;
  saveDataPoint(history, history_count);
}

// Get history point by value
HistoricalDataPoint getHistoricalDataPoint(int offset) {
    int oldest = (history_count < HISTORY_COUNT) ? 0 : history_head;
    int idx = (oldest + offset) % HISTORY_COUNT;
    return history[idx];
}

// Internal call to load history from flash
int loadHistory(HistoricalDataPoint *history) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) != ESP_OK) return 0;
    
    HistoryStorage storage;
    size_t size = sizeof(HistoryStorage);
    if (nvs_get_blob(handle, STORAGE_KEY, &storage, &size) != ESP_OK) {
        nvs_close(handle);
        return 0;
    }
    
    int32_t count = storage.count;
    if (count > HISTORY_COUNT) count = HISTORY_COUNT;
    memcpy(history, storage.data, sizeof(HistoricalDataPoint) * count);
    
    nvs_close(handle);
    return count;
}

// Internal call to save history to flash
void saveDataPoint(HistoricalDataPoint *data, int count) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) != ESP_OK) return;

    HistoryStorage storage;
    storage.count = count;
    memcpy(storage.data, data, sizeof(HistoricalDataPoint) * count);

    nvs_set_blob(handle, STORAGE_KEY, &storage, sizeof(storage));
    nvs_commit(handle);
    nvs_close(handle);
}