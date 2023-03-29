# Numerito
ESP32 Arduino Project in 01204322 Embedded System. Kesetsart U.

<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw==" crossorigin="anonymous" referrerpolicy="no-referrer" />

#  **ที่มาและความสำคัญ** <i class="fa-solid fa-arrow-down-1-9 fa-bounce" ></i> 

---
&emsp;ปัจจุบันบอร์ดเกมเป็นที่นิยมอย่างมากทั้งการเล่นเพื่อความบันเทิง ความสร้างสรรค์ และการเข้าสังคม บอร์ดเกมดังกล่าวมีหลากหลายประเภทอย่างมาก กลุ่มของเราจึงพยายามค้นหาบอร์ดเกมที่เข้าใจง่ายและสามารถนำมาประยุกต์กับความรู้แและประสบการณ์กับเรื่องที่เรียนมาเพื่อให้บอร์ดเกมดังกล่าวมีการเล่นแปลกใหม่มากขึ้น

# **รายละเอียด** <i class="fa-solid fa-circle-exclamation fa-bounce"></i>
---
&emsp;โครงงานนี้เป็นผลงานที่ได้แรงบันดาลใจมาจากบอร์ดเกมหนึ่ง ซึ่งทางกลุ่มของเราได้นำมาประยุกต์เข้ากับความรู้ที่ได้เรียนไป โดยจะมีการเพิ่มอุปกรณ์ที่เป็นฮาร์ดแวร์เป็นอุปกรณ์ที่จับต้องได้สำหรับการทำกิจกรรมภายในเกม อุปกรณ์และกติกาของเกมจะเป็นดังนี้ เกมเรียงลำดับเลข แจกการ์ดหมายเลข 1 ถึง 6 และ 9 (7 ใบ) ให้ผู้เล่นแต่ละคน โดยจะเริ่มจากการโชว์หมายเลขจำนวน 4 หลัก [7 segment] ที่จอหลักตรงกลาง พร้อมกับจอ OLED แสดงสี โดยจะมี 2 สี คือ น้ำเงิน และแดง ผู้เล่นต้องนำการ์ดไปแตะที่เครื่องอ่าน RFID ที่อยู่บนบอร์ดของตนเอง โดยเรียงตามหมายเลขที่ปรากฎบนจอ ถ้าไฟเป็นสีน้ำเงินให้เรียงการ์ดหมายเลขจากหน้าไปหลัง แต่ถ้าไฟเป็นสีแดงให้เรียงการ์ดหมายเลขจากหลังไปหน้า เมื่อเรียงเสร็จให้กดปุ่มบนบอร์ด และผู้เล่นคนอื่นสามารถกดปุ่มเพื่อขอตรวจสอบลำดับการ์ดที่ลงไปได้ และจะแสดงผลลัพธ์ว่าถูกต้องหรือไม่ที่จอ OLED ตรงกลาง และมีไฟ LED บนบอร์ดผู้เล่นเพื่อนับคะแนน ผู้เล่นจะได้รับคะแนนจากกรณีต่อไปนี้ 1. เรียงการ์ดและกดปุ่มเป็นคนแรก โดยไม่มีคนขอตรวจสอบ 2. เรียงการ์ดและกดปุ่มเป็นคนแรก มีคนขอตรวจสอบ แต่ลำดับการ์ดถูก ส่วนกรณีที่ลำดับการ์ดผิดจะโดนหักคะแนน และผู้ที่ขอตรวจสอบจะเป็นฝ่ายได้คะแนนไป


## &thinsp; **ฟีเจอร์ที่สำคัญ** 
- มีบอร์ดกลางสำหรับการสุ่มเลขและไฟ LED เพื่อเป็นฟังก์ชันสำหรับการเรียงเลข
- จอ LCD บอกสถานะระหว่างดำเนินเกม
- ESP-NOW สำหรับการเชื่อมต่อไร้สาย
- กล่องควบคุมผู้เล่นทั้งหมด 3กล่อง มีRFIDสำหรับการอ่านการ์ดในแต่ละกล่อง

