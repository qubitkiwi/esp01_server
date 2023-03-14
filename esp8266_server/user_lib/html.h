/*
 * html.h
 *
 *  Created on: 2023. 3. 11.
 *  Author: asong
 */

#ifndef HTML_H_
#define HTML_H_


const char *OK_HEADER = "HTTP/1.1 200 OK\r\n\r\n";
const char *NOT_found_404 = "HTTP/1.1 404 Not Found\r\n";

const char *HELLO_HTML = "<!doctype html><html><head><title>ESP8266</title></head><body><h1>esp hello world</h1></body></html>";

const char* ROOT_HTML = "<!DOCTYPE html>\
<html>\
<head>\
    <title>esp8366</title>\
</head>\
<body>\
    <button id='gpio1' value='0'>GPIO</button>\
    <output id='state1'></output>\
    <br>\
    <button id='gpio2' value='1'>GPIO</button>\
    <output id='state2'></output>\
    <br>\
    <button id='gpio3' value='2'>GPIO</button>\
    <output id='state3'></output>\
    <br>\
    <script>\
        const gpio1 = document.getElementById('gpio1');\
        const gpio2 = document.getElementById('gpio2');\
        const gpio3 = document.getElementById('gpio3');\
\
        const state1 = document.getElementById('state1');\
        const state2 = document.getElementById('state2');\
        const state3 = document.getElementById('state3');\
\
        let status = [0, 0, 0];\
\
        const rander = () => {\
            state1.textContent = (status[0] == 1) ? 'gpio on' : 'gpio off';\
            state2.textContent = (status[1] == 1) ? 'gpio on' : 'gpio off';\
            state3.textContent = (status[2] == 1) ? 'gpio on' : 'gpio off';\
        };\
\
        const gpio_update = async () => {\
            await fetch('/gpio')\
                .then((res) => res.json())\
                .then((data) => status = data);\
            rander();\
        };\
\
        setInterval(gpio_update, 1000);\
        gpio_update();\
\
        const put_gpio = (e) => {\
            const value = parseInt(e.target.value, 10);\
            status[value] = (status[value] == 1) ? 0 : 1;\
            fetch('/gpio', {\
                method: 'PUT',\
                body: JSON.stringify(status)\
            });\
            rander();\
        };\
\
        gpio1.addEventListener('click', put_gpio);\
        gpio2.addEventListener('click', put_gpio);\
        gpio3.addEventListener('click', put_gpio);\
    </script>\
</body>\
</html>";


#endif /* HTML_H_ */
