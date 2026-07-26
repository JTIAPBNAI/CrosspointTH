# Crosspoint v.1.5.0-TH.1.1

เฟิร์มแวร์อ่านหนังสือภาษาไทยสำหรับ Xteink X3/X4 ปรับแก้และดูแลรุ่นนี้โดย
**JTIAPBN.Ai** โดยพัฒนาต่อยอดจากโครงการโอเพนซอร์ส
[CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader)

> `crosspointTH` เป็น community fork ไม่ใช่ release ทางการของ CrossPoint Reader

## ความสามารถภาษาไทย

- ฟอนต์ Noto Sans/Serif ที่ฝัง glyph ไทยไว้ในเฟิร์มแวร์ และ fallback อัตโนมัติเมื่อฟอนต์บน SD card ไม่มีภาษาไทย
- ตัดคำไทยด้วยพจนานุกรมโดยไม่แยกพยัญชนะ สระ และวรรณยุกต์ออกจากกัน
- จัดตำแหน่งสระบนและวรรณยุกต์ซ้อนสองชั้น เช่น `อึ่` `อื้อ` `ปึ้` และ `อ่ำ`
- เปิดหน้าแรกของไฟล์ `.txt`/`.md` ก่อน แล้วทำ index ทีละหน้าเบื้องหลัง พร้อมแสดงเปอร์เซ็นต์จริง
  ตามตำแหน่ง byte และประมาณจำนวนหน้า; ลดการวัดทั้งบรรทัดซ้ำเพื่อไม่ให้ไฟล์ภาษาไทยบรรทัดยาวดูเหมือนค้าง
- เปิด EPUB และสร้าง index แบบ incremental, ทำส่วนที่เหลือเบื้องหลัง และดึงรูปเมื่อเปิดถึงหน้า
  เพื่อลดเวลารอและ peak memory
- อ่าน `.epub`, `.txt` และ `.md` ภาษาไทย พร้อมเมนูภาษาไทย
- Markdown รองรับ heading, ตัวหนา, ตัวเอียง, inline code, list, quote, link text และ pipe table
  โดยแสดงแต่ละแถวเป็นข้อมูลเรียงลงมาพร้อมชื่อคอลัมน์ เพื่อให้อ่านง่ายบนจอขนาดเล็ก
- ตารางใน EPUB ใช้ชื่อคอลัมน์จริงจาก `<th>` แทน `Tab Row …, Cell …`; ถ้า EPUB ไม่มีหัวตาราง
  ระบบจะแสดง `Column N`
- ใช้ค่าเดิมใน **Settings → Reader** สำหรับ line spacing และ paragraph alignment;
  โหมด Justified เพิ่มระยะไม่เกิน 1 พิกเซลต่อขอบเขตคำไทย เพื่อคงช่องไฟธรรมชาติของ Sarabun
- เมนู **Reading Stats / สถิติการอ่าน** ใน reader แสดงจำนวนครั้ง เวลาอ่าน หน้าที่อ่านไปข้างหน้า
  และหนังสือที่อ่านจบ โดยบันทึกลง SD เฉพาะตอนออกจากหนังสือ

รายละเอียดเชิงเทคนิคและกรณีทดสอบอยู่ใน [docs/THAI_SUPPORT.md](./docs/THAI_SUPPORT.md)

## ข้อแนะนำสำหรับไฟล์ Markdown ขนาดใหญ่

หลังเปิดไฟล์ `.md` กรุณารอให้สถานะ **Indexing ถึง 100%** ก่อนเปลี่ยนหน้าต่อเนื่อง ระหว่างทำ index
เครื่องอาจตอบสนองต่อปุ่มเปลี่ยนหน้าช้าชั่วคราว โดยเฉพาะส่วนที่มีบรรทัดยาว ตาราง หรือภาษาไทยจำนวนมาก
เปอร์เซ็นต์อ้างอิงตำแหน่งข้อมูล จึงเป็นเรื่องปกติที่ช่วง 80–90% อาจใช้เวลามากกว่าช่วงอื่น หากตัวเลข
ยังเพิ่มขึ้นแสดงว่าเครื่องยังทำงานอยู่ และเมื่อครบ 100% การเปิดไฟล์ครั้งถัดไปจะเร็วขึ้นจาก index ที่บันทึกไว้

