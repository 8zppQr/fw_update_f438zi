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

## 使い方
ブートローダーとスロットAについてはCubeIDE上からそのまま**Run as**を押して書き込みをすれば動作します。

スロットBについては鍵生成とファームウェアイメージへの署名およびアドレスを指定して書き込む必要があります。

```sh
cd slot_b/Debug

bash generate_key.sh #署名・検証用の公開鍵ペアを作成

bash sign_image.sh #CubeIDE上で生成されたバイナリ(slot_b.bin)に対して署名をします(slot_b_signed.bin)。

bash write_image.sh #スロットBのアドレスを指定して署名されたFWイメージを書き込みます。
```

またPC側でHTTPサーバーを立ち上げます。
```sh
cd http_server
bash launch_http_server.sh
```

## UARTログ
最初に起動するとブートローダーが立ち上がります。ブートローダー側でスロットBのイメージヘッダーおよびTLVを読み込みます。読み込み後、TLVに付与されているFWイメージのハッシュ値(Image hash)について、ブートローダーが再計算して一致するか確認しています。その後、署名検証に入りますがその前にどの公開鍵で検証するかを特定するために、検証鍵のハッシュ値(Public key hash)を確認します。その後、特定された検証鍵でFWイメージ署名(Image signature)を検証します。

```
bootloader started!
=== Image header @ 0x080C0000 ===
magic:              0x96F3B83D
load_addr:          0x00000000
hdr_size:           0x0200
protected_tlv_size: 0x0000
img_size:           0x0000468C
flags:              0x00000000
version:            1.0.0+0
vector_addr:         0x080C0200
vector[0] MSP:       0x20030000
vector[1] ResetHdlr: 0x080C10F5
=== Image TLV dump ===
TLV start:            0x080C488C
TLV magic:            0x6907
TLV total size:       0x0097
TLV type:             0x0010 (SHA256)
TLV len:              0x0020
TLV type:             0x0001 (KEYHASH)
TLV len:              0x0020
TLV type:             0x0022 (ECDSA256)
TLV len:              0x0047
=== Image hash verify ===
Expected SHA256:
200b2be77a49a95dfa115ac5fca44d0c0d52f49e3a67d1b31be2f45ec8a594de
Calculated SHA256:
200b2be77a49a95dfa115ac5fca44d0c0d52f49e3a67d1b31be2f45ec8a594de
HASH MATCH
image_verify_hash: OK
=== Public key hash verify ===
Expected KEYHASH:
3c4d8538849977fbadb6211beb6a69f050c83b383473aeceda134de6cc3c84af
Calculated KEYHASH:
3c4d8538849977fbadb6211beb6a69f050c83b383473aeceda134de6cc3c84af
KEYHASH MATCH
image_verify_keyhash: OK
=== Image signature verify ===
ECDSA DER signature:
3045022100cbe7d7044bd473525a94428ddf521d2044160a50c01b5fb3304694b9f5d44194022053a13290cf672ea11863ee32310b78ac3a7f604ee83741d6c654fcb0b87caac9
ECDSA raw r||s:
cbe7d7044bd473525a94428ddf521d2044160a50c01b5fb3304694b9f5d4419453a13290cf672ea11863ee32310b78ac3a7f604ee83741d6c654fcb0b87caac9
Image hash:
200b2be77a49a95dfa115ac5fca44d0c0d52f49e3a67d1b31be2f45ec8a594de
SIGNATURE VERIFY OK
image_verify_signature: OK
Select number:
1:Boot slot A
2:Boot slot B
```

スロットAを起動すると、HTTPサーバーから**HTTP GET**をしてダウンロードをします。ダウンロードしたイメージはスロットBに書き込みます。書き込みが終了したら再起動が自動で始まります。
```
Booting SLOT A
jumping to 0x08040000...
Slot A is booted.
DHCP state = 6
DHCP state = 1
DHCP state = 8
DHCP state = 10
DHCP IP acquired!
IP: 10.14.1.128
MASK: 255.255.0.0
GW: 10.14.11.30
http_client_start() called, state=0
Connecting to 10.14.1.111:8000 ...
tcp_connect ret = 0
assigned local port = 52432
connected cb err=0
Connected. Sending HTTP GET...
HTTP header parsed
HTTP/1.0 200 OK
Server: SimpleHTTP/0.6 Python/3.14.0
Date: Sat, 14 Mar 2026 06:09:20 GMT
Content-type: application/octet-stream
Content-Length: 18723
Last-Modified: Sat, 14 Mar 2026 06:08:36 GMT

Content-Length = 18723
Erasing flash: sector 10 -> 10

--- server closed connection ---
Download complete: 18723 / 18723 bytes
Image written to 0x080C0000
Rebooting...
```

スロットBへの書き込みができたら、実際にブートローダーからスロットBを起動してみます。起動後、約4秒経過すると再起動がかかります。これはIWDGというウォッチドッグタイマを利用しており、ウォッチドッグタイマを初期化後にキックしないため自動で再起動する仕組みになっています。今回は簡易的なアップデートの学習のため、あまり必要性はありませんが実際にはファームウェアのダウンロードや書き込みがうまくいかなかったときのためのロールバックの機構として使われることが多いと思います。
```
Select number:
1:Boot slot A
2:Boot slot B
Booting SLOT B
jumping to 0x080C0200...
Slot B is booted(Updated).
~ 1 second passed 
~ 2 second passed
~ 3 second passed //ウォッチドッグタイマによる再起動
bootloader started! //再起動している。
```