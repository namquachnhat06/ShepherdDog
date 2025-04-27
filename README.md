GIỚI THIỆU CƠ BẢN

“Shepherd Dog của tôi” là một game có sự kết hợp độc đáo giữa hai pha:

Ban ngày: Bạn vào vai một chú chó chăn cừu, nhiệm vụ là lùa cừu vào chuồng. Các con cừu được điều chỉnh để có thể đứng gần nhau hơn, tạo thành nhóm chặt chẽ mà không chồng lấn, giúp việc dồn cừu trở nên dễ dàng và tự nhiên hơn. Mỗi con cừu được đưa vào chuồng sẽ được spawn lại sau 10 giây. Pha này kéo dài 1 phút 14 giây.

Ban đêm: Bạn điều khiển nhân vật để phòng thủ trang trại khỏi sói, với cơ chế thắng/thua rõ ràng:

	Thắng nếu không để quái vượt qua nhân vật quá 3 lần sau 74 giây.

	Thua nếu quái vượt qua hơn 3 lần (game kết thúc ngay lập tức) hoặc sau 74 giây mà để vượt quá 3 lần.
 
 	Khi game kết thúc, màn hình kết quả hiển thị với nền bg.png, hình win.png (nếu thắng) hoặc lose.png (nếu thua) ở giữa, kèm âm thanh win.wav hoặc lose.wav.

CẤU TRÚC CHƯƠNG TRÌNH & CÁC CHỨC NĂNG CHÍNH

Phần ban ngày:

	Điều khiển: Bạn điều khiển chó để lùa cừu vào chuồng.
 
	AI: Cừu tự động né khi chó đến gần, với khoảng cách giữa các con cừu được tối ưu để chúng đứng sát nhau hơn nhưng vẫn không chồng lấn. Cừu sẽ spawn lại sau 10 giây khi được đưa vào chuồng.

	Âm thanh: Phát nhạc nền ban ngày; tiếng cừu kêu khi vào chuồng.

Chuyển cảnh:

	Sau 1 phút 14 giây ban ngày, game hiển thị ảnh chuyển cảnh trong 3 giây để chuẩn bị sang phần ban đêm.

Phần ban đêm:

	Điều khiển: Người chơi điều khiển nhân vật cầm súng di chuyển giữa 3 lane bằng phím mũi tên lên/xuống (với delay nhất định) và bắn đạn bằng phím Space.

	Gameplay: Quái vật xuất hiện từ bên phải theo 3 lane cố định. Mỗi viên đạn chỉ tiêu diệt được 1 con quái. Game kết thúc nếu quái vượt qua nhân vật quá 3 lần (ngay lập tức) hoặc sau 74 giây với số lần vượt qua được kiểm tra để xác định thắng/thua.

	Âm thanh: Có nhạc nền ban đêm; tiếng bắn đạn; tiếng quái chết.

	Kết thúc: Sau 1 phút 14 giây, màn đêm kết thúc và chuyển sang màn hình kết quả:

	Nếu số quái vượt qua ≤ 3: Hiển thị win.png và phát win.wav.

	Nếu số quái vượt qua > 3: Hiển thị lose.png và phát lose.wav.

	Người chơi có thể thoát game bằng phím ESC hoặc đóng cửa sổ.

TÀI NGUYÊN & HIỆU ỨNG:

	Hình ảnh: Bao gồm các file ảnh nền ban ngày, nền ban đêm, chuyển cảnh, hình ảnh nhân vật, quái, chuồng, đạn, và hình kết quả (win.png, lose.png):

	Tải về từ Google Image.

	Âm thanh: Bao gồm nhạc nền cho từng pha, tiếng động của cừu, bắn, quái chết, và âm thanh kết quả (win.wav, lose.wav):

	Tải về từ https://pixabay.com/

	Nguồn code: Đoạn code chính được tham khảo cách xây dựng từ ChatGPT, sau đó được cải tiến để tối ưu hóa trải nghiệm chơi.

	Link demo & game hoàn chỉnh:

	Game demo: https://drive.google.com/drive/folders/1XVtpLv_gneD_Vdf_a1CvTwKK416VSxG8

	Game complete: https://drive.google.com/drive/folders/1YWgCrEZn3c306p9QSkgw5YFWkIdS1YcK
