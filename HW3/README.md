캐니 에지 검출 알고리즘 구현 및 허프 변환 원 검출 구현

* 과제 개요 : opencv의 캐니 에지 검출 알고리즘과 허프변환 원 검출 알고리즘을 직접 구현
  * 결과 화면
    
원본이미지|캐니 에지 검출|캐니 에지 원 검출|허프변환 원 검출
--|---|--|--|
![image](https://github.com/shl0501/ComputerVision2024/assets/114389927/593b58a1-55db-4999-bd37-3224e04c4191)|![image](https://github.com/shl0501/ComputerVision2024/assets/114389927/171860a2-09a9-486f-9c5a-fb5954032d7a)|![image](https://github.com/shl0501/ComputerVision2024/assets/114389927/ba5e91bc-ab55-495e-ab96-74f8cae12dc5)|![image](https://github.com/shl0501/ComputerVision2024/assets/114389927/9d77782a-ec34-4de4-bd5b-63fb77e8aa95)|
    

  * 검출한 4개의 원의 중심점, 반지름의 길이와 정답으로 제공된 ground truth와의 비교

-|1번 원|2번원|3번원|4번원
--|--|--|--|--|
ground truth|(a, b) = (284, 116), 반지름 = 76|(a, b) = (118, 290), r = 75|(a, b) = (116, 114), r = 70|(a, b) = (292, 278), r = 56 |
hough transform results|(a, b) = (281, 120), r = 83|(a, b) = (117, 292), r = 77|(a, b) = (120, 114), r = 73|(a, b) = (292, 278), r = 54|
 
