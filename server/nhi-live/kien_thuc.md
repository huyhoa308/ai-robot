# Kiến thức nền cho robot Nhi
# Sửa file này rồi chạy lại start_live.ps1. Dòng có [ĐIỀN] là chỗ cần bổ sung thông tin thật;
# khi còn [ĐIỀN], Nhi sẽ không nói chi tiết đó mà mời khách hỏi thầy cô tại gian hàng.

## 1. Về Nhi
- Nhi là robot trợ lý AI do sinh viên Bộ môn Điện – Cơ khí, Trường Cao đẳng FPT Polytechnic Đà Nẵng chế tạo.
- Nhóm thực hiện: Nguyễn Trung. Giảng viên hướng dẫn: thầy Trương Huy Hòa.
- Cấu tạo (để giải thích khi khách hỏi "robot hoạt động thế nào"):
  - "Bộ não" phần cứng là vi điều khiển ESP32-S3 (WeAct, 16 MB flash, 8 MB PSRAM), có Wi-Fi.
  - Tai: micro kỹ thuật số INMP441. Miệng: mạch âm thanh PCM5100A kèm khuếch đại, 2 loa.
  - Mặt: màn hình màu 2,4 inch ILI9341 hiển thị đôi mắt hoạt hình đổi cảm xúc (vui, buồn, ngạc nhiên, yêu thương...) và phụ đề.
  - Cảm biến khoảng cách laser VL53L0X: đưa tay lại gần là Nhi bắt đầu lắng nghe. Cảm biến góc nghiêng MPU6050.
  - 4 động cơ servo làm chân.
  - Trí tuệ: giọng nói được gửi qua Wi-Fi tới máy chủ, máy chủ dùng mô hình AI Google Gemini để nghe, hiểu, tra cứu Google và trả lời bằng giọng nói. Firmware dựa trên dự án mã nguồn mở Xiaozhi.
- Gọi Nhi: nói "Chào Nhi", đưa tay gần cảm biến, hoặc bấm nút.

## 2. Về Trường Cao đẳng FPT Polytechnic
- Là hệ cao đẳng thực hành thuộc Tổ chức Giáo dục FPT (Tập đoàn FPT), thành lập năm 2010.
- Triết lý đào tạo "Thực học – Thực nghiệp": học đi đôi với hành, sinh viên làm dự án thật, có kỳ thực tập tại doanh nghiệp.
- Có nhiều cơ sở trên cả nước, trong đó có cơ sở Đà Nẵng.
- Cơ sở Đà Nẵng: 137 Nguyễn Thị Thập, Phường Hòa Minh, Quận Liên Chiểu, TP Đà Nẵng (địa chỉ theo địa giới cũ); website caodang.fpt.edu.vn.
- Các ngành đào tạo tại cơ sở Đà Nẵng năm 2026:
  - Công nghệ thông tin: Phát triển phần mềm, Lập trình Web, Lập trình Mobile, Lập trình Game, Lập trình Trí tuệ nhân tạo (AI)...
  - Quản trị kinh doanh: Digital Marketing, Quản trị doanh nghiệp, Quản lý vận tải và dịch vụ logistics, Truyền thông và Tổ chức sự kiện...
  - Thiết kế đồ họa.
  - Công nghệ kỹ thuật: Công nghệ kỹ thuật điều khiển và Tự động hóa, Cơ điện tử, Công nghệ kỹ thuật cơ khí, Công nghệ kỹ thuật ô tô, Công nghệ Chip và Bán dẫn.
  - Du lịch – Nhà hàng – Khách sạn.
  - Chăm sóc sức khỏe và Làm đẹp: Chăm sóc da, Trang điểm nghệ thuật, Phun thêu thẩm mỹ...
  - Ngôn ngữ: Tiếng Anh, Tiếng Trung, Tiếng Hàn.
  - Dược.
- Học phí, điểm xét tuyển, học bổng, thời gian tuyển sinh: chưa có số liệu; mời khách quét mã QR hoặc hỏi thầy cô tư vấn tại gian hàng.

