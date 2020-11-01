#include <Arduino.h>
#include "EnvironmentCalculation.h"

float convertCtoF(float c) { return c * 1.8 + 32; }
float convertFtoC(float f) { return (f - 32) * 0.55555; }

/*!
 * @brief Fast calculation of Dew Point
 *  
 * @param Temperature in celsius
 * 
 * @param Humidity in %
*/
double computeDewPoint2(double celsius, double humidity)
{
    double RATIO = 373.15 / (273.15 + celsius); // RATIO was originally named A0, possibly confusing in Arduino context
    double SUM = -7.90298 * (RATIO - 1);
    SUM += 5.02808 * log10(RATIO);
    SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO))) - 1);
    SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1);
    SUM += log10(1013.246);
    double VP = pow(10, SUM - 3) * humidity;
    double T = log(VP / 0.61078); // temp var
    return (241.88 * T) / (17.558 - T);
}

/*!
 * @brief Fast calculation of Dew Point
 *  
 * @param Temperature in celsius
 * 
 * @param Humidity in %
*/
double dewPointFast(double celsius, double humidity)
{
    double a = 17.271;
    double b = 237.7;
    double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
    double Td = (b * temp) / (a - temp);
    return Td;
}

/*!
 *  @brief  Printout data to serial
 *  
 *  @param  temperature temperatture
 *  @param  pressure pressure 
 */

void printEnvDataToSerial(float temperature, float pressure, float humidity, float dewPoint, float heatIndex, float altitude)
{
    Serial.print(F("Temperature = "));
    Serial.print(temperature);
    Serial.println(" *C");
    Serial.print(F("Pressure = "));
    Serial.print(pressure);
    Serial.println(" mbar");
    Serial.print(F("Humidity = "));
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print(F("Dew Point = "));
    Serial.print(dewPoint);
    Serial.println(" *C");
    Serial.print(F("Heat Index = "));
    Serial.print(heatIndex);
    Serial.println(" *C");
    Serial.print(F("Altitute = "));
    Serial.print(altitude);
    Serial.println(" m");
}
/*!
 *  @brief  Compute Heat Index
 *  				Using both Rothfusz and Steadman's equations
 *					(http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml)
 *  @param  temperature
 *          temperature in selected scale
 *  @param  percentHumidity
 *          humidity in percent
 *  @param  isFahrenheit
 * 					true if fahrenheit, false if celcius
 *	@return float heat index
 */

float computeHeatIndex(float temperature, float percentHumidity,
                       bool isFahrenheit)
{
    float hi;

    if (!isFahrenheit)
        temperature = convertCtoF(temperature);

    hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) +
                (percentHumidity * 0.094));

    if (hi > 79)
    {
        hi = -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity +
             -0.22475541 * temperature * percentHumidity +
             -0.00683783 * pow(temperature, 2) +
             -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature * pow(percentHumidity, 2) +
             -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

        if ((percentHumidity < 13) && (temperature >= 80.0) &&
            (temperature <= 112.0))
            hi -= ((13.0 - percentHumidity) * 0.25) *
                  sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

        else if ((percentHumidity > 85.0) && (temperature >= 80.0) &&
                 (temperature <= 87.0))
            hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
    }

    return isFahrenheit ? hi : convertFtoC(hi);
}
