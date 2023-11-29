[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/Starsznayder/USRP-B210-recorder/blob/main/README.md)

# Rejestratorka B210

Prosty rejestrator sygnałów dla USRP B210

**Zależności**

Wymaga UHD (testowana była wersja 4.1) oraz boost (testowano wersję 1.71).

**Kompilacja:**


1.  Można użyć skryptu build.sh w głównym katalogu projektu, ale należy ustawić ścieżkę instalacji:
`PROJ_PATH="/kitty"`

2. Można też uzyć dowolnego narzędzia rozumiejącego CMAKE. Wtedy koniecznie trzeba ustawić zmienną *KITTY_INSTALL_PATH*, która wskazuje scieżkę instalacji. Poza tym trzeba zatroszczyć się o spełnienie zależności do biblioteki z logami (utu), która powinna trafić do KITTY_INSTALL_PATH/include/utu.

**Uzycie**

Domyślnie program instaluje się w katalogu *KITTY_INSTALL_PATH/bin* a pliki konfiguracyjne w *KITTY_INSTALL_PATH/bin/ini*. 
Uruchomienie programu:
`./PK_B210`

Może być konieczne ustawienie ściezki do obrazów FPGA np.:
`export UHD_IMAGES_DIR=/kitty/rfnoc/share/uhd/images`

**Konfiguracja**

Wszytkie najważniejsze i konfigurowalne parametry znajdują się w pliku konfiguracyjnym *KITTY_INSTALL_PATH/bin/ini/simpleRecorder.ini*, który ma postać 2 lub więcej sekcji.

Pierwsze sekcje (*[USRPX]*) dotyczą konfiguracji urządzeń, gdzie X to umowny identyfikator urządzenia [0 ... N]. W tych sekcjach dostępne są następujące pola:

*  addr - W przypadku B210 ustawia się serial oraz parametry UHD np. `addr=serial=3231C8B,num_recv_frames=512`. Serial można uzyskań np. poprzez *uhd_find_devices*.
*  GainRX - wzmocnienie Rx w GNUdB
*  FcRx - nośna
*  FsRx - Fs
*  B - pasmo filtru
*  writePathY - ściezka zapisu, gdzie Y to umowny identyfikator kanału. Ilośc pól writePath dodatkowo definiuje ilość użytych kanałów w tym urządzeniu np. 
  `writePath0=/kitty/rec/CH0; writePath1=/kitty/rec/CH1` oznacza 2 kanały odbiorcze, usunięcie writePath1 powoduje pozostawienie jednego kanału. Ścieżki mogą wskazywać na np. różne dyski twarde jeżeli wystepują problemy z szybkością zapisu.
*  dataPartSize - rozmiar bloku danych do zapisu na dysku. Większy blok to rzadrze przerywanie strumienia ale też większe zużycie pamięci RAM. Wielość bufora zapisu to będzie 100 * dataPartSize * ilość kanałów odbiorczych * rozmiar próbki complex<int16_t>. Bufor warto zwiąkszyć jeżeli zrywany jest strumień danych z USRP, lub zmniejszyć jeżeli nagranie ma być bardzo krótkie.

Na końcu pliku znajduje się sekcja rec. Zawiera ona pola:
*  timestamp - należy pozostawić null
*  timestamp64 - rónież nie zmieniać
*  notes - notatka do nagrania. Zostanie ona zapisana razem z plikami nagrań (może np. zawierać opis tego co jest nagrywane). **WAŻNE** Notatke można dodać już po rozpoczęciu nagrywania poprzez zmianę pola w tym pliku i zapisanie go. Program odczytuje plik konfiguracyjny z sekundowym interwałem wi wykrywa wszelkie zmiany. Każda kolejna nostatka pojawi sia pod postacią nowego pliku *.note we wszystkich katalogach nagrania.
*  syncSource - źródło synchronizacji do wyboru **[internal external gpsdo]**
*  octoclockAddr - adres Octoclock. Jeżeli używany jest octoclock do synchronizacji USRP a syncSource jest ustawione na gpsdo, to czas GPS zostanie pobrany z Octoclock pod wskazanycm adresem IP (USRP nie muszą wtedy posiadać modułu GPS, będą pracowały w trybie external)
*  targetStartTime - warość większa niż aktualny czas unix w sekundach spowoduje, że nagranie zostanie rozpoczęte o zadanym czasie - nalezy wpisać czas unix w sekundach. Jeżeli jest tu 0 to nagranie zostanie rozpoczęte w chwili skonfigurowania urządzeń + startOffset
*  startOffset - opóźnienie rozpoczecia strumienia, daje urządzeniu czas na zaaplikownaie konfiguracji.

**Mozliwości synchronizacji**
Każde urządzenie USRP jest reprezentowane w pliku konfiguracyjnym przez własną sekcję [USRPX], gdzie X to kolejne indeksy urządzeń począwszy od 0. Wspierane są metody synchronizacji uznane za skuteczne:

*  Octoclock + czas GPS z octoclock:

Kable REF i PPS z USRP'ów są podłączone do octoclocka. Ustawione jest pole [rec].octoclockAddr na adres IP octoclocka (domyślnie 192.168.10.3), syncSource=gpsdo. 

*  Octoclock + czas GPS z USRP:

Kable REF i PPS z USRP'ów są podłączone do octoclocka. Ustawione jest pole [rec].octoclockAddr na **none**. Czas gps zostanie pobrany z urządzenia USRP wyspecyfikowanego w sekcji **[USRP0]**, syncSource=gpsdo. 

*  Bez czasu GPS ale z octoclock

Kable REF i PPS z USRP'ów są podłączone do octoclocka. Ustawione jest pole [rec].octoclockAddr na **none**, syncSource=external. 

*  Bez jakiejkolwiek synchronizacji

Ustawione jest pole [rec].octoclockAddr na **none**, syncSource=internal. 
 

**Performance**

Zmierzono wydajność na komputerze intel NUC 8 z dyskiem NVME Samsung PM981. Uzyskano strumień 60 MS / urządzenie - na wiecej nie pozwolił sam USRP.


#Użycie z WSL (Windows Subsystem for Linux)

**Konfiguracja WSL**

Instalacja środowiska i ubuntu 20.04:
* power shell uruchominy jako administrator

`Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux`

`wsl --install`

* reboot

 `wsl.exe --install -d Ubuntu-20.04`

* podajemy usera i hasło
* reboot

`winget install --interactive --exact dorssel.usbipd-win`

[link1](https://github.com/dorssel/usbipd-win)
[link2](https://woshub.com/share-host-usb-devices-windows-wsl-hyper-v/)

#jeśli możesz, zawróć

**Konfiguracja Ubuntu**

* uruchamiamy konsolę WSL
* instalujemy zależności

`sudo apt install htop ncdu aptitude libboost1.71-all-dev git python3 python3-pip python3-setuptools python3-dev build-essential cmake`

`sudo apt install python-is-python3`

`sudo apt install linux-tools-5.15.0-87-generic hwdata`

`sudo cp -r /usr/lib/linux-tools/5.15.0-87-generic/usbip /usr/local/bin/usbip`

`sudo cp -r /usr/lib/linux-tools/5.15.0-87-generic/usbipd /usr/local/bin/usbipd`

* instalujemy UHD (RFNoC)
[link](https://kb.ettus.com/Getting_Started_with_RFNoC_Development)

`cd`

`sudo pip install git+https://github.com/gnuradio/pybombs.git`

`pybombs recipes add gr-recipes git+https://github.com/gnuradio/gr-recipes.git`

`pybombs recipes add ettus git+https://github.com/EttusResearch/ettus-pybombs.git`

`pybombs --config makewidth=7 prefix init ~/rfnoc -R rfnoc -a rfnoc`

`cd ~/rfnoc`

`./setup_env.sh`


* kompilacja Rejsetratorki

`cd`

`git clone http://192.168.90.95/Kot/rejestratorka-b210.git`

`cd rejestratorka-b210`

`chmod 777 build.sh`

`mkdir ~/kitty`

`mkdir ~/kitty/rec`

`mkdir ~/kitty/rec/CH0`

`mkdir ~/kitty/rec/CH1`

`./build.sh`

**Przekierowanie portu USB**
* Power shell uruchominy jako administrator
* Podłączenie USB B210
* Instalacja sterownika windows [link](https://files.ettus.com/manual/page_transport.html#transport_usb_installwin)

`usbipd wsl list`

* Znalezienie urządzenia Ettus

`usbipd wsl attach --busid <BUSID>`

* Na ubuntu (w konsoli WSL) 

`sudo su`

`cd ~/rfnoc/bin`

`./uhd_find_devices`

Jeżeli nie znaleziono USRP to możliwe, że windows w międzyczasie przełączył sobie BUSID (tak sam z siebie) wtedy powtarzamy do skutku (stanu ustalonego w BUSID):

`usbipd wsl list`

`usbipd wsl attach --busid <BUSID>`

* Uruchomienie

`cd ~/kitty/bin`

`sudo su`

`export LD_LIBRARY_PATH=/home/<username>/rfnoc/lib`

`./PK_B210`

Ważne jest uruchomienie jako root, z powodu uprawnień do portu USB

**Tipy**
* Pamietać o skopiowaniu seriala zwróconego przez `./uhd_find_devices` do pliku konfiguracyjnego /home/<USER>/kitty/bin/ini/simpleRecorder.ini
* Ustawić priorytet Real Time procesom **vmmem** oraz **usbipd**
* Po uruchomieniu nie dotykać komputera, strumień przez usbipd jest bardzo niestabilny, najlepiej wyłączyć wszytko co nie jest potrzebne i ustawić komputer w tryb max performance
* Rozciągnięcie dysku, np. utworzenie bardzo dużego pliku i jego skasowanie, albo nagwywanie przez długi czas z małym FS ~5MS i skasowanie nagrania (taki zabieg potrafi zdziałać cuda!)





