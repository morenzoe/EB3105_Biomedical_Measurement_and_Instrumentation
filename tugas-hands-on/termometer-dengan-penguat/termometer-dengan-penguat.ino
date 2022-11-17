float Vout_read = 0.000;
float Vout = 0.000;
float temp = 0.0;

float movingAverage(float value) {
  const byte nvalues = 15;      // ukuran window

  static byte current = 0;
  static byte cvalues = 0;
  static float sum = 0;
  static float values[nvalues];

  sum += value;

  // Membuang elemen pertama jika window penuh (FIFO)
  if (cvalues == nvalues)
    sum -= values[current];

  values[current] = value;

  if (++current >= nvalues)
    current = 0;

  if (cvalues < nvalues)
    cvalues += 1;

  return sum/cvalues;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  Vout_read = analogRead(A0);
  Vout = (Vout_read * 5.00) / 1024.00;

  // temp = (Vout+14.926)/0.4718;                   // Regresi termometer tubuh
  // temp = ((Vout+14.926)/0.4718)-3.8;             // Error correction penambahan konstanta termometer tubuh
  temp = (((Vout+14.926)/0.4718)-7.7203)/0.8884; // Error correction regresi ganda termometer tubuh

  // temp = (Vout-2.8913)/0.1948;         // Regresi termometer ruang
  // temp = ((Vout-2.8913)/0.1948)+0;     // Error correction penambahan konstanta termometer ruang
  // temp = (((Vout-2.8913)/0.1948)+0)/1; // Error correction regresi ganda termometer ruang

  Serial.print("Vout: ");
  Serial.println(Vout, 2);
  Serial.print("\n");

  Serial.print("Temperature: ");
  Serial.println(movingAverage(temp), 1); // Termometer Tubuh
  // Serial.println(movingAverage(temp)); // Termometer Ruangan
  Serial.print("\n");
  Serial.print("\n");
  
  delay(800);
}
