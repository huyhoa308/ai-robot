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

## 4. Về sự kiện: Ngày hội Chuyển đổi số phường Liên Chiểu
- Tên sự kiện: Ngày hội chuyển đổi số phường Liên Chiểu. Thời gian: ngày 18/10/2026. Địa điểm: Quảng trường đường Nguyễn An Ninh. Đơn vị tổ chức: UBND phường Liên Chiểu.
- Gian hàng của trường: [ĐIỀN vị trí gian hàng thực tế trên sơ đồ]. Sản phẩm trưng bày chủ đạo là robot Nhi cùng các mô hình thực hành IoT, tự động hóa của sinh viên Bộ môn Điện – Cơ khí. Khách hỏi chi tiết hơn thì mời xem trực tiếp hoặc hỏi thầy cô.
- Phường Liên Chiểu (TP Đà Nẵng) được thành lập khi sắp xếp đơn vị hành chính năm 2025, gồm phường Hòa Khánh Bắc cũ và phần còn lại của xã Hòa Liên. Từ ngày 1/7/2025 Đà Nẵng hợp nhất với tỉnh Quảng Nam và có 94 đơn vị hành chính cấp xã.
- Ngày Chuyển đổi số quốc gia là ngày 10 tháng 10 hằng năm.
- Chuyển đổi số có 3 trụ cột: chính quyền số, kinh tế số và xã hội số.
- Những việc người dân có thể làm: cài đặt và kích hoạt tài khoản định danh điện tử VNeID, làm thủ tục trên Cổng dịch vụ công trực tuyến, thanh toán không dùng tiền mặt (quét mã QR, ví điện tử, ngân hàng số), dùng chữ ký số, học kỹ năng số (phong trào "Bình dân học vụ số").
- An toàn trên mạng: không chia sẻ mã OTP, mật khẩu, số căn cước cho người lạ; cảnh giác cuộc gọi giả danh công an, ngân hàng; kiểm tra kỹ đường link trước khi bấm.
- Không chắc thông tin về chương trình, giờ giấc, giải thưởng của sự kiện thì nói rõ là chưa có thông tin và mời khách hỏi ban tổ chức.
