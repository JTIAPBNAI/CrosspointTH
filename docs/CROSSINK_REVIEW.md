# การประเมินฟีเจอร์จาก CrossInk สำหรับ crosspointTH

ตรวจเทียบกับ [uxjulia/CrossInk](https://github.com/uxjulia/CrossInk) ที่ commit
`b7f6708f96d05e5851f8bcfaaf57bc0e91dc0567` (v1.4.0) โดยให้ความสำคัญกับ RAM, flash,
จำนวนครั้งที่เขียน SD และความเข้ากันได้กับฐาน CrossPoint Reader เดิม

## สิ่งที่นำมาปรับใช้

### Reading stats แบบ lightweight

เก็บเฉพาะจำนวน session, เวลาอ่าน, forward pages และจำนวนหนังสือที่อ่านจบ ใช้ตัวนับใน activity
เดิมโดยไม่สร้าง FreeRTOS task หรือ network service เพิ่ม เวลาอ่านนับเฉพาะ page dwell ตั้งแต่ 2 วินาที
ถึง 10 นาที และ session ต้องมีเวลาอย่างน้อย 60 วินาที

ไฟล์ต่อหนังสือมีขนาด 14 bytes และไฟล์รวมมีขนาด 17 bytes เขียนผ่านไฟล์ `.tmp` แล้ว rename เมื่อ
ออกจาก reader จึงไม่เพิ่ม SD write ในทุก page turn และใช้ชื่อไฟล์เฉพาะ `crosspointTH` เพื่อไม่ชนกับ
รูปแบบสถิติของ CrossInk

## สิ่งที่ฐานปัจจุบันมีอยู่แล้ว

- ดัชนี spine ของ EPUB ขนาดใหญ่แบบ hash index แทนการค้นซ้ำแบบ O(n²)
- ลด allocation ใน `ParsedText` ด้วยการ reserve container และใช้ `string_view` ตอนแบ่งคำ
- ลด allocation ใน CSS parser และหลีกเลี่ยงการสแกน CSS ทั้ง ZIP สำหรับ EPUB ขนาดใหญ่
- image pixel cache แบบ streaming band จึงไม่ต้องมี full-image cache buffer ใน RAM
- วาง combining marks ด้วย font metrics และมี Thai shaping/fallback เพิ่มเติมเฉพาะใน crosspointTH
- incremental/windowed section building ของฐาน upstream รุ่นปัจจุบัน ซึ่งลดช่วงที่ UI ดูเหมือนค้าง

ดังนั้นไม่คัดลอก implementation เหล่านี้ซ้ำ เพราะจะเพิ่มความเสี่ยงจาก merge และ cache format โดยไม่
ได้ประโยชน์เพิ่ม

## สิ่งที่ยังไม่นำมาใส่ firmware

- Lexend Deca, Bitter, Inter และ emoji fonts: ไม่มี Thai glyph coverage ครบ และเพิ่ม flash มาก;
  ควรแจกเป็น `.cpfont` บน SD แยกจาก firmware หากต้องการ
- stats sync, reading history, charts และ stats sleep screen: เพิ่ม network/UI/cache surface หลายพัน
  บรรทัด ไม่เหมาะกับพื้นที่ app ที่เหลือน้อย
- Bionic Reading และ Guide Dots: ประโยชน์ต่อภาษาไทยยังไม่ชัด และเพิ่ม metadata ต่อคำใน EPUB cache
- EPUB render-mode fallback ทั้งชุด: เป็นการเปลี่ยน cache/layout pipeline ขนาดใหญ่ ต้องมี corpus และ
  hardware soak test แยกก่อน ไม่ควรผูกกับ release แก้ภาษาไทย
- Nearby progress/stats sync: เพิ่ม protocol และ network state โดยไม่ช่วยการเรนเดอร์ภาษาไทยโดยตรง

## แนวทาง EPUB ที่แนะนำโดยไม่เพิ่ม firmware

- แบ่งคลังหนังสือเป็นโฟลเดอร์ย่อยแทนการใส่หลายร้อยไฟล์ไว้ใน root เดียว
- ลดภาพความละเอียดสูงและ embedded font families ที่ไม่ใช้ก่อนคัดลอก EPUB ลง SD
- ใช้ EPUB Optimizer ในหน้า File Transfer ที่มีอยู่ในฐาน upstream
- ล้างเฉพาะ cache ของหนังสือเมื่อเปลี่ยน font/layout แทนการลบ `.crosspoint` ทั้งหมด
- ทดสอบ text-first EPUB, image-heavy EPUB, EPUB ที่มี chapter เดียวขนาดใหญ่ และ EPUB ไทยแบบไม่มี
  ช่องว่างทุกครั้งก่อนเลื่อนสถานะ release เป็น Stable
