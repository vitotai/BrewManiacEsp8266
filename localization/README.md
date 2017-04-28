# Localization

To localize the LCD strings, you must replace localized `string.h` file and re-build the image.
(I don't really recommend doing this, because the string length is constraint on 20x4 LCD.) 
To localize the Web interface, just update the `index.htm` file by either
* put it in `/data` folder, and upload by Data Uploader tool
* use `http://THE_IP_OR_NAME_LOACAL:8008/filemanager` to upload the localized `index.htm`
---
# Available Localization
* Russian/index.htm
    provided by __Yuri Moiseev__