หากเป็นไฟล์ขนาดใหญ่ แนะนำให้ใช้
[XTEINK EPUB Optimizer](https://xteinkepuboptimizer.whisperingweekends.com/) ซึ่งรองรับทั้ง:

- แปลง Markdown `.md` เป็น EPUB พร้อมจัดหัวข้อ ย่อหน้า และตารางให้เหมาะกับอุปกรณ์
- optimize ไฟล์ `.epub` ที่มีอยู่แล้ว เพื่อลดขนาด ปรับรูปภาพ และจัดโครงสร้างตารางสำหรับ XTEINK

## การติดตั้ง

ใช้ไฟล์ `crosspointTH-firmware.bin` จาก GitHub Releases แล้วเลือกวิธีใดวิธีหนึ่ง:

1. **SD card (แนะนำเมื่อมี CrossPoint อยู่แล้ว):** เปลี่ยนชื่อไฟล์เป็น `firmware.bin`, วางที่ root
   ของ SD card แล้วเลือก **Settings → System → SD Card Firmware Update**
2. **USB:** ใช้ custom firmware ใน web flasher ของ CrossPoint Reader โดยเลือกอุปกรณ์ X3/X4 ให้ถูกต้อง

ห้ามตัดไฟ ปิดเครื่อง หรือถอดสายระหว่างเขียนเฟิร์มแวร์ และไม่ควรเขียนไฟล์ app image นี้ที่ offset
อื่นนอกจาก `0x10000`

## เลือกภาษาและปรับการอ่าน

- **Settings → System → Language → ไทย**
- **Settings → Reader → Text Settings → Font** เลือกฟอนต์; หากฟอนต์ SD ไม่มี glyph ไทย ระบบจะใช้ Noto builtin
- **Text Settings → Layout → Line Spacing** เลือก Tight / Normal / Wide
- **Text Settings → Alignment** เลือก Justified / Left / Center / Right

ไม่เพิ่มค่า “ระยะระหว่างตัวอักษรไทย” แยกต่างหาก เพราะสระและวรรณยุกต์เป็นส่วนหนึ่งของ cluster
เดียวกับพยัญชนะ การยืดภายใน cluster ทำให้ภาษาไทยผิดรูปได้

## การย้อนกลับรุ่นทางการ

ดาวน์โหลด `firmware.bin` จาก
[CrossPoint Reader Releases](https://github.com/crosspoint-reader/crosspoint-reader/releases) แล้ว flash
ผ่าน SD card update หรือ USB custom firmware แบบเดียวกับตอนติดตั้ง `crosspointTH`

- partition table และ bootloader ไม่ได้ถูกเปลี่ยน
- SD updater ตรวจ header, segment, checksum และ SHA-256 ก่อนเขียน จากนั้นอ่าน flash กลับมาเทียบทุก block
  ก่อนเปลี่ยน boot slot; หากตรวจไม่ผ่าน firmware เดิมยังเป็น boot target
- การตั้งค่าและความคืบหน้าการอ่านใช้โครงสร้าง upstream เดิม
- cache layout ของหนังสืออาจถูกสร้างใหม่เมื่อสลับรุ่น แต่ไม่ลบ bookmark หรือความคืบหน้า
- OTA ในตัวเครื่องยังตรวจ release ทางการของ upstream; รุ่นทางการที่มีเลข patch ใหม่กว่าจะถูกเสนอได้ตามปกติ

ถ้า firmware ใหม่เริ่มถึง `setup()` แต่เข้า UI ไม่ได้ และ `ota_0` ยังเก็บ firmware เดิมที่ใช้ได้ ให้กด
**Back + Up ค้างระหว่างกด Reset** เพื่อสลับกลับ หาก image เสียก่อนถึง `setup()` หรือ `ota_0` ถูกเขียนทับแล้ว
ให้ใช้ USB recovery; ปุ่มลัดนี้ไม่ใช่การรับประกัน rollback อัตโนมัติสำหรับทุกความเสียหาย

## ก่อนเผยแพร่ทุก release

ผู้ดูแลต้องทำตาม [Release safety checklist](./docs/THAI_SUPPORT.md#release-safety-checklist), รันชุดทดสอบ,
ตรวจชนิด image/ขนาด partition และเผยแพร่ SHA-256 ของไฟล์จริง ห้ามเผยแพร่ build ที่มีการเปลี่ยน
bootloader, partition table, HAL, power manager หรือ display driver โดยไม่มีการตรวจ hardware แยกต่างหาก

## เครดิตและสัญญาอนุญาต

Copyright และเครดิตต้นฉบับยังเป็นของผู้ร่วมพัฒนา CrossPoint Reader ตามไฟล์ [LICENSE](./LICENSE)
การปรับแก้รุ่นภาษาไทยนี้ระบุผู้ดูแลเป็น **JTIAPBN.Ai** ดูรายละเอียดใน [ATTRIBUTION.md](./ATTRIBUTION.md)
