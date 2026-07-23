# crosspointTH

เฟิร์มแวร์อ่านหนังสือภาษาไทยสำหรับ Xteink X3/X4 ปรับแก้และดูแลรุ่นนี้โดย
**JTIAPBN.Ai** โดยพัฒนาต่อยอดจากโครงการโอเพนซอร์ส
[CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader)

> `crosspointTH` เป็น community fork ไม่ใช่ release ทางการของ CrossPoint Reader

## ความสามารถภาษาไทย

- ฟอนต์ Noto Sans/Serif ที่ฝัง glyph ไทยไว้ในเฟิร์มแวร์ และ fallback อัตโนมัติเมื่อฟอนต์บน SD card ไม่มีภาษาไทย
- ตัดคำไทยด้วยพจนานุกรมโดยไม่แยกพยัญชนะ สระ และวรรณยุกต์ออกจากกัน
- จัดตำแหน่งสระบนและวรรณยุกต์ซ้อนสองชั้น เช่น `อึ่` `อื้อ` `ปึ้` และ `อ่ำ`
- เร่งการทำ index ของไฟล์ `.txt`/`.md` ภาษาไทยด้วย parser แบบช่วงข้อความและคืนเวลาให้ระบบ
  ระหว่างจัดบรรทัดยาว เพื่อไม่ให้เครื่องดูเหมือนค้าง
- อ่าน `.epub`, `.txt` และ `.md` ภาษาไทย พร้อมเมนูภาษาไทย
- Markdown รองรับ heading, ตัวหนา, ตัวเอียง, inline code, list, quote, link text และ pipe table
  โดยแสดงแต่ละแถวเป็นข้อมูลเรียงลงมาพร้อมชื่อคอลัมน์ เพื่อให้อ่านง่ายบนจอขนาดเล็ก
- ใช้ค่าเดิมใน **Settings → Reader** สำหรับ line spacing และ paragraph alignment;
  โหมด Justified เพิ่มระยะไม่เกิน 1 พิกเซลต่อขอบเขตคำไทย เพื่อคงช่องไฟธรรมชาติของ Sarabun
- เมนู **Reading Stats / สถิติการอ่าน** ใน reader แสดงจำนวนครั้ง เวลาอ่าน หน้าที่อ่านไปข้างหน้า
  และหนังสือที่อ่านจบ โดยบันทึกลง SD เฉพาะตอนออกจากหนังสือ

รายละเอียดเชิงเทคนิคและกรณีทดสอบอยู่ใน [docs/THAI_SUPPORT.md](./docs/THAI_SUPPORT.md)

## การติดตั้ง

ใช้ไฟล์ `crosspointTH-firmware.bin` จาก GitHub Releases แล้วเลือกวิธีใดวิธีหนึ่ง:

1. **SD card (แนะนำเมื่อมี CrossPoint อยู่แล้ว):** เปลี่ยนชื่อไฟล์เป็น `firmware.bin`, วางที่ root
   ของ SD card แล้วเลือก **Settings → System → SD Card Firmware Update**
2. **USB:** ใช้ custom firmware ใน web flasher ของ CrossPoint Reader โดยเลือกอุปกรณ์ X3/X4 ให้ถูกต้อง

ห้ามตัดไฟ ปิดเครื่อง หรือถอดสายระหว่างเขียนเฟิร์มแวร์ และไม่ควรเขียนไฟล์ app image นี้ที่ offset
อื่นนอกจาก `0x10000`

## เลือกภาษาและปรับการอ่าน

- **Settings → System → Language → ไทย**
- **Settings → Reader → Font Family** เลือกฟอนต์; หากฟอนต์ SD ไม่มี glyph ไทย ระบบจะใช้ Noto builtin
- **Reader Line Spacing** เลือก Tight / Normal / Wide
- **Reader Paragraph Alignment** เลือก Justified / Left / Center / Right

ไม่เพิ่มค่า “ระยะระหว่างตัวอักษรไทย” แยกต่างหาก เพราะสระและวรรณยุกต์เป็นส่วนหนึ่งของ cluster
เดียวกับพยัญชนะ การยืดภายใน cluster ทำให้ภาษาไทยผิดรูปได้

## การย้อนกลับรุ่นทางการ

ดาวน์โหลด `firmware.bin` จาก
[CrossPoint Reader Releases](https://github.com/crosspoint-reader/crosspoint-reader/releases) แล้ว flash
ผ่าน SD card update หรือ USB custom firmware แบบเดียวกับตอนติดตั้ง `crosspointTH`

- partition table และ bootloader ไม่ได้ถูกเปลี่ยน
- การตั้งค่าและความคืบหน้าการอ่านใช้โครงสร้าง upstream เดิม
- cache layout ของหนังสืออาจถูกสร้างใหม่เมื่อสลับรุ่น แต่ไม่ลบ bookmark หรือความคืบหน้า
- OTA ในตัวเครื่องยังตรวจ release ทางการของ upstream; รุ่นทางการที่มีเลข patch ใหม่กว่าจะถูกเสนอได้ตามปกติ

## ก่อนเผยแพร่ทุก release

ผู้ดูแลต้องทำตาม [Release safety checklist](./docs/THAI_SUPPORT.md#release-safety-checklist), รันชุดทดสอบ,
ตรวจชนิด image/ขนาด partition และเผยแพร่ SHA-256 ของไฟล์จริง ห้ามเผยแพร่ build ที่มีการเปลี่ยน
bootloader, partition table, HAL, power manager หรือ display driver โดยไม่มีการตรวจ hardware แยกต่างหาก

## เครดิตและสัญญาอนุญาต

Copyright และเครดิตต้นฉบับยังเป็นของผู้ร่วมพัฒนา CrossPoint Reader ตามไฟล์ [LICENSE](./LICENSE)
การปรับแก้รุ่นภาษาไทยนี้ระบุผู้ดูแลเป็น **JTIAPBN.Ai** ดูรายละเอียดใน [ATTRIBUTION.md](./ATTRIBUTION.md)
