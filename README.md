# esp8266 모듈을 사용한 간단한 서버

사용보드 : stm32f103c8

wifi 모듈 : esp8266 esp-01

uart, dma, gpio를 LL 라이브러리를 사용하여 구현했습니다.

usb cdc를 이용하여 esp8266에서 받은 데이터를 pc로 전송합니다.

GET /gpio 요청에는 현재 LED 상태를 배열로 전송합니다. 브라우저가 1초마다 요청합니다.

PUT /gpio 요청은 body를 읽고 LED 상태를 변환합니다.


주의사항: wifi 모듈이 전류를 많이 먹어 별도의 전원이 필요합니다.

[led_control.webm](https://user-images.githubusercontent.com/68237656/224910526-1cae7d3b-33e4-4705-83e5-171c107a10bf.webm)
