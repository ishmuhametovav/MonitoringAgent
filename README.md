# MonitoringAgent

Кроссплатформенный агент мониторинга активности пользователя на C++20. Собирает метрики каждые 5 секунд, отправляет
пакеты JSON на сервер раз в 30 секунд (или при 10 записях). При недоступности сервера данные сохраняются на диск и
отправляются при восстановлении связи.

Поддерживаемые платформы: Windows (WinAPI), Linux (X11 + XScreenSaver).

## Структура

```
.
├── main.cpp
├── Application.{h,cpp}      главный цикл, отправка
├── packet/                  сборка JSON-пакета
├── queue/                   очередь с персистентностью
├── metrics/                 платформенный сбор метрик
├── TestServer.cpp           тестовый HTTP-сервер
└── CMakeLists.txt
```

## Зависимости

Подтягиваются автоматически через CMake `FetchContent`:

- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [nlohmann/json](https://github.com/nlohmann/json)

Для Linux дополнительно нужны `libx11-dev` и `libxss-dev`.

## Сборка

### Windows

```bat
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Linux

```bash
sudo apt install libx11-dev libxss-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Будут собраны два бинарных файла: `MonitoringAgent` и `Server`.

## Запуск

В одном терминале — тестовый сервер:

```bash
./build/Server
```

В другом — агент мониторинга:

```bash
./build/MonitoringAgent
```

Агент мониторинга отправляет пакеты на `http://localhost:8080`, сервер печатает их содержимое в консоль.

`Ctrl+C` — корректное завершение с сохранением неотправленных записей в `queue.json`.

## Формат пакета

```json
{
  "agent_id": "DESKTOP-MIDDLE-C",
  "timestamp": 1792147320,
  "payload": [
    {
      "time": "2026-09-15 13:55:00",
      "process_name": "chrome.exe",
      "window_title": "ИНСАЙДЕР — Система мониторинга",
      "user_active": true
    }
  ]
}
```

Очередь ограничена 100 записями; при переполнении вытесняются самые старые. При выходе и при ошибке отправки очередь
сохраняется в `queue.json` и восстанавливается при следующем запуске.