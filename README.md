# crosspointTH

เฟิร์มแวร์อ่านหนังสือภาษาไทยสำหรับ **Xteink X3/X4** ปรับแก้และดูแลโดย
**JTIAPBN.Ai** เพื่อให้ภาษาไทยแสดงผล ตัดคำ และจัดหน้าได้เหมาะกับจอ e-ink มากขึ้น

> `crosspointTH` เป็นรุ่นชุมชนที่พัฒนาต่อยอดจาก CrossPoint Reader และไม่ใช่รุ่นทางการของ
> CrossPoint Reader

## ดาวน์โหลดรุ่นภาษาไทย

รุ่นปัจจุบัน: **v1.4.1-th.2 (Pre-release)**

- [ดาวน์โหลด crosspointTH-firmware.bin โดยตรง](https://github.com/JTIAPBNAI/CrosspointTH/releases/download/v1.4.1-th.2/crosspointTH-firmware.bin)
- [ดูรายละเอียด ไฟล์ตรวจสอบ SHA-256 และไฟล์ประกอบทั้งหมด](https://github.com/JTIAPBNAI/CrosspointTH/releases/tag/v1.4.1-th.2)

รุ่นนี้ยังระบุเป็น **Pre-release** เพราะยังต้องเก็บผลทดสอบบนเครื่อง X3/X4 จริงเพิ่มเติม GitHub จึงอาจ
ไม่แสดงรุ่นนี้ในช่อง “Latest release” แม้ไฟล์ดาวน์โหลดจะเผยแพร่แล้ว ไม่ควรเปลี่ยนสถานะเป็น Stable
จนกว่าจะผ่านการทดสอบบนฮาร์ดแวร์จริงครบถ้วน

SHA-256 ของไฟล์เฟิร์มแวร์:

```text
9376157219d76fd2b9dfc3de1d0f4077c5a5210eee814e8138a876e9eb7b27e6  crosspointTH-firmware.bin
```

## จุดเด่นของ crosspointTH

- มีฟอนต์ Noto Sans/Serif ที่ครอบคลุม glyph ภาษาไทยในเฟิร์มแวร์
- fallback ไปใช้ฟอนต์ builtin โดยอัตโนมัติเมื่อฟอนต์จาก SD card ไม่มี glyph ภาษาไทย
- ตัดคำไทยด้วยพจนานุกรม โดยไม่แยกพยัญชนะ สระ และวรรณยุกต์ออกจาก cluster เดียวกัน
- จัดตำแหน่งสระบนและวรรณยุกต์ซ้อนหลายชั้น เช่น `อึ่` `อื้อ` `ปึ้` และ `อ่ำ`
- ลดงานซ้ำขณะสร้าง index ของไฟล์ `.txt` และ `.md` ภาษาไทย
- แสดง Markdown แบบมี heading, ตัวหนา, ตัวเอียง, inline code, list, quote และข้อความลิงก์
- ใช้การจัดแนวย่อหน้าและระยะบรรทัดจาก Reader Settings กับไฟล์ TXT/Markdown
- จำกัดการขยายแบบ justified ไว้ไม่เกิน 1 พิกเซลต่อขอบเขตคำไทย และไม่ยืดภายใน glyph cluster
- มีสถิติการอ่านแบบ lightweight สำหรับ EPUB: จำนวนครั้ง เวลาอ่าน หน้าที่อ่านไปข้างหน้า
  และหนังสือที่อ่านจบ โดยเขียนลง SD เมื่อออกจาก reader แทนการเขียนทุกหน้า

รายละเอียดการทำงานและกรณีทดสอบอยู่ใน
[เอกสารระบบภาษาไทย](./docs/THAI_SUPPORT.md) และ
[ผลการประเมินฟีเจอร์จาก CrossInk](./docs/CROSSINK_REVIEW.md)

## วิธีติดตั้ง

### อัปเดตผ่าน SD card

เหมาะสำหรับเครื่องที่ใช้งาน CrossPoint อยู่แล้ว

1. ดาวน์โหลด `crosspointTH-firmware.bin`
2. ตรวจสอบ SHA-256 ให้ตรงกับค่าที่เผยแพร่
3. เปลี่ยนชื่อไฟล์เป็น `firmware.bin` แล้ววางไว้ที่ root ของ SD card
4. เลือก **Settings → System → SD Card Firmware Update**

### ติดตั้งผ่าน USB

ใช้หน้า web flasher ของ CrossPoint Reader เลือก X3 หรือ X4 ให้ตรงกับเครื่อง จากนั้นเลือก
**Custom .bin** และระบุไฟล์ `crosspointTH-firmware.bin`

> ห้ามตัดไฟ ปิดเครื่อง หรือถอดสายระหว่างเขียนเฟิร์มแวร์ และห้ามเขียน app image นี้ที่ offset
> อื่นนอกจาก `0x10000` หากไม่เข้าใจเรื่อง offset ให้ใช้ SD card updater หรือ web flasher เท่านั้น

## ตั้งค่าภาษาไทย

- เลือก **Settings → System → Language → ไทย**
- เลือกฟอนต์ที่ **Settings → Reader → Font Family** หากฟอนต์ SD ไม่มีภาษาไทย ระบบจะ fallback
  ไปใช้ Noto builtin
- ปรับ **Reader Line Spacing** เป็น Tight / Normal / Wide
- ปรับ **Reader Paragraph Alignment** เป็น Justified / Left / Center / Right

ไม่มีการเพิ่มระยะห่าง “ระหว่างตัวอักษรไทย” แบบอิสระ เพราะสระและวรรณยุกต์ต้องยึดกับพยัญชนะใน
cluster เดียวกัน การยืดระหว่างองค์ประกอบเหล่านี้ทำให้ภาษาไทยผิดรูปได้

## ความปลอดภัยและการย้อนกลับ

- รุ่นนี้คง partition table, bootloader, HAL, power manager, display driver และเส้นทาง flash/OTA
  ตามฐาน upstream
- เฟิร์มแวร์ release ผ่าน safety gate, unit tests, static checks และการตรวจชนิด/ขนาด image
- ก่อนติดตั้งควรเก็บ firmware ทางการไว้หนึ่งชุด และสำรองไฟล์สำคัญใน SD card
- สามารถย้อนกลับด้วยไฟล์ `firmware.bin` จาก
  [CrossPoint Reader Releases](https://github.com/crosspoint-reader/crosspoint-reader/releases)
  ผ่าน SD card updater หรือ USB custom firmware แบบเดียวกัน

ไม่มีเฟิร์มแวร์ใดรับประกันความเสี่ยงจากไฟดับ สายหลุด เลือกรุ่นอุปกรณ์ผิด หรือฮาร์ดแวร์เสียหายเดิมได้
ผู้ใช้รุ่น Pre-release ควรทดสอบอย่างระมัดระวังและรายงานรุ่นเครื่อง ขั้นตอน และ log เมื่อพบปัญหา

## เอกสาร

- [คู่มือภาษาไทยฉบับย่อ](./README-TH.md)
- [รายละเอียดระบบภาษาไทยและ release safety checklist](./docs/THAI_SUPPORT.md)
- [รายการเปลี่ยนแปลงของ crosspointTH](./CHANGELOG-crosspointTH.md)
- [เครดิตและที่มาของโครงการ](./ATTRIBUTION.md)
- [สัญญาอนุญาตของฟอนต์และข้อมูล third-party](./THIRD_PARTY_NOTICES.md)

## การพัฒนาและเครดิต

ผู้ปรับแก้และดูแลรุ่นภาษาไทย: **JTIAPBN.Ai**

โครงการนี้พัฒนาต่อยอดจาก
[crosspoint-reader/crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader) และเผยแพร่
ภายใต้ GNU GPL v3 ตามไฟล์ [LICENSE](./LICENSE) ลิขสิทธิ์และเครดิตของ upstream และ third-party
ยังคงเป็นของเจ้าของเดิมตาม [ATTRIBUTION.md](./ATTRIBUTION.md) และ
[THIRD_PARTY_NOTICES.md](./THIRD_PARTY_NOTICES.md)