## &thinsp; **แนวคิดและหลักการ**
&emsp;ทุกบอร์ดที่ใช้จะมี <span style="color:#ff00ff">**ESP32 (NodeMCU)** </span>สำหรับการสื่อสารระหว่างบอร์ดแบบไร้สาย (Wireless) โดยใช้ <span style="color:olive"> **ESP-NOW** </span>โดยบอร์ดจะมี 3 ประเภท คือ บอร์ดกลางสำหรับการสุ่มเลข , บอร์ดกลางสำหรับแสดงสถานะผู้เล่น , บอร์ดผู้เล่น 3 บอร์ด โดยใช้ <span style="color:teal">**ArduinoIDE**</span> ในการพัฒนา

 1. **บอร์ดกลางสำหรับการสุ่มเลข**
	- **Source Code :** [7segmentCode.ino](https://github.com/noeyiue/Numerito/blob/main/project/7segmentcode/7segmentcode.ino)
	-	**Library**
		- esp_now.h 
		- WiFi.h
		- SevSeg.h
	- **อุปกรณ์ที่ใช้ในส่วนของ Hardware**
		- Picture: 
			- NodeMCU ESP32 [1] 
			- 7-Segment 4 Digit [1]
			- KY-016 RGB LED [1] : แสดง mode เกม โดยจะประกอบไปด้วย 2 สี ดังนี้
                - <span style="color:blue"> สีน้ำเงิน </span> เรียงเลขจากหน้าไปหลัง
                - <span style="color:red"> สีแดง </span> เรียงเลขจากหลังไปหน้า 
			- Resistor 330 Ω [7]
		
        <div style="text-align:center">
            <img src="https://user-images.githubusercontent.com/86821757/228429246-82fead77-0b90-4994-bed5-f36c2ad5c70d.jpg" width=50% height=50% title="7Segment Schematic">
            <p>7-Segment Board Schematic</p> 
        </div>

		
 2. **บอร์ดกลางสำหรับแสดงสถานะผู้เล่น**
	- **Source Code :** [MainBoardCode.ino](https://github.com/noeyiue/Numerito/blob/main/project/maincode/maincode.ino)
	- **Library**
		- MFRC522.h
		- Adafruit_GFX.h
		- Adafruit_SSD1306.h
		- SPI.h
		- esp_now.h
		- WiFi.h
	- **อุปกรณ์ที่ใช้ในส่วนของ Hardware**
		- Picture: 
			- NodeMCU ESP32 [1] 
			- จอ LCD 16x2 with I2C [1] : แสดงสถานะโดยรวมของเกม
			- KY-016 RGB LED  [3] : แสดงสถานะของแต่ละผู้เล่น โดยมีทั้งหมด 3 สถานะดังนี้
                - <span style="color:lime"> สีเขียว </span> บอกถึงสถานะว่าผู้เล่นพร้อมแล้ว
                - <span style="color:red"> สีแดง </span> บอกถึงสถานะผู้เล่นว่ากดส่งเลขเร็วที่สุด
                - <span style="color:blue"> สีน้ำเงิน </span> บอกถึงสถานะผู้เล่น 2 คนที่แข่งกันในช่วง Challenge
			- Push button [1] : สำหรับ Reset 
		
        <div style="text-align:center">
            <img src="https://user-images.githubusercontent.com/86821757/228429398-b5b1e737-1c9b-409b-a684-56cdf95c8652.jpg" width=50% height=50% title="Status Schematic">
            <p>Status Board Schematic</p> 
        </div>

 3. **บอร์ดผู้เล่น 3 บอร์ด (3 ผู้เล่น)**
    - **Source Code :** [Player Code.ino](https://github.com/noeyiue/Numerito/blob/main/project/playercode/playercode.ino)
	- **Library**
		- MFRC522.h
		- Adafruit_GFX.h
		- Adafruit_SSD1306.h
		- SPI.h
		- esp_now.h
		- WiFi.h
	- **อุปกรณ์ที่ใช้ในส่วนของ Hardware**
		 - Picture: 
			- NodeMCU ESP32 [3]
			- RC522 RFID Card Read/Write [3] : สำหรับการอ่านการ์ด
			- Button [3] : สำหรับกดยืนยัน
			- OLED I2C 128x64 [3] : สำหรับแสดงคะแนนผู้เล่น 
			- RFID Card [21] : สำหรับการเล่นเกม 
            <div style="text-align:center">
                <img src="https://user-images.githubusercontent.com/86821757/228429403-9e5e85a2-30a7-4dec-b8e2-e1f2b14ffa92.jpg" width=50% height=50% title="Player Schematic">
            </div>
            <p style="text-align:center">Player Schematic</p> 
### [<i class="fa-brands fa-github"></i> **github** ]()
