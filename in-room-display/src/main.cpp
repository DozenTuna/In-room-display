#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include "lodepng.h"
#include "FS.h"
#include "SPIFFS.h"

// ピンの定義
const uint8_t SCLK_OLED = 18; //SCLK (SPI Clock)
const uint8_t MOSI_OLED = 23; //MOSI (Master Output Slave Input)
const uint8_t CS_OLED   = 5; //OLED ChipSelect
const uint8_t DC_OLED   = 17; //OLED DC (Data/Command)
const uint8_t RST_OLED  = 16; //OLED Reset RST

// SSD1331のオブジェクトを作成
Adafruit_SSD1331 display = Adafruit_SSD1331(CS_OLED,DC_OLED,MOSI_OLED,SCLK_OLED,RST_OLED);

// Wi-Fi設定を読み込む関数
bool readConfig(String &ssid, String &password, String &address, String &username) {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }

    File file = SPIFFS.open("/config.txt", "r");
    if (!file || file.isDirectory()) {
        Serial.println("Failed to open config file");
        return false;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim(); // Remove any extra whitespace/newline characters
        int separatorIdx = line.indexOf('=');
        if (separatorIdx < 0) continue;
        
        String key = line.substring(0, separatorIdx);
        String value = line.substring(separatorIdx + 1);

        if (key == "ssid") {
            ssid = value;
        } else if (key == "password") {
            password = value;
        } else if (key == "address") {
            address = value;
        } else if (key == "username") {
            username = value;
        }
    }
    file.close();
    return true;
}

// 565形式に変換する関数
uint16_t convertRGBto565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// PNG画像のデコード用関数
void drawPNG(const std::vector<unsigned char>& png) {
    std::vector<unsigned char> image; //RGBA形式の生データ
    unsigned width, height;

    // PNG画像をデコード
    unsigned error = lodepng::decode(image, width, height, png);
    
    // エラーチェック
    if (error) {
        Serial.print("PNG decode error ");
        Serial.println(error);
        return;
    }
    uint16_t* bitmap_img = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    if (bitmap_img == NULL) {
        Serial.println("Memory allocation failed!");
        return;
    }
    // 画像をディスプレイに描画
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            // RGBAからRGBに変換
            unsigned r = image[4 * width * y + 4 * x + 0];
            unsigned g = image[4 * width * y + 4 * x + 1];
            unsigned b = image[4 * width * y + 4 * x + 2];
            // 565形式に変換してピクセルを描画
            uint16_t color = convertRGBto565(r, g, b);
            bitmap_img[width*y+x] = color;
            // display.drawPixel(x, y, color);
        }
    }
    display.drawRGBBitmap(0,0,bitmap_img,96,64);
    free(bitmap_img);
}

void setup() {
    // シリアル通信を開始
    Serial.begin(115200);

    // SPIFFSからWi-Fi設定を読み込む
    String ssid, password, address, username;
    if (!readConfig(ssid, password, address, username)) {
        Serial.println("Failed to read configuration");
        return;
    }

    // Wi-Fiに接続
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // SSD1331ディスプレイの初期化
    display.begin();

    // ディスプレイをクリア
    uint16_t black = convertRGBto565(0,0,0);
    Serial.println(black);
    display.fillRect(0,0,96,64,0xFFFF);
    Serial.println("display blackfill");    
}

void loop() {
     // SPIFFSからWi-Fi設定を読み込む
    String ssid, password, address, username;
    if (!readConfig(ssid, password, address, username)) {
        Serial.println("Failed to read configuration");
        return;
    }
    String position;
    long RandNum = random(3);
    if (RandNum < 1) {
        position = "in";
    } else if (RandNum < 2) {
        position = "out";
    } else {
        position = "near";
    }

    // HTTPクライアントを使用してPNG画像を取得
    HTTPClient http;
    // リモートPCの設定
    String image_path = address + username + "_"+position+".png";
    const char* serverName = image_path.c_str();
    http.begin(serverName);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        std::vector<unsigned char> payload;
        WiFiClient * stream = http.getStreamPtr();
        size_t len = http.getSize();
        payload.resize(len);
        stream->readBytes(payload.data(), len);

        // PNG画像を描画
        drawPNG(payload);
    } else {
        Serial.printf("Failed to retrieve PNG, HTTP code: %d\n", httpCode);
    }

    http.end();
    delay(10000);
}
