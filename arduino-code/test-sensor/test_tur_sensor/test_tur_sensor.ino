#include <Wire.h>
int sensorPin = A14;
float volt;
float ntu;

void setup()
{
  Serial.begin(9600);
}

void loop()
{

  volt = 0;
  for (int i = 0; i < 800; i++)
  {
    volt += ((float)analogRead(A14) / 1023) * 5;
  }
  volt = volt / 800;
  volt = round_to_dp(volt, 2);
  if (volt < 2.5) {
    ntu = 3000;
  } else {
    //ntu = -1120.4 * square(volt) + 5742.3 * volt - 4353.8;
    ntu = -1120.4 * sqrt(volt) + 5742.3 * volt - 4353.8;
    
  }
  Serial.print("Volte : ");
  Serial.println(volt);
  Serial.print("ntu : ");
  Serial.println(ntu);
  Serial.println("----------------");
  Serial.println(analogRead(A14));
  delay(1000);
}

float round_to_dp( float in_value, int decimal_place )
{
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) / multiplier;
  return in_value;
}
