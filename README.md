# cpp-transport-catalogue
Финальный проект: транспортный справочник  
Обзор
Проект «Каталог транспорта» — это приложение C++, предназначенное для управления и запроса информации о системах общественного транспорта. В проекте используются буферы протокола (protobuf) для сериализации данных, CMake для сборки проекта и JSON для входных данных.

Функции
Ввод данных: программа принимает входные данные в формате JSON, включая сведения об остановках, автобусах и их маршрутах.

Сериализация: данные сериализуются с использованием протокольных буферов и сохраняются в указанном файле (например, «transport_catalogue.db»).

Маршрутизация: приложение предоставляет функции маршрутизации, позволяющие пользователям находить оптимальные маршруты между остановками или местами.

Рендеринг карты: программа генерирует представление карты SVG на основе предоставленных входных данных и результатов маршрутизации.

Инструкции по сборке
Чтобы построить проект, выполните следующие действия:

Установите CMake: Руководство по установке CMake
Клонируем репозиторий:git clone https://github.com/your-username/transport-catalogue.git
Перейдите в каталог проекта:cd transport-catalogue
Создайте каталог сборки:mkdir build && cd build
Запустите CMake:cmake ..
Постройте проект:cmake --build .
Применение
Входные данные
Программа ожидает входные данные в два этапа:
Применение
Входные данные
Программа ожидает входные данные в два этапа:

Сделать базу:

Файл:make_base_input.json
Пример:
JSON


{
  "serialization_settings": {
      "file": "transport_catalogue.db"
  },
  "routing_settings": {
      "bus_wait_time": 2,
      "bus_velocity": 30
  },
  "render_settings": {
      // Render settings details...
  },
  
 
"base_requests": [
      
  
// Base requests details...
  ]
}
Обработка запросов:

Фprocess_requests_input.json
Пример
JSON

{
  
  
"serialization_settings": {
      
    
"file": "transport_catalogue.db"
  },
  "stat_requests": [
      
      
// Stat requests details...
  ]
}

`
Выход
The program outputs results in JSON format, containing information about route details, bus statistics, and map data.
