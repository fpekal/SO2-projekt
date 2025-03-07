# Zadanie 1 - Problem jedzących filozofów

## Problem
Problem jedzących filozofów polega na utworzeniu wielu wątków, które próbują uzyskać
wyłączny dostęp do kilku zasobów.
<img src="https://upload.wikimedia.org/wikipedia/commons/8/81/Dining_philosophers_diagram.jpg" width="400"/>  
Problem modeluje się jako N filozofów i widelców (na obrazku N=5).
Każdy z filozofów chce, po skończeniu myślenia, wziąć oba widelce i rozpocząć jedzenie,
ale nie może tego zrobić, jeśli którykolwiek z jego widelców jest już zajęty.
Musi wtedy czekać, aż dany widelec się zwolni.

Podczas symulacji tego problemu możliwe są dwie niepożądane sytuacje:
- któryś z filozofów jest "zagłodzony". Znaczy to, że przez dłuższy okres czasu
nie udaje mu się uzyskać dostępu do jego widelców.
- każdy z filozofów zdążył wziąć jeden z jego widelców, po czym rozpoczął czekanie na
drugi widelec. Powstaje wtedy sytuacja cyklicznego oczekiwania na drugi widelec,
blokująca wszystkich filozofów, poniważ żaden widelec nigdy nie zostanie zwolniony.
Taka sytuacja nazywana jest "deadlockiem".


## Rozwiązanie
Wybranym przeze mnie rozwiązaniem było utworzenie "arbitra" - kodu, który
kontroluje liczbę filozofów, którzy jednocześnie próbują uzyskać dostęp do swoich
widelców. Ograniczenie zostało wybrane jako N-1 liczby wszystkich filozofów.
Dzięki temu sytuacja cyklicznego oczekiwania nie jest możliwa oraz kolejnym wybranym
filozofem do jedzenia jest ten, który poprzednio czekał.


## Kompilacja
Wymagane programy:
 - g++
 - make

Aby skompilować program wystarczy napisać `make` w katalogu z plikami źródłowymi.


## Wylistowanie
### Wątki

Każdy filozof ma swój oddzielny wątek, który rezerwuje swoje miejsce u arbitra,
następnie czeka na zwolnienie widelca lewego, rezerwuje go, potem czeka na zwolnienie
widelca prawego i też go rezerwuje.
Następnie przez zadany czas spożywa. Po tym czasie zwalnia widelec prawy, potem lewy,
zwalnia miejsce u arbitra i informuje inne oczekujące wątki, że pojawiło się nowe
miejsce.
Potem myśli przez zadany czas.
Całość wykonuje się w kółko, aż do wysłania sygnału o końcu programu.

Zadaniem głównego wątku programu jest uruchomienie wątków filozofów, odczekania
10 sekund, a następnie wysłanie sygnału o końcu programu.

### Sekcje krytyczne

#### Zarezerwowanie miejsce u arbitra
```cpp
std::unique_lock<std::mutex> arbiter_guard(arbiter_mutex);
arbiter_cv.wait(arbiter_guard, [&] { return arbiter > 0; });
arbiter--;
arbiter_guard.unlock();
```
Ten blok powoduje, że wątek filozofa czeka, aż zmienna `arbiter` będzie większa
od 0, po czym zmniejsza ją.
Początkowo ta zmienna ustawiona jest na liczbę wszystkich filozofów - 1.
Po skończonym posiłku filozof zwalnia miejsce wykonując:
```cpp
arbiter_guard.lock();
arbiter++;
arbiter_cv.notify_one();
arbiter_guard.unlock();
```

#### Zarezerwowanie widelców
```cpp
std::unique_lock<std::mutex> guard(*left_fork, std::defer_lock);
std::unique_lock<std::mutex> guard2(*right_fork, std::defer_lock);

// [...]

guard.lock();
guard2.lock();
std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 500 + 500));
guard2.unlock();
guard.unlock();
```

#### Wysyłanie sygnału o końcu programu
Tworzona jest zmienna atomiczna `running`.
```cpp
std::atomic<bool> running;
```
A następnie wątki filozofów wykonują się, dopóki zmienna `running` jest `true`.
```cpp
while (running) {
    // [...]
}
```

#### Wyświetlanie tekstu na ekranie
Dodatkowo utworzona została specjalna klasa, która blokuje możliwość jednoczesnego
wypisywania tekstu na ekranie przez wiele wątków na raz.
```cpp
class Printer {
public:
	template<class T>
	Printer& operator<<(const T& t) {
		std::unique_lock<std::mutex> lock(mutex);
		std::cout << t;
		return *this;
	}

	std::mutex mutex;
};
```

## Uruchomienie
Program bierze jako argument liczbę filozofów.  
`./zad1 [liczba filozofów]`

Przykładowo:  
`./zad1 5`
