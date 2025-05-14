# Zadanie 2 - Wielowątkowy serwer czatu

## Treść zadania
Wymagania:
- Serwer tworzy osobny wątek dla każdego połączenia od klienta
- Serwer dba o synchronizację wiadomości od klientów
- Klient widzi wiadomości w chacie
- Klient ma możliwość wysyłania wiadomości

## Program
Program jest podzielony na 2 części: serwer i klientów.
Komunikacja między serwerem i klientami odbywa się z pomocą socketów działących w trybie AF_INET.
Klienci wysyłają do serwera tekst, a serwer wysyła ten tekst dalej do wszystkich połączonych do niego klientów. (Podobnie jak MQTT)
Po połączeniu wysyłana jest do nowego klienta historia wszystkich wiadomości.

Dla ułatwienia komunikacji wiadomości wysyłane przez klientów mają format `[timestamp] <nick> wiadomość`.

## Wielowątkowość
### Klient
Klient posiada 2 wątki:
- Odczytujący klawiaturę i wysyłający wiadomości
- Odczytujący wiadomości

Jedyna synchronizacja między tymi dwoma wątkami polega na wysłaniu informacji do wątku odczytującego wiadomości, że powinien się kończyć.
Zaimplementowane jest to używając `std::atomic<bool>`.

```cpp
std::atomic<bool> running = true;
// [...]
while (running) {
  std::string message;
  std::getline(std::cin, message);

  if (!std::cin.good()) {
    running = false;
    break;
  }

  c.send(std::string{"["} + get_date() + "] " + std::string{"<\e[0;36m"} +
         nickname + "\e[0m> " + message);
}
```

```cpp
static void receiver_thread_func(Client &c, std::atomic<bool> &running) {
  while (running) {
    std::cout << c.receive();
  }
}
```


### Serwer
Każdy klient ma swój własny wątek czekający na wiadomość od klienta.
Po tym jak odczytają one, że klient wysłał wiadomość, wysyłają one ją do wszystkich klientów.
Zostało dodane zabezpieczenie, aby nie była możliwa sytuacja, że dwia wątki próbują napisać wiadomość do tego samego klienta.
Zostało to zaimplementowane z użyciem `std::mutex`.
```cpp
void send(const std::string &message) {
  std::unique_lock<std::mutex> lock{send_mutex};

  int ret = write(fd, message.c_str(), message.length());
  if (ret == -1) {
    throw std::runtime_error{"Client disconnected"};
  }
}
```

Dodatkowo podczas dostępu do listy wszystkich połączonych użytkowników należy się upewnić, że robi to na raz tylko 1 wątek.
To również zostało zaimplementowane z użyciem `std::mutex`.
```cpp
std::shared_ptr<Client> add_client(int fd) {
  auto client = std::make_shared<Client>(fd);

  std::unique_lock<std::mutex> lock{clients_mutex};
  clients.push_back(client);
  return client;
}
```

```cpp
void remove_client(std::shared_ptr<Client> client) {
  std::unique_lock<std::mutex> lock{clients_mutex};
  auto iter = std::find(clients.begin(), clients.end(), client);
  clients.erase(iter);
}
```

```cpp
void send_to_all(const std::string &message) {
  std::unique_lock<std::mutex> lock{clients_mutex};

  for (auto client : clients) {
    client->send(message);
  }
}
```

To samo trzeba było wykonać podczas dostępu do historii wszystkich wiadomości.
```cpp
void add_history(const std::string &message) {
  std::unique_lock<std::mutex> lock{history_mutex};

  history += trim(message);
  history += "\n";
}
```

```cpp
std::string get_history() {
  std::unique_lock<std::mutex> lock{history_mutex};

  return history;
}
```

## Kompilacja
Wymagane programy:
 - g++
 - make

Aby skompilować program wystarczy napisać `make` w katalogu z plikami `Makefile`, oddzielnie klienta i serwer.

## Uruchomienie
### Serwer
Serwer bierze jako argumenty adres ip i port na którym ma być dostępny.  
`./server [adres-ip port]`  
Domyślnie adres ip to `127.0.0.1`, a port to `2137`.

Przykładowo:  
`./server 192.168.1.100 4321`


### Klient
Klient bierze jako argumenty adres ip i port serwera, z którym ma się połączyć
`./client [adres-ip [port]]`  
Domyślnie adres ip to `127.0.0.1`, a port to `2137`.

Przykładowo:  
`./client 192.168.1.100 4321`