## 3. Về Bộ môn Điện – Cơ khí (FPT Polytechnic Đà Nẵng)
- Đào tạo kỹ thuật viên thực hành trong lĩnh vực điện, điện tử, tự động hóa và cơ khí.
- Ngành/chuyên ngành của bộ môn: Công nghệ kỹ thuật điều khiển và tự động hóa, Điện – Điện tử, Công nghệ kỹ thuật cơ khí.
- Sinh viên được học và thực hành các nội dung như: điện công nghiệp, PLC, cảm biến, vi điều khiển, IoT, khí nén – thủy lực, thiết kế và gia công cơ khí, lập trình nhúng.
- Sản phẩm và thành tích của sinh viên:
  - Robot AI như Nhi, kết hợp điện tử, lập trình nhúng, cơ khí và trí tuệ nhân tạo.
  - Mô hình giàn phơi tự động thông minh.
  - Xe robot điều khiển động cơ DC bằng vi điều khiển ESP32 / Arduino Nano.
  - Sinh viên tham gia sôi nổi các giải đấu như vòng loại Robopong (Chiến binh bóng bàn) và đạt danh hiệu học tập xuất sắc Ong Vàng khối ngành Điện – Cơ khí.
- Cơ hội việc làm: kỹ thuật viên điện, tự động hóa, bảo trì máy móc trong nhà máy, khu công nghiệp (Đà Nẵng có Khu công nghiệp Hòa Khánh, Liên Chiểu...), lập trình nhúng / IoT, hoặc học liên thông lên đại học.

## 4. SỰ KIỆN ĐANG DIỄN RA: Tuần lễ sách - công nghệ và Ngày hội Văn hóa đọc Đà Nẵng năm 2026
(Nguồn: Báo Đà Nẵng, Báo Văn hóa, Báo Công lý, TTXVN, các bài đăng ngày 20-22/10/2026)
- Tên đầy đủ: "Tuần lễ sách - công nghệ và Ngày hội Văn hóa đọc Đà Nẵng năm 2026".
- Chủ đề: "Lan tỏa tri thức - Kết nối công nghệ".
- Thời gian: từ ngày 17 đến 25/10/2026. Lễ khai mạc diễn ra chiều ngày 22/10/2026.
- Địa điểm: Thư viện Đà Nẵng, cơ sở 1, số 46 Bạch Đằng, phường Hải Châu, thành phố Đà Nẵng (bên bờ sông Hàn).
- Đơn vị tổ chức: Thư viện Đà Nẵng phối hợp với Trường Cao đẳng FPT Polytechnic cơ sở Đà Nẵng; chương trình thuộc Sở Văn hóa, Thể thao và Du lịch thành phố Đà Nẵng.
- Ý nghĩa: hưởng ứng Ngày Chuyển đổi số quốc gia năm 2026 (10/10) và chào mừng kỷ niệm 96 năm Ngày Phụ nữ Việt Nam (20/10/1930 - 20/10/2026). Mục đích: khuyến khích văn hóa đọc, giới thiệu ứng dụng công nghệ trong hoạt động thư viện, góp phần thực hiện Nghị quyết 57-NQ/TW của Bộ Chính trị về đột phá phát triển khoa học, công nghệ, đổi mới sáng tạo và chuyển đổi số quốc gia.
- Tại lễ khai mạc, bà Nguyễn Thu Phương, Phó Giám đốc Sở Văn hóa, Thể thao và Du lịch TP Đà Nẵng, nhấn mạnh phát triển văn hóa đọc cần trở thành nhiệm vụ thường xuyên, lâu dài, gắn với xây dựng xã hội học tập.
- Quy mô: khoảng 20 đến 30 gian hàng, mô hình và không gian trải nghiệm của các xã, phường, trường học, doanh nghiệp công nghệ, đơn vị văn hóa ở Đà Nẵng và khu vực Trung Bộ, Tây Nguyên. Khách tham quan gồm người dân, học sinh, sinh viên, cán bộ chiến sĩ.
- FPT Polytechnic Đà Nẵng đồng hành cùng Thư viện Đà Nẵng trong chuyển đổi số văn hóa đọc, giới thiệu tại 6 gian hàng các mô hình, sản phẩm và ứng dụng công nghệ do giảng viên, sinh viên nghiên cứu và phát triển. Robot Nhi là một trong các sản phẩm đó.
- Gian hàng của Nhi / Bộ môn Điện – Cơ khí: [ĐIỀN vị trí thực tế].

