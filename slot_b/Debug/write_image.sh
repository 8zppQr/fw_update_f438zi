openocd \
  -f interface/stlink.cfg \
  -f target/stm32f4x.cfg \
  -c "init" \
  -c "reset init" \
  -c "flash write_image erase slot_b_signed.bin 0x080C0000" \
  -c "verify_image slot_b_signed.bin 0x080C0000" \
  -c "reset run" \
  -c "exit"
