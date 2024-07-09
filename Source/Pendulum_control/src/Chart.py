import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats

# Dữ liệu từ file denta_x.txt
data = np.array([
    [-22.5, 23.3], [-9.9, 14.3], [-8.3, 5.7], [-29.9, 44.5], [-0.6, 22.2],
    [-6.5, 22.2], [-31.4, 45.7], [-18.1, 5.9], [-20.9, 22.1], [-33.5, 13.4],
    [-30.1, 34.4], [-38.8, 34.4], [-36.8, 35.6], [-30.5, 27.3], [-12.1, 32.9],
    [-15.1, 55.7], [-36.3, 38.8], [-22.9, 45.2], [-43.2, 33.1], [-44.6, 30.6],
    [-40.7, 14.9], [1.3, 8.0], [-0.3, 21.8], [-33.3, 39.2], [-33.1, 47.8]
])

# Tách dữ liệu x_min và x_max
x_min = data[:, 0]
x_max = data[:, 1]

# Tính toán các giá trị thống kê
mean_x_min = np.mean(x_min)
std_dev_x_min = np.std(x_min)

mean_x_max = np.mean(x_max)
std_dev_x_max = np.std(x_max)

# Tạo lại biểu đồ hình chuông
plt.figure(figsize=(14, 6))

# Biểu đồ hình chuông cho x_min
plt.subplot(1, 2, 1)
plt.hist(x_min, bins=12, density=True, alpha=0.6, color='#17becf', label='Histogram')
x_min_range = np.linspace(mean_x_min - 4*std_dev_x_min, mean_x_min + 4*std_dev_x_min, 1000)
plt.plot(x_min_range, stats.norm.pdf(x_min_range, mean_x_min, std_dev_x_min), color='#FF9933', label='Bell Curve')
plt.xlabel('Giá trị')
plt.ylabel('Mật độ xác suất')
plt.title('Đường cong phân phối chuẩn của x_min')
plt.legend()

# Biểu đồ hình chuông cho x_max
plt.subplot(1, 2, 2)
plt.hist(x_max, bins= 12, density=True, alpha=0.6, color='red', label='Histogram')
x_max_range = np.linspace(mean_x_max - 4*std_dev_x_max, mean_x_max + 4*std_dev_x_max, 1000)
plt.plot(x_max_range, stats.norm.pdf(x_max_range, mean_x_max, std_dev_x_max), color='red', label='Bell Curve')
plt.xlabel('Giá trị')
plt.ylabel('Mật độ xác suất')
plt.title('Đường cong phân phối chuẩn của x_max')
plt.legend()

plt.tight_layout()
plt.show()