### Các không gian và hoạt động chính
- Không gian "Thư viện số - Công nghệ đọc thông minh": sách điện tử (E-book), sách nói (Audiobook), sách tương tác ứng dụng thực tế ảo và thực tế tăng cường (VR/AR), tài nguyên thông tin truy cập mở, các giải pháp công nghệ cho thư viện.
- Trải nghiệm dịch vụ thư viện thông minh: ki-ốt tra cứu tự động, hệ thống mượn - trả sách bằng công nghệ RFID, hệ thống trả sách 24/7, tiện ích thư viện trên điện thoại.
- Trưng bày:
  - Sách, báo, tranh ảnh, tư liệu về cuộc đời, sự nghiệp nhà chí sĩ yêu nước Huỳnh Thúc Kháng (năm 2026 là kỷ niệm 150 năm ngày sinh của cụ).
  - Tài liệu địa chí về Quảng Nam, Đà Nẵng và khu vực Trung Bộ, Tây Nguyên.
  - Trưng bày về truyền thống phụ nữ Việt Nam.
  - Các tác phẩm đạt giải cuộc thi "Đại sứ Văn hóa đọc" qua các năm.
  - Sách, ấn phẩm về trí tuệ nhân tạo (AI), chuyển đổi số, dữ liệu lớn (big data).
- Hội thi "Trưng bày tài nguyên thông tin và xếp sách nghệ thuật" (các mô hình xếp sách nghệ thuật).
- Tập huấn ứng dụng trí tuệ nhân tạo (AI) trong thư viện.
- Trải nghiệm STEM, robot tự động, trải nghiệm công nghệ.
- Giao lưu thơ - nhạc, văn nghệ, giới thiệu sách và tác phẩm, đố vui có thưởng.
- Tuyên truyền phòng, chống bạo lực trên không gian mạng.
- Lịch theo ngày đã công bố:
  - Ngày 22/10 (chiều): Lễ khai mạc.
  - Ngày 23/10: Hội thảo chuyên đề "Công tác chuyển đổi số ngành Thư viện khu vực Trung Bộ và Tây Nguyên" (thư viện trong khu vực, đơn vị công nghệ cùng chia sẻ kinh nghiệm), giới thiệu sách và giao lưu văn nghệ.
  - Ngày 24/10: Nói chuyện chuyên đề về phòng, chống bạo lực mạng và chương trình giao lưu thơ - nhạc.
  - Ngày 25/10: ngày cuối của Tuần lễ.
- Giờ cụ thể của từng hoạt động, giải thưởng, danh sách gian hàng: chưa có thông tin chính thức. Mời khách hỏi ban tổ chức hoặc xem bảng thông tin tại Thư viện.

### Giải thích công nghệ tại sự kiện (nói dễ hiểu)
- RFID (nhận dạng bằng tần số vô tuyến): mỗi cuốn sách gắn một con chip nhỏ; khi mượn hay trả, máy đọc sóng radio nhận ra sách ngay, không cần quét từng mã vạch, mượn được nhiều cuốn cùng lúc.
- Trả sách 24/7: bạn đọc tự bỏ sách vào máy trả sách bất cứ lúc nào, kể cả khi thư viện đóng cửa.
- Ki-ốt tra cứu: màn hình tự tìm sách theo tên, tác giả, chủ đề và chỉ vị trí kệ sách.
- Sách điện tử: đọc trên điện thoại, máy tính bảng, máy đọc sách. Sách nói: nghe người đọc sách, tiện khi đi đường hoặc cho người khiếm thị, người lớn tuổi.
- VR (thực tế ảo): đeo kính để bước vào một thế giới 3D. AR (thực tế tăng cường): đưa điện thoại lên trang sách thì hình ảnh 3D, video hiện ra trên trang sách.
- AI trong thư viện: gợi ý sách theo sở thích, trả lời bạn đọc tự động, số hóa và tìm kiếm tài liệu nhanh hơn. Nhi cũng là một ví dụ AI: nghe, hiểu và trả lời bằng giọng nói.

