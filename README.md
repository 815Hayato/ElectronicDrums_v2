# ElectronicDrums_v2

![ElectronicDrums_v2](https://github.com/user-attachments/assets/e0eed7ac-79a3-4384-800b-0ae96cfe2dc1)

## 概要
自作電子ドラム,ElectronicDrums_v1を改良したもの  
→https://github.com/815Hayato/ElectronicDrums_v1

## 使用例
以下のリンクから試聴することができます。  
→https://drive.google.com/file/d/1L2tY4BiOWwSm7toYpCxmLvDLf6x0IrJ4/view?usp=drive_link

## 使用技術
Arduino, 電子回路, MIDI

## 制作時期
2024年冬

## 特徴
1. ヘッドにゴム素材を採用し、静音性を向上
2. ４つの入力センサ(圧電素子：中心に一つ、円周上に三つ)
3. ピッチ変更フェーダを採用
4. ゲイン検出システムの改良：音量を256段階で表現
5. 打点位置(半径方向)検出システムの改良：中心からの距離を256段階で表現
6. ヘッドの中心と端で異なるサウンドを発音
7. トリガリングと音声出力をそれぞれ別のArduinoで行う(MIDI信号でタイミング・ゲイン・打点位置を送受)

## 改善点・今後の展望
1. 音質の改善：メモリ容量のより大きいマイコンを用いる、ノイズ対策を行う
2. 入力装置の改善：センサをより高精度にする、ヘッド・シェルの機構を見直す
3. トリガーの改善：細かいストロークやゴーストノートも繊細に反映できるようにする
