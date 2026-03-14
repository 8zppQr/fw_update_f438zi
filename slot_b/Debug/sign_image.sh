imgtool sign \
  -k root-ec-p256.pem \
  --header-size 0x200 \
  --pad-header \
  --align 4 \
  --slot-size 0x80000 \
  --version 1.0.0 \
  slot_b.bin slot_b_signed.bin
