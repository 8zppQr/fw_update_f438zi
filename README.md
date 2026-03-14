# fw_update_f438zi
## 開発環境
- Windows 11
- STM32 CubeIDE Version: 2.0.0
- STM32 CubeMX Version: 6.16.1
- NUCLEO-F439ZI

## 構成
### bootloader(0x0800_0000)
キー入力で起動先(Slot A or Slot B)を選択できます。

### slot_a(0x0804_0000)
こちらでHTTPサーバーからバイナリをダウンロードして、Slot Bに書き込みます。ダウンロード終了後、再起動します。

### slot_b(0x080C_0000)
ダウンロードしたバイナリが書き込まれる先です。ウォッチドッグタイマを設定しているため、約4秒後に再起動します。

## ブートローダーの作成
STM32マイコンではFLASHの0x0800_0000から起動します。Cortex-Mのファームウェアにおいて、最初の4バイトがMain Stack Pointerで、次の4バイトがReset Handlerになります。このブートローダーからアプリケーションを起動するためには、アプリケーションの先頭から