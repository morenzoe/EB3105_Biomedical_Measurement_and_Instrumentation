float Vout_read1 = 0.000;
float Vout_read2 = 0.000;
float Vout1 = 0.000;
float Vout2 = 0.000;
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
  Vout_read1 = analogRead(A0);
  Vout_read2 = analogRead(A1);
  Vout1 = (Vout_read1 * 5.00) / 1024.00;
  Vout2 = (Vout_read2 * 5.00) / 1024.00;
  Vout = Vout1 - Vout2;

  // temp = (Vout+0.3177)/0.01;                   // Regresi termometer tubuh
  // temp = ((Vout+0.3177)/0.01)+1.8;             // Error correction penambahan konstanta termometer tubuh
  temp = (((Vout+0.3177)/0.01)+7.2489)/1.1454; // Error correction regresi ganda termometer tubuh

  // temp = (Vout-0.6689)/0.0451;         // Regresi termometer ruang
  // temp = ((Vout-0.6689)/0.0451)+2;     // Error correction penambahan konstanta termometer ruang
  // temp = (((Vout-0.6689)/0.0451)+2)/1; // Error correction regresi ganda termometer ruang

  Serial.print("Vout: ");
  Serial.println(Vout, 4); // Termometer Tubuh
  // Serial.println(Vout, 2); // Termometer Ruangan
  Serial.print("\n");

  Serial.print("Temperature: ");
  Serial.println(movingAverage(temp), 1); // Termometer Tubuh
  // Serial.println(movingAverage(temp)); // Termometer Ruangan
  Serial.print("\n");
  Serial.print("\n");
  
  delay(800);
}
