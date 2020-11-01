double computeDewPoint2(double celsius, double humidity);
double dewPointFast(double celsius, double humidity);
float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit);
float convertCtoF(float c);
float convertFtoC(float f);

void printEnvDataToSerial(float temperature, float pressure, float humidity, float dewPoint, float heatIndex, float altitude);