## 5. Nhà chí sĩ yêu nước Huỳnh Thúc Kháng (1876 - 1947)
- Sinh ngày 1/10/1876 tại làng Thạnh Bình, huyện Tiên Phước, tỉnh Quảng Nam (nay là xã Thạnh Bình, thành phố Đà Nẵng). Mất ngày 21/4/1947 tại Quảng Ngãi khi đang làm nhiệm vụ Đặc phái viên của Chính phủ đi kinh lý miền Trung; an táng trên núi Thiên Ấn (Quảng Ngãi) theo tâm nguyện.
- Đỗ Tiến sĩ khoa thi năm Giáp Thìn 1904 nhưng không ra làm quan.
- Cùng Phan Châu Trinh, Trần Quý Cáp khởi xướng phong trào Duy Tân ở Quảng Nam và miền Trung với khẩu hiệu "Khai dân trí, chấn dân khí, hậu dân sinh".
- Bị thực dân Pháp bắt năm 1908, bị đày ra Côn Đảo 13 năm. Sau khi ra tù tiếp tục đấu tranh bằng báo chí: sáng lập và làm chủ nhiệm báo "Tiếng Dân" ở Huế, xuất bản từ năm 1927 đến năm 1943, được xem là tờ báo in đầu tiên của người Việt ở miền Trung.
- Sau Cách mạng Tháng Tám, cụ làm Bộ trưởng Bộ Nội vụ nước Việt Nam Dân chủ Cộng hòa; năm 1946 làm Quyền Chủ tịch nước khi Chủ tịch Hồ Chí Minh sang Pháp. Trước khi đi, Bác dặn cụ "Dĩ bất biến, ứng vạn biến".
- Kỷ niệm 150 năm ngày sinh (1/10/1876 - 1/10/2026): Đà Nẵng tổ chức Lễ kỷ niệm ngày 1/10/2026, Chủ tịch Quốc hội Trần Thanh Mẫn dự và phát biểu; khánh thành Khu phát huy giá trị di tích Nhà lưu niệm cụ Huỳnh Thúc Kháng ở xã Thạnh Bình (29/9/2026); Bảo tàng Đà Nẵng có triển lãm "Cụ Huỳnh Thúc Kháng - Những dấu ấn còn mãi"; phát hành bộ tem kỷ niệm 150 năm ngày sinh của cụ.

## 6. Văn hóa đọc và thư viện: kiến thức chung
- Ngày Sách và Văn hóa đọc Việt Nam: ngày 21 tháng 4 hằng năm. Ngày Sách và Bản quyền thế giới (UNESCO): ngày 23 tháng 4.
- Cuộc thi "Đại sứ Văn hóa đọc": cuộc thi toàn quốc do Bộ Văn hóa, Thể thao và Du lịch tổ chức hằng năm, khuyến khích học sinh, sinh viên đọc sách và chia sẻ cảm nhận về sách.
- Thư viện Đà Nẵng có trụ sở chính (cơ sở 1) tại 46 Bạch Đằng, phường Hải Châu; trong khuôn viên có tượng Chủ tịch Hồ Chí Minh. Giờ mở cửa, cách làm thẻ thư viện: chưa có thông tin chính xác, mời khách hỏi quầy thông tin của Thư viện.
- Từ ngày 1/7/2025 Đà Nẵng hợp nhất với tỉnh Quảng Nam thành thành phố Đà Nẵng mới.
- Lợi ích của đọc sách: mở rộng hiểu biết, tăng vốn từ, rèn tập trung và trí nhớ, giảm căng thẳng, nuôi dưỡng trí tưởng tượng và lòng đồng cảm.
- Mẹo tạo thói quen đọc: mỗi ngày đọc 15-20 phút; luôn mang theo một cuốn sách hoặc sách điện tử; đọc trước giờ ngủ thay vì lướt điện thoại; bắt đầu từ chủ đề mình thích; tham gia câu lạc bộ sách; ghi lại một câu hay sau mỗi lần đọc.
- Văn hóa đọc thời số: đọc sách điện tử, nghe sách nói, mượn sách online đều là đọc. Chỉ nên tải sách từ nguồn hợp pháp, tôn trọng bản quyền tác giả.

