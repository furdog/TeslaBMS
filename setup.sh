#SETUP tools
mkdir -p tools
cd tools

curl -L https://github.com/arduino/arduino-cli/releases/download/v1.0.4/arduino-cli_1.0.4_Windows_64bit.zip > arduino-cli.zip
unzip arduino-cli.zip arduino-cli.exe
rm arduino-cli.zip

./arduino-cli core install esp8266:esp8266 --config-file ../.cli-config.yml
./arduino-cli core install esp32:esp32     --config-file ../.cli-config.yml

cd ..

#SETUP libraries
mkdir -p libraries
cd libraries

#curl -L https://github.com/lorol/LITTLEFS/archive/refs/tags/1.0.6.zip > fs.zip
#unzip fs.zip
#rm fs.zip

curl -L https://github.com/furdog/lwDASH/archive/refs/heads/main.zip > lwDASH.zip
unzip lwDASH.zip
rm lwDASH.zip

curl -L https://github.com/Bodmer/TFT_eSPI/archive/refs/tags/V2.5.43.zip > TFT_eSPI.zip
unzip TFT_eSPI.zip
rm TFT_eSPI.zip

curl -L https://github.com/lvgl/lvgl/archive/refs/tags/v9.1.0.zip > lvgl.zip
unzip lvgl.zip
rm lvgl.zip

curl -L https://github.com/ElektroBOXua/TeslaBMS/archive/refs/heads/main.zip > tbms.zip
unzip tbms.zip
rm tbms.zip

cd ..
