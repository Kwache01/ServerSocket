// Указывает на использование только основной части заголовочных файлов Windows
#define WIN32_LEAN_AND_MEAN

// Подключение необходимых заголовочных файлов для работы с Windows и сокетами
#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

// Использование пространства имен std для упрощения написания кода
using namespace std;

int main() {
    WSADATA wsaData;  // Структура для хранения информации о реализации Windows Sockets
    ADDRINFO hints;  // Структура для указания критериев для getaddrinfo
    ADDRINFO* addrResult = nullptr;  // Указатель для хранения результатов getaddrinfo
    SOCKET ListenSocket = INVALID_SOCKET;  // Сокет для прослушивания входящих соединений
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для работы с принятым соединением
    char recvBuffer[512];  // Буфер для получения данных

    const char* sendBuffer = "Hello from server";  // Данные для отправки клиенту

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Обнуление структуры hints и настройка параметров для создания сокета
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // Использовать IPv4
    hints.ai_socktype = SOCK_STREAM; // Использовать потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE;     // Указать сокет для прослушивания

    // Получение информации о локальном адресе и порте для создания сокета
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания входящих соединений
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязка сокета к полученному IP адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Установка сокета в режим прослушивания входящих соединений
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Сокет для прослушивания больше не нужен, закрываем его
    closesocket(ListenSocket);

    // Цикл для получения данных от клиента и отправки ответа
    do {
        ZeroMemory(recvBuffer, 512);  // Обнуление буфера для получения данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных обратно клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << WSAGetLastError() << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            // Соединение закрыто клиентом
            cout << "Connection closing" << endl;
        }
        else {
            // Ошибка при получении данных
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0);

    // Завершение отправки данных
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрытие сокета и освобождение ресурсов
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}