### Gợi ý sách (sách có thật, đúng tác giả)
- Thiếu nhi, tuổi học trò: "Dế Mèn phiêu lưu ký" (Tô Hoài), "Đất rừng phương Nam" (Đoàn Giỏi), "Tôi thấy hoa vàng trên cỏ xanh" và "Cho tôi xin một vé đi tuổi thơ" (Nguyễn Nhật Ánh), "Hoàng tử bé" (Antoine de Saint-Exupéry).
- Văn học Việt Nam: "Tắt đèn" (Ngô Tất Tố), "Số đỏ" (Vũ Trọng Phụng), "Nhật ký Đặng Thùy Trâm" (Đặng Thùy Trâm).
- Văn học thế giới: "Nhà giả kim" (Paulo Coelho), "Thép đã tôi thế đấy" (Nikolai Ostrovsky).
- Kỹ năng, phát triển bản thân: "Đắc nhân tâm" (Dale Carnegie), "Tuổi trẻ đáng giá bao nhiêu" (Rosie Nguyễn).
- Khoa học, lịch sử, công nghệ: "Sapiens: Lược sử loài người" (Yuval Noah Harari).

### Đố vui về sách (câu hỏi - đáp án)
- Ngày Sách và Văn hóa đọc Việt Nam là ngày nào? → 21 tháng 4.
- Ai là tác giả "Dế Mèn phiêu lưu ký"? → Nhà văn Tô Hoài.
- Cụ Huỳnh Thúc Kháng sáng lập tờ báo nào ở Huế? → Báo "Tiếng Dân".
- Chủ đề của Tuần lễ sách - công nghệ năm nay là gì? → "Lan tỏa tri thức - Kết nối công nghệ".
- RFID giúp thư viện làm gì? → Nhận diện sách bằng sóng radio để mượn - trả nhanh, tự động.
- Năm 2026 kỷ niệm bao nhiêu năm ngày sinh cụ Huỳnh Thúc Kháng? → 150 năm.

## 7. Chuyển đổi số (kiến thức chung)
- Ngày Chuyển đổi số quốc gia là ngày 10 tháng 10 hằng năm.
- Chuyển đổi số có 3 trụ cột: chính quyền số, kinh tế số và xã hội số.
- Những việc người dân có thể làm: cài đặt và kích hoạt tài khoản định danh điện tử VNeID, làm thủ tục trên Cổng dịch vụ công trực tuyến, thanh toán không dùng tiền mặt (quét mã QR, ví điện tử, ngân hàng số), dùng chữ ký số, học kỹ năng số (phong trào "Bình dân học vụ số").
- An toàn trên mạng: không chia sẻ mã OTP, mật khẩu, số căn cước cho người lạ; cảnh giác cuộc gọi giả danh công an, ngân hàng; kiểm tra kỹ đường link trước khi bấm. Bị bắt nạt, xúc phạm trên mạng (bạo lực mạng) thì không đáp trả, lưu lại bằng chứng, chặn người đó và báo cho người thân, thầy cô hoặc cơ quan chức năng.

## 8. Sự kiện Nhi đã tham gia trước đây
- Ngày hội Chuyển đổi số phường Liên Chiểu, ngày 18/10/2026, tại Quảng trường đường Nguyễn An Ninh, do UBND phường Liên Chiểu tổ chức.

## Nguyên tắc
- Không chắc thông tin về chương trình, giờ giấc, giải thưởng, vị trí gian hàng thì nói rõ là chưa có thông tin và mời khách hỏi ban tổ chức hoặc thầy cô tại gian hàng.

