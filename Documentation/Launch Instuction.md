# 📋 Инструкция по запуску P2P Pong Game

***🎯 Общая информация***

Игра использует P2P-архитектуру с SSL-шифрованием. Один игрок выступает сервером, второй - клиентом.

# 📚 Необходимые зависимости и библиотеки

# 🐧 Ubuntu/Debian:

1. Для начала установите gcc компилятор:

```bash
apt install -y build-essential make gcc
```

2. ncurses
```bash
apt install -y libncurses-dev
```

(Дополнительно может понадобиться установка библиотек ниже)

3. ssl
```bash
apt install -y libssl-dev openssl
# или
apt install -y libssl-dev
```

4. crypto
```bash
apt install -y libcrypto++-dev
```

# 🔴 Для RedHat/CentOS/Fedora:

```bash
# Для RedHat/CentOS:
sudo yum install -y gcc make ncurses-devel openssl-devel
```

```bash
# Для Fedora:
sudo dnf install -y gcc make ncurses-devel openssl-devel net-tools curl
```

# 🍎 Для macOS:

```bash
# Установка Homebrew (если не установлен)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Установка зависимостей через Homebrew
brew update
brew install openssl ncurses make gcc

# Добавление openssl в PATH (может потребоваться)
echo 'export PATH="/usr/local/opt/openssl@3/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

# 🪟 Для Windows (WSL2):

```bash
# Установка WSL2 (в PowerShell от администратора)
wsl --install

# Установка Ubuntu в WSL
wsl --install -d Ubuntu

# В WSL Ubuntu выполнить:
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential libncurses-dev libssl-dev make gcc net-tools curl
```

# Можете также воспользоваться автоматическим скриптом установки всех необходимых компонентов "install_depencedencies.sh"🙃

######################################################################################################

# 🖥️ Вариант 1: Запуск на виртуальных машинах (VM)

Предварительные требования:

- Две VM с Ubuntu/Linux
- Сетевой мост (bridged network) для прямой связи между VM
- Установленные зависимости: build-essential libncurses-dev libssl-dev

# Шаг 1: Настройка сети на VM

```bash
# Проверьте IP-адреса на обеих VM
ip addr show

# Разрешите порт 8080 в firewall
sudo ufw allow 8080/tcp
sudo ufw enable
```

# Шаг 2: Создание SSL-сертификатов

```bash
# На ОБОИХ компьютерах выполните в директории, где будет запущена игра:
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/C=RU/ST=Moscow/L=Moscow/O=PongGame/CN=localhost"

# Убедитесь что файлы создались:
ls -la cert.pem key.pem
```

# Шаг 3: Компиляция игры

```bash
# На обоих компьютерах:
make clean
make
```

# Шаг 4: Запуск игры

1. На VM 1 (Сервер):
```bash
./pong_game
# Выберите: 1 - Wait for opponent
# Запишите显示的 IP-адрес
```

2. На VM 2 (Клиент):
```bash
./pong_game
# Выберите: 2 - Connect to opponent
# Введите IP-адрес VM 1
```

# 💻 Вариант 2: Запуск на физических компьютерах в одной сети

Предварительные требования:

- Компьютеры в одной LAN сети (Wi-Fi/Ethernet)
- Отключен firewall или разрешен порт 8080
- Установлены зависимости: build-essential libncurses-dev libssl-dev

# Шаг 1: Определение IP-адресов

```bash
# На Linux:
ip addr show

# На Windows:
ipconfig

# На Mac:
ifconfig
```

# Шаг 2: Создание SSL-сертификатов

```bash
# На обоих компьютерах, в директории с файлом игры:
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/C=RU/ST=Moscow/L=Moscow/O=PongGame/CN=localhost"
```

# Шаг 3: Компиляция и запуск

1. На компьютере 1 (Сервер):
```bash
make && ./pong_game
# Выберите 1, запомните IP
```

2. На компьютере 2 (Клиент):
```bash
make && ./pong_game  
# Выберите 2, введите IP компьютера 1
```

# 🌐 Вариант 3: Запуск через интернет (с пробросом портов)

***Для продвинутых пользователей! Требуется настройка роутера.***

# Шаг 1: Настройка проброса портов

1. Зайдите в настройки роутера (обычно 192.168.1.1 или 192.168.0.1)
2. Найдите "Port Forwarding" или "Виртуальные серверы"
3. Добавьте правило: порт 8080 TCP → локальный IP серверного компьютера

# Шаг 2: Определение внешнего IP

```bash
# На серверном компьютере:
curl ifconfig.me
# или посмотрите в настройках роутера "WAN IP"
```
# Шаг 3: Запуск игры

Сервер (с проброшенным портом):

```bash
./pong_game
# Выберите 1
```

Клиент (где угодно в интернете):

```bash
./pong_game
# Выберите 2, введите внешний IP сервера
```

# 🔧 Вариант 4: Локальный запуск (тестирование на одном компьютере)

**Шаг 1: Создание сертификатов**

```bash
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/C=RU/ST=Moscow/L=Moscow/O=PongGame/CN=localhost"
```

**Шаг 2: Запуск в двух терминалах**

Терминал 1 (Сервер):

```bash
./pong_game
# Выберите 1
# Используйте IP 127.0.0.1 или localhost
```
Терминал 2 (Клиент):

```bash
./pong_game
# Выберите 2
# Введите 127.0.0.1 или localhost
```

######################################################################################################

# 🎮 Управление в игре

1. Левый игрок (сервер):

A - движение вверх
Z - движение вниз
Q - выход

2. Правый игрок (клиент):

K - движение вверх
M - движение вниз
Q - выход