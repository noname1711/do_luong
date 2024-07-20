#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); 
#include <DHT.h>
#include <math.h>

const int DHTPIN = 5;  // Chân Out của cảm biến nối chân số 5 Arduino
const int DHTTYPE = DHT11;   // Khai báo kiểu cảm biến là DHT11

DHT dht(DHTPIN, DHTTYPE); // Khai báo thư viện chân cảm biến và kiểu cảm biến

const int a = 50;  // Số lượng giá trị đo

float doC_Readings[a];  // Mảng lưu trữ giá trị nhiệt độ
float doam_Readings[a];     // Mảng lưu trữ giá trị độ ẩm
int currentIndex = 0;  // Chỉ số hiện tại trong mảng

void setup() {
  Serial.begin(9600);
  dht.begin(); // Khởi động cảm biến
  lcd.init();  // Khởi động LCD
  lcd.backlight(); // Mở đèn
  lcd.setCursor(0,0);
  lcd.print("DOAM:");
  lcd.setCursor(0,1);
  lcd.print("C:");
}

void loop() {
  // Đọc giá trị nhiệt độ và độ ẩm từ cảm biến
  float doC = dht.readTemperature();
  float doam = dht.readHumidity();
  // Kiểm tra xem giá trị có hợp lệ hay không
  if (isnan(doC) || isnan(doam)) {
    Serial.println("Không có giá trị trả về từ cảm biến DHT");
    return;
  }
  // Lưu giá trị vào mảng
  doC_Readings[currentIndex] = doC;
  doam_Readings[currentIndex] = doam;
  currentIndex = (currentIndex + 1) % a;  // Di chuyển chỉ số vòng tròn trong mảng

  // Tính giá trị trung bình
  float doC_avg = 0;
  float doam_avg = 0;
  for (int i = 0; i < a; i++) {
    doC_avg += doC_Readings[i];
    doam_avg += doam_Readings[i];
  }
  doC_avg /= a;
  doam_avg /= a;

  // Tính độ lệch chuẩn
  float doC_std_dev  = 0;
  float doam_std_dev = 0;
  for (int i = 0; i < a; i++) {
    doC_std_dev  += pow(doC_Readings[i] -  doC_avg , 2);
    doam_std_dev += pow(doam_Readings[i] - doam_avg, 2);
  }
  doC_std_dev /= a; 
  doC_std_dev = sqrt(doC_std_dev); 
  doam_std_dev /= a; 
  doam_std_dev = sqrt(doam_std_dev); 

  // tinh khoang tin cay 
  float doC_lower_bound = doC_avg - (3 * doC_std_dev); 
  float doC_upper_bound = doC_avg + (3 * doC_std_dev);
  float doam_lower_bound = doam_avg - (3 * doam_std_dev); 
  float doam_upper_bound = doam_avg + (3 * doam_std_dev);
  
  // loai bo sai so tho nhiet do
  int valid_count = 0;
  float doC_sum = 0;
  for(int i = 0; i < a; i++) {
    if (doC_Readings[i] >= doC_lower_bound && doC_Readings[i] <= doC_upper_bound) 
    {
      valid_count++;
      doC_sum += doC_Readings[i];
    }
  }
  float doC_avg_adjusted = doC_sum / valid_count;
  float doC_std_dev_adjust = 0;
  for (int i = 0; i < a; i++) 
  {
    if (doC_Readings[i] >= doC_lower_bound && doC_Readings[i] <= doC_upper_bound) 
    {
      float deviation1 = doC_Readings[i] - doC_avg_adjusted;
       doC_std_dev_adjust += deviation1 * deviation1;
    }
  }
  doC_std_dev_adjust /= valid_count;
  doC_std_dev_adjust = sqrt(doC_std_dev_adjust);
  
  // loai bo sai so tho do am
  int valid1_count = 0;
  float doam_sum = 0;
  for(int i = 0; i < a; i++) {
    if (doam_Readings[i] >= doam_lower_bound && doam_Readings[i] <= doam_upper_bound) 
    {
      valid1_count++;
      doam_sum  += doam_Readings[i];
    }
  }
  float doam_avg_adjusted = doam_sum / valid1_count;
  float doam_std_dev_adjust = 0;
  for (int i = 0; i < a; i++) 
  {
    if (doam_Readings[i] >= doam_lower_bound && doam_Readings[i] <= doam_upper_bound) 
    {
      float deviation2 = doam_Readings[i] - doam_avg_adjusted;
       doam_std_dev_adjust += deviation2 * deviation2;
    }
  }
  doam_std_dev_adjust /= valid1_count;
  doam_std_dev_adjust = sqrt(doam_std_dev_adjust);

  // In ra màn hình LCD
  Serial.print("Độ ẩm: ");
  Serial.print(doam_avg_adjusted);
  Serial.print("% |  ");
  Serial.print(doam_std_dev_adjust);
  lcd.setCursor(5, 0); 
  lcd.print(doam_avg_adjusted);
  lcd.setCursor(10, 0); 
  lcd.print("|");
  lcd.print(doam_std_dev_adjust);
  
  Serial.print("Nhiệt độ: ");
  Serial.print(doC_avg_adjusted);
  Serial.print(" | ");
  Serial.print(doC_std_dev_adjust);
  Serial.print("\n");
  lcd.setCursor(5, 1);
  lcd.print(doC_avg_adjusted);
  lcd.setCursor(10, 1); 
  lcd.print("|");
  lcd.print(doC_std_dev_adjust);

  delay(1000);
}
