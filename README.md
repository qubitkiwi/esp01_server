# esp8266 모듈을 사용한 간단한 서버

사용보드 : stm32f103c8

wifi 모듈 : esp8266 esp-01

uart, dma, gpio를 LL 라이브러리를 사용하여 구현했습니다.

usb cdc를 이용하여 esp8266에서 받은 데이터를 pc로 전송합니다.

pc에서 stm32로 보내는건 되지 않습니다.

<img src="picture.png">