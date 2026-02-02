# ESP32 Workshop – Ortam Kurulum Rehberi

> Accelerator Workshop - Tiremo IoT Cybersecurity & Cyber Resilience Act

Bu doküman, Tiremo - IoT Cybersecurity (CRA) - Accelerator workshop kapsamında kullanılacak ESP32 geliştirme ortamının Windows, macOS ve Linux işletim sistemleri için nasıl kurulacağını adım adım açıklamaktadır.

## 📋 İçindekiler

- [Sistem Gereksinimleri](#-sistem-gereksinimleri)
- [Windows Kurulumu](#1-windows-kurulumu)
- [macOS / Linux Kurulumu](#2-macos--linux-kurulumu)
- [Kurulum Doğrulama](#-kurulum-doğrulama)
- [Faydalı Komutlar](#-faydalı-komutlar)
- [Sorun Giderme](#-sorun-giderme)
- [Kaynaklar](#-kaynaklar)

## 💻 Sistem Gereksinimleri

### Windows
- Windows 10 veya daha yeni
- Minimum 4 GB RAM (8 GB önerilir)
- En az 10 GB boş disk alanı
- Internet bağlantısı

### macOS
- macOS 10.15 (Catalina) veya daha yeni
- Homebrew paket yöneticisi
- Xcode Command Line Tools
- Minimum 4 GB RAM (8 GB önerilir)
- En az 10 GB boş disk alanı

### Linux
- Ubuntu 20.04 LTS veya daha yeni (Debian tabanlı dağıtımlar)
- Minimum 4 GB RAM (8 GB önerilir)
- En az 10 GB boş disk alanı
- sudo yetkisi

## 1. Windows Kurulumu

- https://dl.espressif.com/dl/esp-idf/ adresinden **Universal Online Installer 2.4.0** indirilir.

![Windows Installer Download](images/Picture1.png)

![IDF Language Selection](images/Picture2.png)

![Installation Progress](images/Picture3.png)

![Pre-Install Check](images/Picture4.png)

![ESP-IDF Nereden sourcelanacak](images/Picture5.png)


![ESP-IDF Versiyon Seçimi](images/Picture6.png)

- IDF versiyonu **v5.3** olarak seçilmelidir.

![ESP-IDF Kurulum Alanı](images/Picture7.png)

![ESP-IDF Komponentlerin seçimi](images/Picture8.png)

![ESP-IDF Kuruluma Hazır](images/Picture9.png)

![ESP-IDF Kuruluyor](images/Picture10.png)

- Bu aşama biraz vakit alabilir, gerekli ESP kütüphaneleri indiriliyor.

![ESP-IDF kurulum tamamlandı](images/Picture11.png)

- İşlem tamamlandıktan sonra ESP projeleri için kullanabileceğiniz terminal açılacaktır. İlerleyen günlerde sizlerle çalışacağımız proje dosyasını paylaşacağız. Bu terminalde örnek proje klasörünün içine girerek `idf.py build` komutunu çalıştırıp kodu derleyebilirsiniz.

## 2. macOS / Linux Kurulumu

### Aşama 1 – Gerekli Paketlerin Kurulumu

**Linux Kullanıcıları için:**
```bash
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

**macOS Kullanıcıları için:**
```bash
brew install cmake ninja dfu-util
```

### Aşama 2 – Python 3 Kontrolü

Python sürümünü kontrol ediniz:
```bash
python --version
```

Python 3.x yüklü olmalıdır.

Eğer Python yüklü değilse (macOS):
```bash
brew install python3
```

### Aşama 3 – ESP-IDF İndirme

```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git
```

📦 **ESP-IDF Repository:** [https://github.com/espressif/esp-idf.git](https://github.com/espressif/esp-idf.git)

### Aşama 4 – Gerekli Araçların Kurulumu

```bash
cd ~/esp/esp-idf
./install.sh esp32
```

Bu adımda derleyici, debugger ve gerekli Python paketleri kurulacaktır.

### Aşama 5 – Ortam Değişkenlerinin Ayarlanması

ESP-IDF araçlarının terminalden kullanılabilmesi için aşağıdaki komut çalıştırılmalıdır:

```bash
. $HOME/esp/esp-idf/export.sh
```

Artık proje klasörü içerisine girerek aşağıdaki komut çalıştırılabilir:

```bash
idf.py build
```

Bu komutun hatasız tamamlanması, kurulumun başarılı olduğunu gösterir.

> **Not:** Proje dosyası ilerleyen günlerde sizlerle paylaşılacaktır, bunun akabinde terminalde ESP ortamı ayarlandıktan sonra `idf.py build` komutu ile projeyi derleyebilirsiniz.

---

## Kurulum Doğrulama

Kurulumun başarılı olduğundan emin olmak için aşağıdaki komutları çalıştırın:

### ESP-IDF Versiyonunu Kontrol Etme
```bash
idf.py --version
```
Çıktıda `v5.3` görmelisiniz.

### Python Versiyonunu Kontrol Etme
```bash
python --version
```
Python 3.8 veya daha yeni bir sürüm görmelisiniz.

### Compiler Kontrolü
```bash
xtensa-esp32-elf-gcc --version
```
Bu komut ESP32 için cross-compiler versiyonunu gösterecektir.

---

## Faydalı Komutlar

Geliştirme sürecinde sıkça kullanacağınız komutlar:

### Proje Derleme
```bash
idf.py build
```

### Kod Temizleme
```bash
idf.py fullclean
```

### Seri Port Seçimi
```bash
idf.py -p /dev/ttyUSB0 flash monitor    # Linux
idf.py -p /dev/cu.usbserial-* flash monitor    # macOS
idf.py -p COM3 flash monitor    # Windows
```

### Kodu ESP32'ye Yükleme
```bash
idf.py flash
```

### Seri Monitor Başlatma
```bash
idf.py monitor
```
*Monitörden çıkmak için: `Ctrl + ]`*

### Tek Komutta Derleme, Yükleme ve İzleme
```bash
idf.py build flash monitor
```

### Proje Yapılandırma Menüsü
```bash
idf.py menuconfig
```

---

## 📚 Kaynaklar

### Resmi Dokümantasyon
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32/index.html)
- [ESP32 Teknik Referans](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP-IDF GitHub Repository](https://github.com/espressif/esp-idf)

### Faydalı Linkler
- [ESP32 Forum](https://esp32.com/)
- [Espressif GitHub Examples](https://github.com/espressif/esp-idf/tree/master/examples)
- [ESP-IDF VS Code Extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)

### Video Eğitimler
- [Espressif Systems YouTube](https://www.youtube.com/c/EspressifSystems)

---
