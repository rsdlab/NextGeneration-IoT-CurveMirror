# 次世代IoTカーブミラー

## 概要
本製品は，SPRESENSEのDNNRT機能，HDRカメラ，GPS及びLTE通信拡張ボードを活かし**カーブミラーをIoT化**することを目的としています．
カメラを用いて見通しの悪い曲がり角や交差点の先を人の代わりに観測，人や車の存在をDNNRT機能で推論，結果をLTE通信で外部と情報を共有，そして**人や車の存在を周囲に分かりやすく通知**するシステムデバイスです．

本製品を使用する際は事前の学習データが必要となります．

現時点では，通信による外部との情報共有機能は未実装となります．

## 機能
- 推論モデルによる人と車の検出
- ライトを用いた検出結果の提示
- LTE通信を用いた情報共有(未実装)

## 使用方法
1. 環境セットアップ
    1. SPRESENSEのArduinoIDE開発環境セットアップを行います．(詳細は[SPRESENSE_Arduinoスタートガイド](https://developer.sony.com/develop/spresense/docs/arduino_set_up_ja.html)を参照)
    2. NeuralNetworkConsoreの環境セットアップを行います．

2. データセット作成
    1. SPRESENSEをPCに接続し,TakePicture.inoファイルを書き込みます.
    2. SPRESENSEへHDRカメラモジュール，SDカード，バッテリーを接続し，画像を撮影します．
    3. 撮った画像を "0None","1Human","2Vehicle" に仕分け，NNCを使用してデータセットを作成します．

3. 推論モデルの作成と書き込み
    1. NNCでデータセットを選択し，学習と評価を行います．
    2. 学習データからmodel.nnbファイルをエクスポートします．
    3. PCにSDカードを接続し，model.nnbファイルを書き込みます.

4. 推論システム
    1. SPRESENSEをPCに接続し,AIMirror.inoファイルを書き込みます.
    2. ライト，バッテリーなどの接続を行い，起動します
    3. 起動後，推論モデルによる人・車検出を行います．

    判定状況を確認する場合はディスプレイを接続してください

## 提案システム

![SystemImage](./images/system.png)

## ハードウェア概要

![HardwarOverviewImage](./images/Hardware_overview.png)

## その他
### プレゼンテーション資料

[Presentation_20221217.pdf](./Presentation_20221217.pdf)
